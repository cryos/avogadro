/**********************************************************************
  Color - Base class for handling color changes in OpenGL

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

#include <QGLWidget> // for GLfloat

namespace Avogadro {

  class Primitive;
  class ColorPrivate; // for future expansion
  
  /**
   * @class Color color.h <avogadro/color.h>
   * @author Benoit Jacob
   * @author Geoff Hutchison
   * @brief Color in OpenGL float red, green, blue, alpha format.
   * 
   * This class represents a color in OpenGL float red, green, blue, alpha format.
   */
  class A_EXPORT Color
  {
  public:
    Color();
    virtual ~Color();

    /**
     * This constructor sets the four components of the color
     * individually. Each one ranges from 0.0 (lowest intensity) to
     * 1.0 (highest intensity). For the alpha component, 0.0 means totally
     * transparent and 1.0 (the default) means totally opaque. */
    Color( GLfloat red, GLfloat green, GLfloat blue,
        GLfloat alpha = 1.0 );

    /**
     * Set the color based on the supplied Primitive.
     * If NULL is passed do nothing. 
     * @param Primitive the color is derived from this primitive.
     */
    Color( const Primitive * );

    /**
     * Set the four components of the color
     * individually. Each one ranges from 0.0 (lowest intensity) to
     * 1.0 (highest intensity). For the alpha component, 0.0 means totally
     * transparent and 1.0 (the default) means totally opaque. */
    virtual void set(GLfloat red, GLfloat green, GLfloat blue,
                     GLfloat alpha = 1.0 );

    /**
     * Set the color based on the supplied Primitive.
     * If NULL is passed do nothing.
     * @param Primitive the color is derived from this primitive.
     */
    virtual void set(const Primitive *);

    /**
     * Set the alpha component of the color, 0.0 means totally transparent and
     * 1.0 means totally opaque.
     */
    virtual void setAlpha(double alpha);

    /**
     * Sets this color to be the one used by OpenGL for rendering
     * when lighting is disabled.
     */
    inline virtual void apply()
    {
      glColor4fv( &m_red );
    }

    /**
     * Applies nice OpenGL materials using this color as the
     * diffuse color while using different shades for the ambient and
     * specular colors. This is only useful if lighting is enabled.
     */
    virtual void applyAsMaterials();
    
    /**
     * Applies an OpenGL material more appropriate for flat surfaces.
     */
    virtual void applyAsFlatMaterials();

    /**
     * @return the red component of the color.
     */
    inline float red() { return m_red; }
    /**
     * @return the green component of the color.
     */
    inline float green() { return m_green; }
    /**
     * @return the blue component of the color.
     */
    inline float blue() { return m_blue; }
    /**
     * @return the alpha component of the color.
     */
    inline float alpha() { return m_alpha; }
    
    /**
     * Set the name of this instance of the class.
     */
    virtual void setName(QString name);
    /**
     * @return the name of this instance of the class.
     */
    virtual QString name() const;
    /**
     * @return the type of the Color class.
     */
    virtual QString type() const { return "Virtual Base Class"; }

    /**
     * @return the widget for controlling settings for this color map
     * or NULL if none exists. */
    virtual QWidget *settingsWidget() { return NULL; }

  protected:
    /**
     * \var GLfloat m_red
     * The red component of the color ranging from 0 to 1.
     */
    /**
     * \var GLfloat m_green
     * The green component of the color ranging from 0 to 1.
     */
    /**
     * \var GLfloat m_blue
     * The blue component of the color ranging from 0 to 1.
     */
    /**
     * \var GLfloat m_alpha
     * The alpha component of the color ranging from 0 to 1.
     */
    GLfloat m_red, m_green, m_blue, m_alpha;
    
    /**
     * The name of the class instance.
     */
    QString m_name;
    
    /**
     * The d-pointer used to preserve binary compatibility.
     */
    ColorPrivate *d;
  };

}

#endif
