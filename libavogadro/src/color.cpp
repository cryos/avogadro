/**********************************************************************
  Color - Class for handling color changes in OpenGL

  Copyright (C) 2006 Benoit Jacob
  Copyright (C) 2007 Geoffrey R. Hutchison

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.openmolecules.net/>

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

#include "color.h"
#include <cmath> // for fabs()

#ifdef ENABLE_GLSL
  #include <GL/glew.h>
#endif

#include <QGLWidget>


namespace Avogadro {

  using std::fabs;

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
      //delete d;
  }

  Color::Color(float red, float green, float blue, float alpha ) : d(0)
  {
    m_channels[0] = red;
    m_channels[1] = green;
    m_channels[2] = blue;
    m_channels[3] = alpha;
  }

  Color::Color( const Primitive *p ): d(0)
  {
    setFromPrimitive(p);
  }

  void Color::setFromRgba(float red, float green, float blue, float alpha)
  {
    m_channels[0] = red;
    m_channels[1] = green;
    m_channels[2] = blue;
    m_channels[3] = alpha;
  }

  void Color::setToSelectionColor()
  {
    m_channels[0] = 0.3;
    m_channels[1] = 0.6;
    m_channels[2] = 1.0;
    m_channels[3] = 0.7;
  }

  void Color::setFromQColor(const QColor &color)
  {
    m_channels[0] = color.redF();
    m_channels[1] = color.greenF();
    m_channels[2] = color.blueF();
    m_channels[3] = color.alphaF();
  }

  void Color::setFromPrimitive(const Primitive *)
  {
    return;
  }

  void Color::setFromIndex(const unsigned int)
  {
    return;
  }

  void Color::setFromGradient(const double, const double,
                              const double, const double)
  {
    return;
  }

  void Color::setAlpha(double alpha)
  {
    m_channels[3] = alpha;
  }

  void Color::applyAsMaterials()
  {
    float ambientColor [] = { m_channels[0] / 3, m_channels[1] / 3, m_channels[2] / 3,
      m_channels[3] };

    float s = ( 0.5 + fabs( m_channels[0] - m_channels[1] )
        + fabs( m_channels[2] - m_channels[1] )
        + fabs( m_channels[2] - m_channels[0] ) ) / 4.0;

    float t = 1.0 - s;

    float specularColor [] = { s + t * m_channels[0],
      s + t * m_channels[1],
      s + t * m_channels[2],
      m_channels[3] };

    glMaterialfv( GL_FRONT, GL_AMBIENT, ambientColor );
    glMaterialfv( GL_FRONT, GL_DIFFUSE, m_channels );
    glMaterialfv( GL_FRONT, GL_SPECULAR, specularColor );
    glMaterialf( GL_FRONT, GL_SHININESS, 50.0 );
  }

  void Color::applyAsFlatMaterials()
  {
    glMaterialfv( GL_FRONT, GL_AMBIENT, m_channels );
    glMaterialfv( GL_FRONT, GL_DIFFUSE, m_channels );
    glMaterialfv( GL_FRONT, GL_SPECULAR, m_channels );
    glMaterialf( GL_FRONT, GL_SHININESS, 1.0 );
  }

  void Color::setName(const QString& name)
  {
    m_name = name;
  }

  QString Color::name() const
  {
   if (m_name.isEmpty())
     return "Generic Color"; // should never, ever appear
    else
      return m_name;
  }

  QString Color::identifier() const
  {
    return "Generic Color";
  }
  
  void Color::apply()
  {
    glColor4fv(m_channels);
  }
}

#include "color.moc"
