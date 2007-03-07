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

#ifndef __CYLINDER_H
#define __CYLINDER_H

#include <avogadro/global.h>

#include <openbabel/mol.h>
#include <eigen/matrix.h>

namespace Avogadro {

  /**
   * This class represents and draws a cylinder
   *
   * @author Benoit Jacob
   */
  class A_EXPORT Cylinder
  {
    protected:
      /** Pointer to the buffer storing the vertex array */
      Eigen::Vector3f *m_vertexBuffer;
      /** Pointer to the buffer storing the normal array */
      Eigen::Vector3f *m_normalBuffer;
      /** The number of vertices, i.e. the size of m_vertexBuffer
       * or equivalently m_normalBuffer */
      int m_vertexCount;
      /** The id of the OpenGL display list (used only if this option
       * is turned on) */
      GLuint m_displayList;
      /** Equals true if the vertex array has been correctly initialized */
      bool m_isValid;

      /** the number of faces of the cylinder. This only
       * includes the lateral faces, as the base and top faces (the
       * two discs) are not rendered. */
      int m_faces;

      void initialize();
      void freeBuffers();
      void do_draw() const;

    public:
      Cylinder();
      ~Cylinder();
      /** initializes the cylinder with given number of faces. If the
       * cylinder was already initialized, any pre-allocated buffers
       * are freed and then re-allocated */
      void setup( int faces );
      /**
       * draws the cylinder at specified position, with specified
       * radius. the order and shift arguments allow to render
       * multiple cylinders at once, which is useful in libavogadro.
       * for multiple bonds between atoms. If you only want to render one
       * cylinder, leave order and shift at their default values.
       @param end1 the position of the first end of the cylinder.
       that is, the center of the first disc-shaped face.
       @param end2 the position of the second end of the cylinder.
       that is, the center of the second disc-shaped face.
       @param radius the radius of the cylinder
       @param order to render only one cylinder, leave this set to
       the default value, which is 1. If order>1, then order
       parallel cylinders are drawn around the axis
       (end1 - end2).
       @param shift this is only meaningful of order>1, otherwise
       just let this set to the default value. When order>1,
       this is interpreted as the displacement of the axis
       of the drawn cylinders from the axis (end1 - end2).
       */
      void draw( const Eigen::Vector3d &end1, const Eigen::Vector3d &end2,
          double radius, int order = 1, double shift = 0.0 ) const;
  };

}

#endif
