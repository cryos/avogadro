/**********************************************************************
  GLHit - detect mouse clicks and the objects hit

  Copyright (C) 2006-2009 Geoffrey R. Hutchison
  Copyright (C) 2006,2007 Donald Ephraim Curtis
  Copyright (C) 2007-2009 Marcus D. Hanwell

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

#ifndef GLHIT_H
#define GLHIT_H

#include <avogadro/global.h>

#ifdef ENABLE_GLSL
  #include <GL/glew.h>
#endif
#include <QGLWidget>

namespace Avogadro {

  /**
   * @class GLHit glwidget.h <avogadro/glhit.h>
   * @brief Hits from OpenGL selections/picking.
   * @author Donald Ephraim Curtis
   *
   * Provides an easy to use class to contain OpenGL hits returned from the
   * process of picking.  This class relies on the %Engine subclasses properly
   * naming the objects that they are rendering.  For more information see the
   * Engine documentation.
   */
  class GLHitPrivate;
  class A_EXPORT GLHit
  {
  public:
    /**
     * Blank constructor.
     */
    GLHit();

    /**
     * Copy constructor.
     */
    GLHit(const GLHit &glHit);

    /**
     * Constructor.
     * @param type The type of the OpenGL object that was picked which corresponds
     * to the Primitive::Type for the object
     * (i.e. type==Primitive::AtomType means an Atom was picked).
     * @param name The name of the OpenGL object that was picked corresponding
     * to the primitive index
     * (i.e. name==1 means Atom 1).
     * @param minZ minimum window Z value of the hit
     * @param maxZ maximum window Z value of the hit
     */
    GLHit(GLuint type, GLuint name, GLuint minZ, GLuint maxZ);

    /**
     * Destructor.
     */
    ~GLHit();

    /**
     * Less than operator.
     * @param other the other GLHit object to compare to.
     * @return (this->minZ < other->minZ) ? @c true : @c false
     */
    bool operator<(const GLHit &other) const;

    /**
     * Equivalence operator.
     * @param other the other GLHit object to test equivalence with.
     * @return returns true if all elements are equivalent (type, name, minZ, maxZ).
     */
    bool operator==(const GLHit &other) const;

    /**
     * Copy operator.
     * @param other the GLHit object to set this object equal to.
     * @return  *this
     */
    GLHit &operator=(const GLHit &other);

    /**
     * @return type of the object that was picked.
     */
    GLuint type() const;

    /**
     * @return name of the object that was picked.
     */
    GLuint name() const;

    /**
     * @return the minimum z value of this hit corresponding
     * to the z value of the drawn object closest to the camera.
     */
    GLuint minZ() const;

    /**
     * @return the maximum z value of this hit corresponding
     * to the z value of the drawn object farthest from the camera.
     */
    GLuint maxZ() const;

    /**
     * @param type new object type.
     */
    void setType(GLuint type);

    /**
     * @param name new object name.
     */
    void setName(GLuint name);

    /**
     * @param minZ minimum z value to set for this object.
     */
    void setMinZ(GLuint minZ);

    /**
     * @param maxZ maximum z value to set for this object.
     */
    void setMaxZ(GLuint maxZ);

  private:
    GLHitPrivate * const d;
  };

} // End namespace Avogadro

#endif // GLHIT_H
