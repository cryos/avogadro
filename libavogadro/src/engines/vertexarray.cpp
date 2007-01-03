/***************************************************************************
    copyright            : (C) 2006 by Benoit Jacob
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "vertexarray.h"
#include <math.h>

using namespace Avogadro;
using namespace OpenBabel;

VertexArray::VertexArray( GLenum mode,
	bool hasIndexBuffer,
	bool hasSeparateNormalBuffer )
{
	m_mode = mode;
	m_vertexBuffer = 0;
	m_normalBuffer = 0;
	m_indexBuffer = 0;
	m_displayList = 0;
	m_hasIndexBuffer = hasIndexBuffer;
	m_hasSeparateNormalBuffer = hasSeparateNormalBuffer;
	m_isValid = false;
}

VertexArray::~VertexArray()
{
	freeBuffers();
	if( m_displayList )
		glDeleteLists( m_displayList, 1 );
}

void VertexArray::freeBuffers()
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
	if( m_normalBuffer && m_hasSeparateNormalBuffer ) 
	{
		delete [] m_normalBuffer;
		m_normalBuffer = 0;
	}
}

bool VertexArray::allocateBuffers()
{
	if( m_vertexCount > 65536 ) return false;

	freeBuffers();

	m_vertexBuffer = new Vector[m_vertexCount];
	if( ! m_vertexBuffer ) return false;
	
	if( m_hasSeparateNormalBuffer )
	{
		m_normalBuffer = new Vector[m_vertexCount];
		if( ! m_normalBuffer ) return false;
	}
	else m_normalBuffer = m_vertexBuffer;

	if( m_hasIndexBuffer )
	{
		m_indexBuffer = new unsigned short[m_indexCount];
		if( ! m_indexBuffer ) return false;
	}

	return true;
}

void VertexArray::do_draw()
{
	glVertexPointer( 3, GL_FLOAT, 0, m_vertexBuffer );
	glNormalPointer( GL_FLOAT, 0, m_normalBuffer );
	if( m_hasIndexBuffer )
		glDrawElements( m_mode, m_indexCount,
			GL_UNSIGNED_SHORT, m_indexBuffer );
	else
		glDrawArrays( m_mode, 0, m_vertexCount );
}

void VertexArray::compileDisplayList()
{
#ifdef USE_DISPLAY_LISTS
	if( ! m_displayList )
		m_displayList = glGenLists( 1 );
	if( ! m_displayList ) return;
	
	glNewList( m_displayList, GL_COMPILE );
	do_draw();
	glEndList();

	freeBuffers();
#endif
}

void VertexArray::initialize()
{
	m_isValid = false;
	m_vertexCount = getVertexCount();
	m_indexCount = getIndexCount();
	if( m_indexCount < 0 || m_vertexCount < 0 ) return;
	if( ! allocateBuffers() ) return;
	buildBuffers();
	compileDisplayList();
	m_isValid = true;
}

Sphere::Sphere()
	: VertexArray( GL_TRIANGLE_STRIP, true, false )
{
	m_detail = 0;
}

unsigned short Sphere::indexOfVertex( int strip, int column, int row)
{
	return ( row + ( 3 * m_detail + 1 ) * ( column + m_detail * strip ) );
}

void Sphere::computeVertex( int strip, int column, int row)
{
	strip %= 5;
	int next_strip = (strip + 1) % 5;

	unsigned short index = indexOfVertex( strip, column, row );

	const double phi = ( 1 + sqrt(5) ) / 2;

	const vector3 northPole( 0, 1, phi );
	const vector3 northVertices[5] = {
		vector3( 0, -1, phi ),
		vector3( phi, 0, 1 ),
		vector3( 1, phi, 0 ),
		vector3( -1, phi, 0 ),
		vector3( -phi, 0, 1 ) };
	const vector3 southVertices[5] = {
		vector3( -1, -phi, 0 ),
		vector3( 1, -phi, 0 ),
		vector3( phi, 0, -1 ),
		vector3( 0, 1, -phi ),
		vector3( -phi, 0, -1 )
		 };
	const vector3 southPole( 0, -1, -phi );

	const vector3 *v0, *v1, *v2;
	int  c1, c2;

	if( row >= 2 * m_detail && column == 0 )
	{
		strip--;
		if( strip < 0 ) strip += 5;
		next_strip--;
		if( next_strip < 0 ) next_strip += 5;
		column = m_detail;
	}

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

	double u1 = double(c1) / m_detail;
	double u2 = double(c2) / m_detail;

	vector3 v = *v0 + u1 * ( *v1 - *v0 ) + u2 * ( *v2 - *v0 );
	v.normalize();

	Vector *vertex =
		&m_vertexBuffer[ index ];
	vertex->x = v.x();
	vertex->y = v.y();
	vertex->z = v.z();
}

int Sphere::getVertexCount()
{
	if( m_detail < 1 ) return -1;
	return ( 3 * m_detail + 1 ) * ( 5 * m_detail + 1 );
}

int Sphere::getIndexCount()
{
	if( m_detail < 1 ) return -1;
	return (2 * ( 2 * m_detail + 1 ) + 2 ) * 5 * m_detail;
}

void Sphere::buildBuffers()
{
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
}

void Sphere::setup( int detail )
{
	if( detail == m_detail ) return;
	m_detail = detail;
	initialize();
}

void Sphere::draw( const vector3 &center, double radius )
{
	glPushMatrix();
	glTranslated( center.x(), center.y(), center.z() );
	glScaled( radius, radius, radius );
	VertexArray::draw();
	glPopMatrix();
}

Cylinder::Cylinder()
	: VertexArray( GL_QUAD_STRIP, false, true )
{
	m_faces = 0;
}

void Cylinder::setup( int faces )
{
	if( faces == m_faces ) return;
	m_faces = faces;
	initialize();
}

int Cylinder::getVertexCount()
{
	if( m_faces < 3 ) return -1;
	return 2 * m_faces + 2;
}

void Cylinder::buildBuffers()
{
	for( int i = 0; i <= m_faces; i++ )
	{
		float angle = 2 * M_PI * i / m_faces;
		float x = cosf( angle );
		float y = sinf( angle );

		m_normalBuffer[ 2 * i ].x = x;
		m_normalBuffer[ 2 * i ].y = y;
		m_normalBuffer[ 2 * i ].z = 0.0;

		m_vertexBuffer[ 2 * i ].x = x;
		m_vertexBuffer[ 2 * i ].y = y;
		m_vertexBuffer[ 2 * i ].z = 1.0;

		m_normalBuffer[ 2 * i + 1 ].x = x;
		m_normalBuffer[ 2 * i + 1 ].y = y;
		m_normalBuffer[ 2 * i + 1 ].z = 0.0;

		m_vertexBuffer[ 2 * i + 1 ].x = x;
		m_vertexBuffer[ 2 * i + 1 ].y = y;
		m_vertexBuffer[ 2 * i + 1 ].z = 0.0;
	}
}

void Cylinder::draw( const vector3 &end1, const vector3 &end2,
	double radius, int order, double shift )
{
	// the "axis vector" of the cylinder
	vector3 axis = end2 - end1;
	
	// find two unit vectors v, w such that
	// (axis,v,w) is an orthogonal basis
	vector3 v, w;
	createOrthoBasisGivenFirstVector( axis, v, w );

	// construct the 4D transformation matrix
	GLdouble matrix[16];

	// column 1
	matrix[0] = v.x() * radius;
	matrix[1] = v.y() * radius;
	matrix[2] = v.z() * radius;
	matrix[3] = 0.0;

	// column 2
	matrix[4] = w.x() * radius;
	matrix[5] = w.y() * radius;
	matrix[6] = w.z() * radius;
	matrix[7] = 0.0;

	// column 3
	matrix[8] = axis.x();
	matrix[9] = axis.y();
	matrix[10] = axis.z();
	matrix[11] = 0.0;

	// column 4
	matrix[12] = end1.x();
	matrix[13] = end1.y();
	matrix[14] = end1.z();
	matrix[15] = 1.0;

	//now we can do the actual drawing !
	glPushMatrix();
	glMultMatrixd( matrix );

	if( order == 1 ) VertexArray::draw();
	else for( int i = 0; i < order; i++)
	{
		glPushMatrix();
		glRotated( 360.0 * i / order, 0.0, 0.0, 1.0 );
		glTranslated( shift / radius, 0.0, 0.0 );
		VertexArray::draw();
		glPopMatrix();
	}
	glPopMatrix();
}

bool Avogadro::createOrthoBasisGivenFirstVector
	( const vector3 &U, vector3 & v, vector3 & w )
{
  U.createOrthoVector(v);
  //	if( ! U.createOrthoVector( v ) ) return false;
	w = cross( U, v );
	w.normalize();
	return true;
}
/*
void LinearRegression( const std::list<vector3 *> & points,
	vector3 & ret_plane_base_point, vector3 & ret_plane_normal_vector )
{
	double sum_x = 0.0;
	double sum_y = 0.0;
	double sum_z = 0.0;
	double sum_xx = 0.0;
	double sum_xy = 0.0;
	double sum_xz = 0.0;
	double sum_yy = 0.0;
	double sum_yz = 0.0;
	double sum_zz = 0.0;

	for( std::list<vector3 *>::const_iterator iter = points.begin();
		iter != points.end(); iter++ )
	{
		double x = iter->x();
		double y = iter->y();
		double z = iter->z();
		sum_x += x;
		sum_y += y;
		sum_z += z;
		sum_xx += x * x;
		sum_xy += x * y;
		sum_xz += x * z;
		sum_yy += y * y;
		sum_yz += y * z;
		sum_zz += z * z;
	}

	
}*/
