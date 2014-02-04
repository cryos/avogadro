/**********************************************************************
  Color3f - Simple color class that uses three floats to represent RGB

  Copyright (C) 2009 Marcus D. Hanwell

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.cc/>

  Avogadro is free software; you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as published by
  the Free Software Foundation; either version 2.1 of the License, or
  (at your option) any later version.

  Avogadro is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.
 **********************************************************************/

#ifndef COLOR3F_H
#define COLOR3F_H

namespace Avogadro {

  /**
   * @class Color3f color3f.h <avogadro/color3f.h>
   * @brief Representation of an RGB color using three floats.
   * @author Marcus D. Hanwell
   *
   * This class represents a color as three floats ranging from 0.0 to 1.0
   * specifying the intensity of the red, green and blue components of the
   * color. It is stored in memory as float[3], and so vectors containing this
   * type may be passed directly to OpenGL and other functions as C arrays.
   *
   * Several convenience functions are provided, the class is written with an
   * emphasis on efficiency and memory layout.
   */

  class Color3f
  {
  public:
    /**
     * Constructor, results in a black Color3f object unless the RGB values are
     * set.
     * @param red Intensity (from 0.0 to 1.0) of the red component of the color.
     * @param green Intensity (from 0.0 to 1.0) of the green component of the color.
     * @param blue Intensity (from 0.0 to 1.0) of the blue component of the color.
     */
    Color3f(float red = 0.0, float green = 0.0, float blue = 0.0);

    /**
     * Constructor where the color is constructed from integer values.
     * @param red Intensity (from 0 to 255) of the red component of the color.
     * @param green Intensity (from 0 to 255) of the green component of the color.
     * @param blue Intensity (from 0 to 255) of the blue component of the color.
     */
    Color3f(int red, int green, int blue);

    /**
     * Sets the color objects components.
     * @param red Intensity (from 0.0 to 1.0) of the red component of the color.
     * @param green Intensity (from 0.0 to 1.0) of the green component of the color.
     * @param blue Intensity (from 0.0 to 1.0) of the blue component of the color.
     */
    void set(float red, float green, float blue);

    /**
     * @return The intensity of the red component of the color (0.0 to 1.0).
     */
    float red() const { return m_data[0]; }

    /**
     * @return The intensity of the green component of the color (0.0 to 1.0).
     */
    float green() const { return m_data[1]; }

    /**
     * @return The intensity of the blue component of the color (0.0 to 1.0).
     */
    float blue() const { return m_data[2]; }

    /**
     * @return Direct access to the underlying float array of size 3.
     */
    float * data();

    /**
     * This function is useful when calling OpenGL functions which expect a
     * float * array of size 3.
     * @return Direct access to the underlying float array of size 3.
     */
    const float * data() const;

  protected:
    float m_data[3];
  };

  inline Color3f::Color3f(float red, float green, float blue)
  {
    m_data[0] = red;
    m_data[1] = green;
    m_data[2] = blue;
  }

  inline Color3f::Color3f(int red, int green, int blue)
  {
    m_data[0] = red / 255.0f;
    m_data[1] = green / 255.0f;
    m_data[2] = blue / 255.0f;
  }

  inline void Color3f::set(float red, float green, float blue)
  {
    m_data[0] = red;
    m_data[1] = green;
    m_data[2] = blue;
  }

  inline float * Color3f::data()
  {
    return &(m_data[0]);
  }

  inline const float * Color3f::data() const
  {
    return &(m_data[0]);
  }

} // End namespace Avogadro

#endif // COLOR3F_H
