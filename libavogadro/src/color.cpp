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
#include <math.h> // for fabs()

namespace Avogadro {
  
  class ColorPrivate {
  public:
    ColorPrivate() 
    {    }
    
    ~ColorPrivate()
    {    }
  };

  Color::Color(): d(0) {
  }

  Color::~Color() {
  }

  Color::Color( GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha ):
    m_red(red), m_green(green), m_blue(blue), m_alpha(alpha), d(0)
  {  }

  Color::Color( const Primitive *p ): d(0)
  {
    set(p);
  }

  /* Color& Color::operator=( const Color& other )
  {
    m_red = other.m_red;
    m_green = other.m_green;
    m_blue = other.m_blue;
    m_alpha = other.m_alpha;

    return *this;
  }
   */
  
  void Color::set(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
  {
    m_red = red;
    m_green = green;
    m_blue = blue;
    m_alpha = alpha;
  }

  void Color::set(const Primitive *)
  {
    return;
  }

  void Color::setAlpha(double alpha)
  {
    m_alpha = alpha;
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

  void Color::applyAsFlatMaterials()
  {
    GLfloat diffuseColor [] = { m_red, m_green, m_blue, m_alpha };
    
    glMaterialfv( GL_FRONT, GL_AMBIENT, diffuseColor );
    glMaterialfv( GL_FRONT, GL_DIFFUSE, diffuseColor );
    glMaterialfv( GL_FRONT, GL_SPECULAR, diffuseColor );
    glMaterialf( GL_FRONT, GL_SHININESS, 1.0 );
  }

  void Color::setName(QString name)
  {
    m_name = name;
  }
  
  QString Color::name() const
  {
   if (m_name.isEmpty())
     return type();
    else
      return m_name;
  }
}
