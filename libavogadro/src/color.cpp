/**********************************************************************
  Color - Class for handling color changes in OpenGL

  Copyright (C) 2006 Benoit Jacob
  Copyright (C) 2007 Geoffrey R. Hutchison

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.sourceforge.net/>

  Avogadro is free software; you can redistribute it and/or modify 
  it under the terms of the GNU General Public License as published by 
  the Free Software Foundation; either version 2 of the License, or 
  (at your option) any later version.

  Avogadro is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.
 **********************************************************************/

#include <config.h>

#include <avogadro/color.h>

using namespace OpenBabel;

namespace Avogadro {

  Color::Color() {
  }

  Color::~Color() {
  }

  Color::Color( GLfloat red, GLfloat green, GLfloat blue,
      GLfloat alpha )
  {
    set(red, green, blue, alpha);
  }

  Color::Color( const OBAtom* atom )
  {
    set(atom);
  }

  Color& Color::operator=( const Color& other )
  {
    m_red = other.m_red;
    m_green = other.m_green;
    m_blue = other.m_blue;
    m_alpha = other.m_alpha;

    return *this;
  }

  void Color::set(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
  {
    m_red = red;
    m_green = green;
    m_blue = blue;
    m_alpha = alpha;
  }

  void Color::set(const OpenBabel::OBAtom *atom)
  {
    if (!atom)
      return;

    std::vector<double> rgb = etab.GetRGB( atom->GetAtomicNum() );
    m_red = rgb[0];
    m_green = rgb[1];
    m_blue = rgb[2];
    m_alpha = 1.0;
  }

  void Color::set(double value, double low, double high)
  {
    m_red = m_green = m_blue = m_alpha = 1.0;
  }

  void Color::applyAsMaterials()
  {
    GLfloat ambientColor [] = { m_red / 3, m_green / 3, m_blue / 3,
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

}
