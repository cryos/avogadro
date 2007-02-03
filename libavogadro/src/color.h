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
  struct A_EXPORT Color
  {
    ///{ The four components of the color, ranging between 0 and 1.
    GLfloat m_red, m_green, m_blue, m_alpha;
    ///}

    Color() {}

    /**
     * This constructor sets the four components of the color
     * individually. Each one ranges from 0.0 (lowest intensity) to
     * 1.0 (highest intensity). For the alpha component, 0.0 means fully
     * transparent and 1.0 (the default) means fully opaque. */
    Color( GLfloat red, GLfloat green, GLfloat blue,
        GLfloat alpha = 1.0 );

    /**
     * This constructor uses OpenBabel to retrieve the color in which
     * the atom should be rendered. */
    Color( const OpenBabel::OBAtom *atom );

    /**
     * Sets this color to be the one used by OpenGL for rendering
     * when lighting is disabled. */
    inline void apply()
    {
      glColor4fv( &m_red );
    }

    /**
     * Equal overloading operator */
    Color &operator=( const Color& other );

    /**
     * Applies nice OpenGL materials using this color as the
     * diffuse color while using different shades for the ambient and
     * specular colors. This is only useful if lighting is enabled. */
    void applyAsMaterials();
  };

}

#endif
