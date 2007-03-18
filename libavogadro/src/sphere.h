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

#ifndef __SPHERE_H
#define __SPHERE_H

#include <avogadro/global.h>

#include <openbabel/mol.h>
#include <eigen/matrix.h>
#include <QGLWidget>

namespace Avogadro {

  /**
   * This class represents and draws a sphere. The sphere is computed as a
   * "geosphere", that is, one starts with an icosahedron, which is the regular
   * solid with 20 triangular faces, and one then sub-tesselates each face into
   * smaller triangles. This is a classical algorithm, known to give very good
   * results.
   *
   * @author Benoit Jacob
   */
  class A_EXPORT Sphere
  {
    protected:
      /** Pointer to the buffer storing the vertex array */
      Eigen::Vector3f *m_vertexBuffer;
      /** Pointer to the buffer storing the indices */
      unsigned short *m_indexBuffer;
      /** The number of vertices, i.e. the size of m_vertexBuffer */
      int m_vertexCount;
      /** The number of indices, i.e. the size of m_indexBuffer */
      int m_indexCount;
      /** The id of the OpenGL display list */
      GLuint m_displayList;

      /** computes the index (position inside the index buffer)
       * of a vertex given by its position (strip, column, row)
       * inside a certain flat model of the sub-tesselated
       * icosahedron */
      inline unsigned short indexOfVertex(
          int strip, int column, int row);
      /** computes the coordinates
       * of a vertex given by its position (strip, column, row)
       * inside a certain flat model of the sub-tesselated
       * icosahedron */
      void computeVertex( int strip, int column, int row );
      /** the detail-level of the sphere. Must be at least 1.
       * This is interpreted as the number of sub-edges into which
       * each edge of the icosahedron must be split. So the
       * number of faces of the sphere is simply:
       * 20 * detail^2. When detail==1, the sphere is just the
       * icosahedron */
      int m_detail;

      void freeBuffers();
      void initialize();
      void do_draw() const;

    public:
      Sphere();
      ~Sphere();

      /** initializes the sphere with given level of detail. If the
       * sphere was already initialized, any pre-allocated buffers
       * are freed and then re-allocated.
       @param detail the wanted level of detail. See m_detail member */
      void setup( int detail );

      /** draws the sphere at specified position and with
       * specified radius */
      void draw( const Eigen::Vector3d &center, double radius ) const;
  };

}

#endif
