/***************************************************************************
 *   copyright            : (C) 2006 by Benoit Jacob
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "config.h"

#include <avogadro/cylinder.h>
#include <math.h>

using namespace Avogadro;
using namespace OpenBabel;
using namespace Eigen;

Cylinder::Cylinder()
{
	m_vertexBuffer = 0;
	m_normalBuffer = 0;
	m_displayList = 0;
	m_faces = 0;
}

Cylinder::~Cylinder()
{
	freeBuffers();
	if( m_displayList )
		glDeleteLists( m_displayList, 1 );
}

void Cylinder::freeBuffers()
{
	if( m_normalBuffer )
	{
		delete [] m_normalBuffer;
		m_normalBuffer = 0;
	}
	if( m_vertexBuffer )
	{
		delete [] m_vertexBuffer;
		m_vertexBuffer = 0;
	}
}

void Cylinder::setup( int faces )
{
	if( faces == m_faces ) return;
	m_faces = faces;
	initialize();
}

void Cylinder::initialize()
{
	if( m_faces < 3 ) return;

	// compute number of vertices
	m_vertexCount = 2 * m_faces + 2;

	// deallocate any previously allocated buffer
	freeBuffers();

	// allocate memory for buffers
	m_vertexBuffer = new Vector3f[m_vertexCount];
	if( ! m_vertexBuffer ) return;
	m_normalBuffer = new Vector3f[m_vertexCount];
	if( ! m_normalBuffer ) return;

	// build vertex and normal buffers
	for( int i = 0; i <= m_faces; i++ )
	{
		float angle = 2 * M_PI * i / m_faces;
		Vector3f v( cosf(angle), sinf(angle), 0.0f );
		m_normalBuffer[ 2 * i ] = v;
		m_normalBuffer[ 2 * i + 1 ] = v;
		m_vertexBuffer[ 2 * i ] = v;
		m_vertexBuffer[ 2 * i + 1 ] = v;
		m_vertexBuffer[ 2 * i ].z() = 1.0f;
	}

	// compile display list and free buffers
	if( ! m_displayList ) m_displayList = glGenLists( 1 );
	if( ! m_displayList ) return;
	glNewList( m_displayList, GL_COMPILE );
	do_draw();
	glEndList();
	freeBuffers();
}

void Cylinder::do_draw() const
{
	glVertexPointer( 3, GL_FLOAT, 0, m_vertexBuffer );
	glNormalPointer( GL_FLOAT, 0, m_normalBuffer );
	glDrawArrays( GL_QUAD_STRIP, 0, m_vertexCount );
}

void Cylinder::draw( const Vector3d &end1, const Vector3d &end2,
	double radius, int order, double shift ) const
{
	// the "axis vector" of the cylinder
	Vector3d axis = end2 - end1;

	// now we want to construct an orthonormal basis whose first
	// vector is axis.normalized(). We don't use Eigen's loadOrthoBasis()
	// for that, because we want one more thing. The second vector in this
	// basis, which we call ortho1, should be approximately lying in the
	// z=0 plane if possible. This is to ensure double bonds don't look
	// like single bonds from the default point of view.
	double axisNorm = axis.norm();
	if( axisNorm == 0.0 ) return;
	Vector3d axisNormalized = axis / axisNorm;
	
	Vector3d ortho1( axisNormalized.y(), -axisNormalized.x(), 0.0 );
	double ortho1Norm = ortho1.norm();
	if( ortho1Norm > 0.001 ) ortho1 /= ortho1Norm;
	else {
		ortho1 = Vector3d( 0.0,
		                   axisNormalized.z(),
		                   -axisNormalized.y() );
		ortho1.normalize();
	}
	ortho1 *= radius;

	Vector3d ortho2 = cross( axisNormalized, ortho1 );       

	// construct the 4D transformation matrix
	Matrix4d matrix;

	matrix(0, 0) = ortho1(0);
	matrix(1, 0) = ortho1(1);
	matrix(2, 0) = ortho1(2);
	matrix(3, 0) = 0.0;

	matrix(0, 1) = ortho2(0);
	matrix(1, 1) = ortho2(1);
	matrix(2, 1) = ortho2(2);
	matrix(3, 1) = 0.0;

	matrix(0, 2) = axis(0);
	matrix(1, 2) = axis(1);
	matrix(2, 2) = axis(2);
	matrix(3, 2) = 0.0;

	matrix(0, 3) = end1(0);
	matrix(1, 3) = end1(1);
	matrix(2, 3) = end1(2);
	matrix(3, 3) = 1.0;

	//now we can do the actual drawing !
	glPushMatrix();
	glMultMatrixd( matrix.array() );
	if( order == 1 )
		glCallList( m_displayList );
	else
	{
		double angleOffset = 0.0;
		if( order >= 3 )
		{
			if( order == 3 ) angleOffset = 90.0;
			else angleOffset = 22.5;
		}
		
		double displacementFactor = shift / radius;
		for( int i = 0; i < order; i++)
		{
			glPushMatrix();
			glRotated( angleOffset + 360.0 * i / order,
			           0.0, 0.0, 1.0 );
			glTranslated( displacementFactor, 0.0, 0.0 );
			glCallList( m_displayList );
			glPopMatrix();
		}
	}
	glPopMatrix();
}

