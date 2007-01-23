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
#include <avogadro/color.h>
#include <math.h>

using namespace Avogadro;
using namespace OpenBabel;

Color::Color( GLfloat red, GLfloat green, GLfloat blue,
                  GLfloat alpha )
{
	m_red = red;
	m_green = green;
	m_blue = blue;
	m_alpha = alpha;
}

Color::Color( const OBAtom* atom )
{
	std::vector<double> rgb = etab.GetRGB( atom->GetAtomicNum() );
	m_red = rgb[0];
	m_green = rgb[1];
	m_blue = rgb[2];
	m_alpha = 1.0;
}

Color& Color::operator=( const Color& other )
{
	m_red = other.m_red;
	m_green = other.m_green;
	m_blue = other.m_blue;
	m_alpha = other.m_alpha;

	return *this;
}

void Color::applyAsMaterials()
{
	GLfloat ambientColor [] = { m_red / 2, m_green / 2, m_blue / 2,
	                            m_alpha };
	GLfloat diffuseColor [] = { m_red, m_green, m_blue, m_alpha };

	float s = ( 0.5 + fabsf( m_red - m_green )
		+ fabsf( m_blue - m_green ) + fabsf( m_blue - m_red ) ) / 4.0;

	float t = 1.0 - s;

	GLfloat specularColor [] = { s + t * m_red,
		s + t * m_green,
		s + t * m_blue,
		m_alpha };

	glMaterialfv( GL_FRONT, GL_AMBIENT, ambientColor );
	glMaterialfv( GL_FRONT, GL_DIFFUSE, diffuseColor );
	glMaterialfv( GL_FRONT, GL_SPECULAR, specularColor );
	glMaterialf( GL_FRONT, GL_SHININESS, 50.0 );
}

