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

#include <avogadro/sphere.h>
#include <math.h>

using namespace Avogadro;
using namespace OpenBabel;
using namespace Eigen;

Sphere::Sphere()
{
	m_vertexBuffer = 0;
	m_indexBuffer = 0;
	m_displayList = 0;
	m_detail = 0;
}

Sphere::~Sphere()
{
	freeBuffers();
	if( m_displayList )
		glDeleteLists( m_displayList, 1 );
}

void Sphere::freeBuffers()
{
	if( m_indexBuffer )
	{
		delete [] m_indexBuffer;
		m_indexBuffer = 0;
	}
	if( m_vertexBuffer )
	{
		delete [] m_vertexBuffer;
		m_vertexBuffer = 0;
	}
}

void Sphere::do_draw() const
{
	glVertexPointer( 3, GL_FLOAT, 0, m_vertexBuffer );
	glNormalPointer( GL_FLOAT, 0, m_vertexBuffer );
	glDrawElements( GL_TRIANGLE_STRIP, m_indexCount,
			GL_UNSIGNED_SHORT, m_indexBuffer );
}

void Sphere::draw( const Eigen::Vector3d &center, double radius ) const
{
	glPushMatrix();
	glTranslated( center.x(), center.y(), center.z() );
	glScaled( radius, radius, radius );
#ifdef USE_DISPLAY_LISTS
	glCallList( m_displayList );
#else
	do_draw();
#endif
	glPopMatrix();
}

void Sphere::initialize()
{
	if( m_detail < 1 ) return;

	// compute number of vertices and indices
	m_vertexCount = ( 3 * m_detail + 1 ) * ( 5 * m_detail + 1 );
	m_indexCount = (2 * ( 2 * m_detail + 1 ) + 2 ) * 5 * m_detail;

	// deallocate any previously allocated buffer
	freeBuffers();

	// allocate memory for buffers
	m_vertexBuffer = new Vector3f[m_vertexCount];
	if( ! m_vertexBuffer ) return;
	m_indexBuffer = new unsigned short[m_indexCount];
	if( ! m_indexBuffer ) return;

	// build vertex buffer
	for( int strip = 0; strip < 5; strip++ )
	for( int column = 1; column < m_detail; column++ )
	for( int row = column; row <= 2 * m_detail + column; row++ )
		computeVertex( strip, column, row );

	for( int strip = 1; strip < 5; strip++ )
	for( int row = 0; row <= 3 * m_detail; row++ )
		computeVertex( strip, 0, row );

	for( int row = 0; row <= 2 * m_detail; row++ )
		computeVertex( 0, 0, row );

	for( int row = m_detail; row <= 3 * m_detail; row++ )
		computeVertex( 4, m_detail, row );

	// build index buffer
	unsigned int i = 0;
	for( int strip = 0; strip < 5; strip++ )
	for( int column = 0; column < m_detail; column++ )
	{
		int row = column;
		m_indexBuffer[i++] = indexOfVertex( strip, column, row );
		for( ; row <= 2 * m_detail + column; row++ )
		{
			m_indexBuffer[i++] =
				indexOfVertex( strip, column, row );
			m_indexBuffer[i++] =
				indexOfVertex( strip, column + 1, row + 1 );
		}
		m_indexBuffer[i++] = indexOfVertex( strip, column + 1,
			2 * m_detail + column + 1);
	}

#ifdef USE_DISPLAY_LISTS
	// compile display list and free buffers
	if( ! m_displayList ) m_displayList = glGenLists( 1 );
	if( ! m_displayList ) return;
	glNewList( m_displayList, GL_COMPILE );
	do_draw();
	glEndList();
	freeBuffers();
#endif
}

unsigned short Sphere::indexOfVertex( int strip, int column, int row)
{
	return ( row + ( 3 * m_detail + 1 ) * ( column + m_detail * strip ) );
}

void Sphere::computeVertex( int strip, int column, int row)
{
	strip %= 5;
	int next_strip = (strip + 1) % 5;

	// the index of the vertex we want to store the result in
	unsigned short index = indexOfVertex( strip, column, row );

	// reference to the vertex we want to store the result in
	Vector3f & vertex = m_vertexBuffer[ index ];

	// the "golden ratio", useful to construct an icosahedron
	const float phi = ( 1 + sqrt((float)5) ) / 2;

	// the 12 vertices of the icosahedron
	const Vector3f northPole( 0, 1, phi );
	const Vector3f northVertices[5] = {
		Vector3f( 0, -1, phi ),
		Vector3f( phi, 0, 1 ),
		Vector3f( 1, phi, 0 ),
		Vector3f( -1, phi, 0 ),
		Vector3f( -phi, 0, 1 ) };
	const Vector3f southVertices[5] = {
		Vector3f( -1, -phi, 0 ),
		Vector3f( 1, -phi, 0 ),
		Vector3f( phi, 0, -1 ),
		Vector3f( 0, 1, -phi ),
		Vector3f( -phi, 0, -1 )
		 };
	const Vector3f southPole( 0, -1, -phi );

	// pointers to the 3 vertices of the face of the icosahedron
	// in which we are
	const Vector3f *v0, *v1, *v2;

	// coordinates of our position inside this face.
	// range from 0 to m_detail.
	int  c1, c2;

	// first, normalize the global coords row, column
	if( row >= 2 * m_detail && column == 0 )
	{
		strip--;
		if( strip < 0 ) strip += 5;
		next_strip--;
		if( next_strip < 0 ) next_strip += 5;
		column = m_detail;
	}

	// next, determine in which face we are, and determine the coords
	// of our position inside this face
	if( row  <= m_detail )
	{
		v0 = &northVertices[strip];
		v1 = &northPole;
		v2 = &northVertices[next_strip];
		c1 = m_detail - row;
		c2 = column;
	}
	else if( row >= 2 * m_detail )
	{
		v0 = &southVertices[next_strip];
		v1 = &southPole;
		v2 = &southVertices[strip];
		c1 = row - 2 * m_detail;
		c2 = m_detail - column;
	}
	else if( row <= m_detail + column )
	{
		v0 = &northVertices[next_strip];
		v1 = &southVertices[next_strip];
		v2 = &northVertices[strip];
		c1 = row - m_detail;
		c2 = m_detail - column;
	}
	else
	{
		v0 = &southVertices[strip];
		v1 = &southVertices[next_strip];
		v2 = &northVertices[strip];
		c1 = column;
		c2 = 2 * m_detail - row;
	}

	// now, compute the actual coords of the vertex
	float u1 = static_cast<float>(c1) / m_detail;
	float u2 = static_cast<float>(c2) / m_detail;
	vertex = *v0 + u1 * ( *v1 - *v0 ) + u2 * ( *v2 - *v0 );

	// project the vertex onto the sphere
	vertex.normalize();
}

void Sphere::setup( int detail )
{
	if( detail == m_detail ) return;
	m_detail = detail;
	initialize();
}

