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

#ifndef __COLOR_H
#define __COLOR_H

#include <avogadro/global.h>

#include <QGLWidget>
#include <openbabel/mol.h>

namespace Avogadro {

  /**
   * This class represents a color in OpenGL float red-green-blue-alpha format.
   *
   * @author Benoit Jacob
   */
  class A_EXPORT Color
  {
  public:
    Color();
    virtual ~Color();

    /**
     * This constructor sets the four components of the color
     * individually. Each one ranges from 0.0 (lowest intensity) to
     * 1.0 (highest intensity). For the alpha component, 0.0 means fully
     * transparent and 1.0 (the default) means fully opaque. */
    Color( GLfloat red, GLfloat green, GLfloat blue,
        GLfloat alpha = 1.0 );

    /**
     * This constructor uses OpenBabel to retrieve the color in which
     * the atom should be rendered. Default is to render based on element. */
    Color( const OpenBabel::OBAtom *atom );

    /**
     * Equal overloading operator */
    Color &operator=( const Color& other );

    /**
     * Set the four components of the color
     * individually. Each one ranges from 0.0 (lowest intensity) to
     * 1.0 (highest intensity). For the alpha component, 0.0 means fully
     * transparent and 1.0 (the default) means fully opaque. */
    virtual void set(GLfloat red, GLfloat green, GLfloat blue,
                     GLfloat alpha = 1.0 );

    /**
     * Set the color in which the atom should be rendered. 
     * If NULL is passed, do nothing */
    virtual void set(const OpenBabel::OBAtom *atom );

    /**
     * Set the color from an floating-point range 
     * Default will just set white for everything
     */
    virtual void set(double value, double low, double high);

    /**
     * Set the alpha component of the color */
    virtual void setAlpha(double alpha);

    /**
     * Sets this color to be the one used by OpenGL for rendering
     * when lighting is disabled. */
    inline virtual void apply()
    {
      glColor4fv( &m_red );
    }

    /**
     * Applies nice OpenGL materials using this color as the
     * diffuse color while using different shades for the ambient and
     * specular colors. This is only useful if lighting is enabled. */
    virtual void applyAsMaterials();

    inline float red() { return m_red; }
    inline float green() { return m_green; }
    inline float blue() { return m_blue; }
    inline float alpha() { return m_alpha; }

  private:
    ///{ The four components of the color, ranging between 0 and 1.
    GLfloat m_red, m_green, m_blue, m_alpha;
    ///}
  };

}

#endif
