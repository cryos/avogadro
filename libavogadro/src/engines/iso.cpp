/****************************************************************************
 **
 ** Copyright (C) 1991-2007 Heinz van Saanen
 **
 ** This file is part of the the function viewer Zhu3D.
 **
 ** This file may be used under the terms of the GNU General Public
 ** License version 2 as published by the Free Software Foundation
 ** and appearing in the file LICENSE.GPL included in the packaging of
 ** this file.
 **
 ** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 ** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 **
 ****************************************************************************/

/**********************************************************************
 Iso - Iso class for displaying surfaces using OpenGL

 Copyright (C) 2008      Marcus D. Hanwell
 Copyright (C) 2008      Tim Vandermeersch

 The original copyright headers are shown above. This source was originally
 part of the Zhu3D project. Subsequent changes were made to adapt the source
 in order to be used in the Avogadro molecular editor project.

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

#include "iso.h"

#include <QDebug>

using namespace std;
using namespace Eigen;
using namespace OpenBabel;

namespace Avogadro
{

  // ****************************************************************************
  // LOCAL CONSTANT/TABLE-STUFF
  // ****************************************************************************

  // Z-Translation, original value was 48.0
  const float IsoGen::fTargetValue __attribute__((aligned(16))) = 0.0f;

  // These tables are used so that everything can be done in little loops that
  // you can look at all at once rather than in pages and pages of unrolled code
  // a2fVertexOffset lists the positions, relative to vertex0, of each of the
  // eight vertices of a cube
  const float IsoGen::a2fVertexOffset[8][3] __attribute__ ((aligned(16))) =
  {
    {0.0f, 0.0f, 0.0f},
    {1.0f, 0.0f, 0.0f},
    {1.0f, 1.0f, 0.0f},
    {0.0f, 1.0f, 0.0f},
    {0.0f, 0.0f, 1.0f},
    {1.0f, 0.0f, 1.0f},
    {1.0f, 1.0f, 1.0f},
    {0.0f, 1.0f, 1.0f}
  };

  const int IsoGen::a2iVertexOffset[8][3] __attribute__ ((aligned(16))) =
  {
    {0, 0, 0},
    {1, 0, 0},
    {1, 1, 0},
    {0, 1, 0},
    {0, 0, 1},
    {1, 0, 1},
    {1, 1, 1},
    {0, 1, 1}
  };

  // a2iEdgeConnection lists the index of the endpoint vertices for each of the
  // twelve edges of the cube
  const long IsoGen::a2iEdgeConnection[12][2] __attribute__((aligned(16))) =
  {
    {0,1},
    {1,2},
    {2,3},
    {3,0},
    {4,5},
    {5,6},
    {6,7},
    {7,4},
    {0,4},
    {1,5},
    {2,6},
    {3,7}
  };

  // a2fEdgeDirection lists the direction vector (vertex1-vertex0) for each edge
  // in the cube
  const float IsoGen::a2fEdgeDirection[12][3] __attribute__((aligned(16))) =
  {
    { 1.0f, 0.0f, 0.0f},
    { 0.0f, 1.0f, 0.0f},
    { -1.0f, 0.0f, 0.0f},
    { 0.0f, -1.0f, 0.0f},
    { 1.0f, 0.0f, 0.0f},
    { 0.0f, 1.0f, 0.0f},
    { -1.0f, 0.0f, 0.0f},
    { 0.0f, -1.0f, 0.0f},
    { 0.0f, 0.0f, 1.0f},
    { 0.0f, 0.0f, 1.0f},
    { 0.0f, 0.0f, 1.0f},
    { 0.0f, 0.0f, 1.0f}
  };

  // a2iTetrahedronEdgeConnection lists the index of the endpoint vertices for
  // each of the six edges of the tetrahedron
  const long IsoGen::a2iTetrahedronEdgeConnection[6][2]
    __attribute__((aligned(16))) =
  {
    {0,1},
    {1,2},
    {2,0},
    {0,3},
    {1,3},
    {2,3}
  };

  // a2iTetrahedronEdgeConnection lists the index of vertices from a cube that
  // made up each of the six tetrahedrons within the cube
  const long IsoGen::a2iTetrahedronsInACube[6][4] __attribute__((aligned(16))) =
  {
    {0,5,1,6},
    {0,1,2,6},
    {0,2,3,6},
    {0,3,7,6},
    {0,7,4,6},
    {0,4,5,6}
  };

  // For any edge, if one vertex is inside of the surface and the other is outside of
  // the surface then the edge intersects the surface
  // For each of the 4 vertices of the tetrahedron can be two possible states: either inside
  // or outside of the surface
  // For any tetrahedron the are 2^4=16 possible sets of vertex states
  // This table lists the edges intersected by the surface for all 16 possible vertex states
  // There are 6 edges.  For each entry in the table, if edge #n is intersected,
      // then bit #n is set to 1
  const long IsoGen::aiTetrahedronEdgeFlags[16] __attribute__((aligned(16))) =
  {
    0x00, 0x0d, 0x13, 0x1e, 0x26, 0x2b, 0x35, 0x38, 0x38, 0x35, 0x2b, 0x26, 0x1e, 0x13, 0x0d, 0x00,
  };

  // For each of the possible vertex states listed in aiTetrahedronEdgeFlags there is a
  // specific triangulation of the edge intersection points.
  // a2iTetrahedronTriangles lists all of them in the form of
  // 0-2 edge triples with the list terminated by the invalid value -1
  const long IsoGen::a2iTetrahedronTriangles[16][7] __attribute__((aligned(16))) =
  {
    {-1, -1, -1, -1, -1, -1, -1},
    {0, 3, 2, -1, -1, -1, -1},
    {0, 1, 4, -1, -1, -1, -1},
    {1, 4, 2, 2, 4, 3, -1},

    {1, 2, 5, -1, -1, -1, -1},
    {0, 3, 5, 0, 5, 1, -1},
    {0, 2, 5, 0, 5, 4, -1},
    {5, 4, 3, -1, -1, -1, -1},

    {3, 4, 5, -1, -1, -1, -1},
    {4, 5, 0, 5, 2, 0, -1},
    {1, 5, 0, 5, 3, 0, -1},
    {5, 2, 1, -1, -1, -1, -1},

    {3, 4, 2, 2, 4, 1, -1},
    {4, 1, 0, -1, -1, -1, -1},
    {2, 3, 0, -1, -1, -1, -1},
    {-1, -1, -1, -1, -1, -1, -1},
  };

  // For any edge, if one vertex is inside of the surface and the other is outside of the surface
  // then the edge intersects the surface
  // For each of the 8 cube-vertices can be two possible states : either inside or outside of the surface
  // For any cube the are 2^8=256 possible sets of vertex states
  // This table lists the edges intersected by the surface for all 256 possible vertex states
  // There are 12 edges.  For each entry in the table, if edge #n is intersected, then bit #n is set to 1
  const long IsoGen::aiCubeEdgeFlags[256] __attribute__((aligned(16))) =
  {
    0x000, 0x109, 0x203, 0x30a, 0x406, 0x50f, 0x605, 0x70c, 0x80c, 0x905, 0xa0f, 0xb06, 0xc0a, 0xd03, 0xe09, 0xf00,
    0x190, 0x099, 0x393, 0x29a, 0x596, 0x49f, 0x795, 0x69c, 0x99c, 0x895, 0xb9f, 0xa96, 0xd9a, 0xc93, 0xf99, 0xe90,
    0x230, 0x339, 0x033, 0x13a, 0x636, 0x73f, 0x435, 0x53c, 0xa3c, 0xb35, 0x83f, 0x936, 0xe3a, 0xf33, 0xc39, 0xd30,
    0x3a0, 0x2a9, 0x1a3, 0x0aa, 0x7a6, 0x6af, 0x5a5, 0x4ac, 0xbac, 0xaa5, 0x9af, 0x8a6, 0xfaa, 0xea3, 0xda9, 0xca0,
    0x460, 0x569, 0x663, 0x76a, 0x066, 0x16f, 0x265, 0x36c, 0xc6c, 0xd65, 0xe6f, 0xf66, 0x86a, 0x963, 0xa69, 0xb60,
    0x5f0, 0x4f9, 0x7f3, 0x6fa, 0x1f6, 0x0ff, 0x3f5, 0x2fc, 0xdfc, 0xcf5, 0xfff, 0xef6, 0x9fa, 0x8f3, 0xbf9, 0xaf0,
    0x650, 0x759, 0x453, 0x55a, 0x256, 0x35f, 0x055, 0x15c, 0xe5c, 0xf55, 0xc5f, 0xd56, 0xa5a, 0xb53, 0x859, 0x950,
    0x7c0, 0x6c9, 0x5c3, 0x4ca, 0x3c6, 0x2cf, 0x1c5, 0x0cc, 0xfcc, 0xec5, 0xdcf, 0xcc6, 0xbca, 0xac3, 0x9c9, 0x8c0,
    0x8c0, 0x9c9, 0xac3, 0xbca, 0xcc6, 0xdcf, 0xec5, 0xfcc, 0x0cc, 0x1c5, 0x2cf, 0x3c6, 0x4ca, 0x5c3, 0x6c9, 0x7c0,
    0x950, 0x859, 0xb53, 0xa5a, 0xd56, 0xc5f, 0xf55, 0xe5c, 0x15c, 0x055, 0x35f, 0x256, 0x55a, 0x453, 0x759, 0x650,
    0xaf0, 0xbf9, 0x8f3, 0x9fa, 0xef6, 0xfff, 0xcf5, 0xdfc, 0x2fc, 0x3f5, 0x0ff, 0x1f6, 0x6fa, 0x7f3, 0x4f9, 0x5f0,
    0xb60, 0xa69, 0x963, 0x86a, 0xf66, 0xe6f, 0xd65, 0xc6c, 0x36c, 0x265, 0x16f, 0x066, 0x76a, 0x663, 0x569, 0x460,
    0xca0, 0xda9, 0xea3, 0xfaa, 0x8a6, 0x9af, 0xaa5, 0xbac, 0x4ac, 0x5a5, 0x6af, 0x7a6, 0x0aa, 0x1a3, 0x2a9, 0x3a0,
    0xd30, 0xc39, 0xf33, 0xe3a, 0x936, 0x83f, 0xb35, 0xa3c, 0x53c, 0x435, 0x73f, 0x636, 0x13a, 0x033, 0x339, 0x230,
    0xe90, 0xf99, 0xc93, 0xd9a, 0xa96, 0xb9f, 0x895, 0x99c, 0x69c, 0x795, 0x49f, 0x596, 0x29a, 0x393, 0x099, 0x190,
    0xf00, 0xe09, 0xd03, 0xc0a, 0xb06, 0xa0f, 0x905, 0x80c, 0x70c, 0x605, 0x50f, 0x406, 0x30a, 0x203, 0x109, 0x000
  };

  // For each of the possible vertex states listed in aiCubeEdgeFlags there is a specific
  // triangulation of the edge intersection points.  a2iTriangleConnectionTable lists all
  // of them in the form of 0-5 edge triples with the list terminated by the invalid value -1
  //
  // For example: a2iTriangleConnectionTable[3] list the 2 triangles formed when corner[0]
  // and corner[1] are inside of the surface, but the rest of the cube is not
  const long IsoGen::a2iTriangleConnectionTable[256][16]
    __attribute__((aligned(16))) =
  {
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {0, 8, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {0, 1, 9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {1, 8, 3, 9, 8, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {1, 2, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {0, 8, 3, 1, 2, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {9, 2, 10, 0, 2, 9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {2, 8, 3, 2, 10, 8, 10, 9, 8, -1, -1, -1, -1, -1, -1, -1},
    {3, 11, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {0, 11, 2, 8, 11, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {1, 9, 0, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {1, 11, 2, 1, 9, 11, 9, 8, 11, -1, -1, -1, -1, -1, -1, -1},
    {3, 10, 1, 11, 10, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {0, 10, 1, 0, 8, 10, 8, 11, 10, -1, -1, -1, -1, -1, -1, -1},
    {3, 9, 0, 3, 11, 9, 11, 10, 9, -1, -1, -1, -1, -1, -1, -1},
    {9, 8, 10, 10, 8, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {4, 7, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {4, 3, 0, 7, 3, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {0, 1, 9, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {4, 1, 9, 4, 7, 1, 7, 3, 1, -1, -1, -1, -1, -1, -1, -1},
    {1, 2, 10, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {3, 4, 7, 3, 0, 4, 1, 2, 10, -1, -1, -1, -1, -1, -1, -1},
    {9, 2, 10, 9, 0, 2, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1},
    {2, 10, 9, 2, 9, 7, 2, 7, 3, 7, 9, 4, -1, -1, -1, -1},
    {8, 4, 7, 3, 11, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {11, 4, 7, 11, 2, 4, 2, 0, 4, -1, -1, -1, -1, -1, -1, -1},
    {9, 0, 1, 8, 4, 7, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1},
    {4, 7, 11, 9, 4, 11, 9, 11, 2, 9, 2, 1, -1, -1, -1, -1},
    {3, 10, 1, 3, 11, 10, 7, 8, 4, -1, -1, -1, -1, -1, -1, -1},
    {1, 11, 10, 1, 4, 11, 1, 0, 4, 7, 11, 4, -1, -1, -1, -1},
    {4, 7, 8, 9, 0, 11, 9, 11, 10, 11, 0, 3, -1, -1, -1, -1},
    {4, 7, 11, 4, 11, 9, 9, 11, 10, -1, -1, -1, -1, -1, -1, -1},
    {9, 5, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {9, 5, 4, 0, 8, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {0, 5, 4, 1, 5, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {8, 5, 4, 8, 3, 5, 3, 1, 5, -1, -1, -1, -1, -1, -1, -1},
    {1, 2, 10, 9, 5, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {3, 0, 8, 1, 2, 10, 4, 9, 5, -1, -1, -1, -1, -1, -1, -1},
    {5, 2, 10, 5, 4, 2, 4, 0, 2, -1, -1, -1, -1, -1, -1, -1},
    {2, 10, 5, 3, 2, 5, 3, 5, 4, 3, 4, 8, -1, -1, -1, -1},
    {9, 5, 4, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {0, 11, 2, 0, 8, 11, 4, 9, 5, -1, -1, -1, -1, -1, -1, -1},
    {0, 5, 4, 0, 1, 5, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1},
    {2, 1, 5, 2, 5, 8, 2, 8, 11, 4, 8, 5, -1, -1, -1, -1},
    {10, 3, 11, 10, 1, 3, 9, 5, 4, -1, -1, -1, -1, -1, -1, -1},
    {4, 9, 5, 0, 8, 1, 8, 10, 1, 8, 11, 10, -1, -1, -1, -1},
    {5, 4, 0, 5, 0, 11, 5, 11, 10, 11, 0, 3, -1, -1, -1, -1},
    {5, 4, 8, 5, 8, 10, 10, 8, 11, -1, -1, -1, -1, -1, -1, -1},
    {9, 7, 8, 5, 7, 9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {9, 3, 0, 9, 5, 3, 5, 7, 3, -1, -1, -1, -1, -1, -1, -1},
    {0, 7, 8, 0, 1, 7, 1, 5, 7, -1, -1, -1, -1, -1, -1, -1},
    {1, 5, 3, 3, 5, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {9, 7, 8, 9, 5, 7, 10, 1, 2, -1, -1, -1, -1, -1, -1, -1},
    {10, 1, 2, 9, 5, 0, 5, 3, 0, 5, 7, 3, -1, -1, -1, -1},
    {8, 0, 2, 8, 2, 5, 8, 5, 7, 10, 5, 2, -1, -1, -1, -1},
    {2, 10, 5, 2, 5, 3, 3, 5, 7, -1, -1, -1, -1, -1, -1, -1},
    {7, 9, 5, 7, 8, 9, 3, 11, 2, -1, -1, -1, -1, -1, -1, -1},
    {9, 5, 7, 9, 7, 2, 9, 2, 0, 2, 7, 11, -1, -1, -1, -1},
    {2, 3, 11, 0, 1, 8, 1, 7, 8, 1, 5, 7, -1, -1, -1, -1},
    {11, 2, 1, 11, 1, 7, 7, 1, 5, -1, -1, -1, -1, -1, -1, -1},
    {9, 5, 8, 8, 5, 7, 10, 1, 3, 10, 3, 11, -1, -1, -1, -1},
    {5, 7, 0, 5, 0, 9, 7, 11, 0, 1, 0, 10, 11, 10, 0, -1},
    {11, 10, 0, 11, 0, 3, 10, 5, 0, 8, 0, 7, 5, 7, 0, -1},
    {11, 10, 5, 7, 11, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {10, 6, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {0, 8, 3, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {9, 0, 1, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {1, 8, 3, 1, 9, 8, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1},
    {1, 6, 5, 2, 6, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {1, 6, 5, 1, 2, 6, 3, 0, 8, -1, -1, -1, -1, -1, -1, -1},
    {9, 6, 5, 9, 0, 6, 0, 2, 6, -1, -1, -1, -1, -1, -1, -1},
    {5, 9, 8, 5, 8, 2, 5, 2, 6, 3, 2, 8, -1, -1, -1, -1},
    {2, 3, 11, 10, 6, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {11, 0, 8, 11, 2, 0, 10, 6, 5, -1, -1, -1, -1, -1, -1, -1},
    {0, 1, 9, 2, 3, 11, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1},
    {5, 10, 6, 1, 9, 2, 9, 11, 2, 9, 8, 11, -1, -1, -1, -1},
    {6, 3, 11, 6, 5, 3, 5, 1, 3, -1, -1, -1, -1, -1, -1, -1},
    {0, 8, 11, 0, 11, 5, 0, 5, 1, 5, 11, 6, -1, -1, -1, -1},
    {3, 11, 6, 0, 3, 6, 0, 6, 5, 0, 5, 9, -1, -1, -1, -1},
    {6, 5, 9, 6, 9, 11, 11, 9, 8, -1, -1, -1, -1, -1, -1, -1},
    {5, 10, 6, 4, 7, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {4, 3, 0, 4, 7, 3, 6, 5, 10, -1, -1, -1, -1, -1, -1, -1},
    {1, 9, 0, 5, 10, 6, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1},
    {10, 6, 5, 1, 9, 7, 1, 7, 3, 7, 9, 4, -1, -1, -1, -1},
    {6, 1, 2, 6, 5, 1, 4, 7, 8, -1, -1, -1, -1, -1, -1, -1},
    {1, 2, 5, 5, 2, 6, 3, 0, 4, 3, 4, 7, -1, -1, -1, -1},
    {8, 4, 7, 9, 0, 5, 0, 6, 5, 0, 2, 6, -1, -1, -1, -1},
    {7, 3, 9, 7, 9, 4, 3, 2, 9, 5, 9, 6, 2, 6, 9, -1},
    {3, 11, 2, 7, 8, 4, 10, 6, 5, -1, -1, -1, -1, -1, -1, -1},
    {5, 10, 6, 4, 7, 2, 4, 2, 0, 2, 7, 11, -1, -1, -1, -1},
    {0, 1, 9, 4, 7, 8, 2, 3, 11, 5, 10, 6, -1, -1, -1, -1},
    {9, 2, 1, 9, 11, 2, 9, 4, 11, 7, 11, 4, 5, 10, 6, -1},
    {8, 4, 7, 3, 11, 5, 3, 5, 1, 5, 11, 6, -1, -1, -1, -1},
    {5, 1, 11, 5, 11, 6, 1, 0, 11, 7, 11, 4, 0, 4, 11, -1},
    {0, 5, 9, 0, 6, 5, 0, 3, 6, 11, 6, 3, 8, 4, 7, -1},
    {6, 5, 9, 6, 9, 11, 4, 7, 9, 7, 11, 9, -1, -1, -1, -1},
    {10, 4, 9, 6, 4, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {4, 10, 6, 4, 9, 10, 0, 8, 3, -1, -1, -1, -1, -1, -1, -1},
    {10, 0, 1, 10, 6, 0, 6, 4, 0, -1, -1, -1, -1, -1, -1, -1},
    {8, 3, 1, 8, 1, 6, 8, 6, 4, 6, 1, 10, -1, -1, -1, -1},
    {1, 4, 9, 1, 2, 4, 2, 6, 4, -1, -1, -1, -1, -1, -1, -1},
    {3, 0, 8, 1, 2, 9, 2, 4, 9, 2, 6, 4, -1, -1, -1, -1},
    {0, 2, 4, 4, 2, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {8, 3, 2, 8, 2, 4, 4, 2, 6, -1, -1, -1, -1, -1, -1, -1},
    {10, 4, 9, 10, 6, 4, 11, 2, 3, -1, -1, -1, -1, -1, -1, -1},
    {0, 8, 2, 2, 8, 11, 4, 9, 10, 4, 10, 6, -1, -1, -1, -1},
    {3, 11, 2, 0, 1, 6, 0, 6, 4, 6, 1, 10, -1, -1, -1, -1},
    {6, 4, 1, 6, 1, 10, 4, 8, 1, 2, 1, 11, 8, 11, 1, -1},
    {9, 6, 4, 9, 3, 6, 9, 1, 3, 11, 6, 3, -1, -1, -1, -1},
    {8, 11, 1, 8, 1, 0, 11, 6, 1, 9, 1, 4, 6, 4, 1, -1},
    {3, 11, 6, 3, 6, 0, 0, 6, 4, -1, -1, -1, -1, -1, -1, -1},
    {6, 4, 8, 11, 6, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {7, 10, 6, 7, 8, 10, 8, 9, 10, -1, -1, -1, -1, -1, -1, -1},
    {0, 7, 3, 0, 10, 7, 0, 9, 10, 6, 7, 10, -1, -1, -1, -1},
    {10, 6, 7, 1, 10, 7, 1, 7, 8, 1, 8, 0, -1, -1, -1, -1},
    {10, 6, 7, 10, 7, 1, 1, 7, 3, -1, -1, -1, -1, -1, -1, -1},
    {1, 2, 6, 1, 6, 8, 1, 8, 9, 8, 6, 7, -1, -1, -1, -1},
    {2, 6, 9, 2, 9, 1, 6, 7, 9, 0, 9, 3, 7, 3, 9, -1},
    {7, 8, 0, 7, 0, 6, 6, 0, 2, -1, -1, -1, -1, -1, -1, -1},
    {7, 3, 2, 6, 7, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {2, 3, 11, 10, 6, 8, 10, 8, 9, 8, 6, 7, -1, -1, -1, -1},
    {2, 0, 7, 2, 7, 11, 0, 9, 7, 6, 7, 10, 9, 10, 7, -1},
    {1, 8, 0, 1, 7, 8, 1, 10, 7, 6, 7, 10, 2, 3, 11, -1},
    {11, 2, 1, 11, 1, 7, 10, 6, 1, 6, 7, 1, -1, -1, -1, -1},
    {8, 9, 6, 8, 6, 7, 9, 1, 6, 11, 6, 3, 1, 3, 6, -1},
    {0, 9, 1, 11, 6, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {7, 8, 0, 7, 0, 6, 3, 11, 0, 11, 6, 0, -1, -1, -1, -1},
    {7, 11, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {7, 6, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {3, 0, 8, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {0, 1, 9, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {8, 1, 9, 8, 3, 1, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1},
    {10, 1, 2, 6, 11, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {1, 2, 10, 3, 0, 8, 6, 11, 7, -1, -1, -1, -1, -1, -1, -1},
    {2, 9, 0, 2, 10, 9, 6, 11, 7, -1, -1, -1, -1, -1, -1, -1},
    {6, 11, 7, 2, 10, 3, 10, 8, 3, 10, 9, 8, -1, -1, -1, -1},
    {7, 2, 3, 6, 2, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {7, 0, 8, 7, 6, 0, 6, 2, 0, -1, -1, -1, -1, -1, -1, -1},
    {2, 7, 6, 2, 3, 7, 0, 1, 9, -1, -1, -1, -1, -1, -1, -1},
    {1, 6, 2, 1, 8, 6, 1, 9, 8, 8, 7, 6, -1, -1, -1, -1},
    {10, 7, 6, 10, 1, 7, 1, 3, 7, -1, -1, -1, -1, -1, -1, -1},
    {10, 7, 6, 1, 7, 10, 1, 8, 7, 1, 0, 8, -1, -1, -1, -1},
    {0, 3, 7, 0, 7, 10, 0, 10, 9, 6, 10, 7, -1, -1, -1, -1},
    {7, 6, 10, 7, 10, 8, 8, 10, 9, -1, -1, -1, -1, -1, -1, -1},
    {6, 8, 4, 11, 8, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {3, 6, 11, 3, 0, 6, 0, 4, 6, -1, -1, -1, -1, -1, -1, -1},
    {8, 6, 11, 8, 4, 6, 9, 0, 1, -1, -1, -1, -1, -1, -1, -1},
    {9, 4, 6, 9, 6, 3, 9, 3, 1, 11, 3, 6, -1, -1, -1, -1},
    {6, 8, 4, 6, 11, 8, 2, 10, 1, -1, -1, -1, -1, -1, -1, -1},
    {1, 2, 10, 3, 0, 11, 0, 6, 11, 0, 4, 6, -1, -1, -1, -1},
    {4, 11, 8, 4, 6, 11, 0, 2, 9, 2, 10, 9, -1, -1, -1, -1},
    {10, 9, 3, 10, 3, 2, 9, 4, 3, 11, 3, 6, 4, 6, 3, -1},
    {8, 2, 3, 8, 4, 2, 4, 6, 2, -1, -1, -1, -1, -1, -1, -1},
    {0, 4, 2, 4, 6, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {1, 9, 0, 2, 3, 4, 2, 4, 6, 4, 3, 8, -1, -1, -1, -1},
    {1, 9, 4, 1, 4, 2, 2, 4, 6, -1, -1, -1, -1, -1, -1, -1},
    {8, 1, 3, 8, 6, 1, 8, 4, 6, 6, 10, 1, -1, -1, -1, -1},
    {10, 1, 0, 10, 0, 6, 6, 0, 4, -1, -1, -1, -1, -1, -1, -1},
    {4, 6, 3, 4, 3, 8, 6, 10, 3, 0, 3, 9, 10, 9, 3, -1},
    {10, 9, 4, 6, 10, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {4, 9, 5, 7, 6, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {0, 8, 3, 4, 9, 5, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1},
    {5, 0, 1, 5, 4, 0, 7, 6, 11, -1, -1, -1, -1, -1, -1, -1},
    {11, 7, 6, 8, 3, 4, 3, 5, 4, 3, 1, 5, -1, -1, -1, -1},
    {9, 5, 4, 10, 1, 2, 7, 6, 11, -1, -1, -1, -1, -1, -1, -1},
    {6, 11, 7, 1, 2, 10, 0, 8, 3, 4, 9, 5, -1, -1, -1, -1},
    {7, 6, 11, 5, 4, 10, 4, 2, 10, 4, 0, 2, -1, -1, -1, -1},
    {3, 4, 8, 3, 5, 4, 3, 2, 5, 10, 5, 2, 11, 7, 6, -1},
    {7, 2, 3, 7, 6, 2, 5, 4, 9, -1, -1, -1, -1, -1, -1, -1},
    {9, 5, 4, 0, 8, 6, 0, 6, 2, 6, 8, 7, -1, -1, -1, -1},
    {3, 6, 2, 3, 7, 6, 1, 5, 0, 5, 4, 0, -1, -1, -1, -1},
    {6, 2, 8, 6, 8, 7, 2, 1, 8, 4, 8, 5, 1, 5, 8, -1},
    {9, 5, 4, 10, 1, 6, 1, 7, 6, 1, 3, 7, -1, -1, -1, -1},
    {1, 6, 10, 1, 7, 6, 1, 0, 7, 8, 7, 0, 9, 5, 4, -1},
    {4, 0, 10, 4, 10, 5, 0, 3, 10, 6, 10, 7, 3, 7, 10, -1},
    {7, 6, 10, 7, 10, 8, 5, 4, 10, 4, 8, 10, -1, -1, -1, -1},
    {6, 9, 5, 6, 11, 9, 11, 8, 9, -1, -1, -1, -1, -1, -1, -1},
    {3, 6, 11, 0, 6, 3, 0, 5, 6, 0, 9, 5, -1, -1, -1, -1},
    {0, 11, 8, 0, 5, 11, 0, 1, 5, 5, 6, 11, -1, -1, -1, -1},
    {6, 11, 3, 6, 3, 5, 5, 3, 1, -1, -1, -1, -1, -1, -1, -1},
    {1, 2, 10, 9, 5, 11, 9, 11, 8, 11, 5, 6, -1, -1, -1, -1},
    {0, 11, 3, 0, 6, 11, 0, 9, 6, 5, 6, 9, 1, 2, 10, -1},
    {11, 8, 5, 11, 5, 6, 8, 0, 5, 10, 5, 2, 0, 2, 5, -1},
    {6, 11, 3, 6, 3, 5, 2, 10, 3, 10, 5, 3, -1, -1, -1, -1},
    {5, 8, 9, 5, 2, 8, 5, 6, 2, 3, 8, 2, -1, -1, -1, -1},
    {9, 5, 6, 9, 6, 0, 0, 6, 2, -1, -1, -1, -1, -1, -1, -1},
    {1, 5, 8, 1, 8, 0, 5, 6, 8, 3, 8, 2, 6, 2, 8, -1},
    {1, 5, 6, 2, 1, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {1, 3, 6, 1, 6, 10, 3, 8, 6, 5, 6, 9, 8, 9, 6, -1},
    {10, 1, 0, 10, 0, 6, 9, 5, 0, 5, 6, 0, -1, -1, -1, -1},
    {0, 3, 8, 5, 6, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {10, 5, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {11, 5, 10, 7, 5, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {11, 5, 10, 11, 7, 5, 8, 3, 0, -1, -1, -1, -1, -1, -1, -1},
    {5, 11, 7, 5, 10, 11, 1, 9, 0, -1, -1, -1, -1, -1, -1, -1},
    {10, 7, 5, 10, 11, 7, 9, 8, 1, 8, 3, 1, -1, -1, -1, -1},
    {11, 1, 2, 11, 7, 1, 7, 5, 1, -1, -1, -1, -1, -1, -1, -1},
    {0, 8, 3, 1, 2, 7, 1, 7, 5, 7, 2, 11, -1, -1, -1, -1},
    {9, 7, 5, 9, 2, 7, 9, 0, 2, 2, 11, 7, -1, -1, -1, -1},
    {7, 5, 2, 7, 2, 11, 5, 9, 2, 3, 2, 8, 9, 8, 2, -1},
    {2, 5, 10, 2, 3, 5, 3, 7, 5, -1, -1, -1, -1, -1, -1, -1},
    {8, 2, 0, 8, 5, 2, 8, 7, 5, 10, 2, 5, -1, -1, -1, -1},
    {9, 0, 1, 5, 10, 3, 5, 3, 7, 3, 10, 2, -1, -1, -1, -1},
    {9, 8, 2, 9, 2, 1, 8, 7, 2, 10, 2, 5, 7, 5, 2, -1},
    {1, 3, 5, 3, 7, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {0, 8, 7, 0, 7, 1, 1, 7, 5, -1, -1, -1, -1, -1, -1, -1},
    {9, 0, 3, 9, 3, 5, 5, 3, 7, -1, -1, -1, -1, -1, -1, -1},
    {9, 8, 7, 5, 9, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {5, 8, 4, 5, 10, 8, 10, 11, 8, -1, -1, -1, -1, -1, -1, -1},
    {5, 0, 4, 5, 11, 0, 5, 10, 11, 11, 3, 0, -1, -1, -1, -1},
    {0, 1, 9, 8, 4, 10, 8, 10, 11, 10, 4, 5, -1, -1, -1, -1},
    {10, 11, 4, 10, 4, 5, 11, 3, 4, 9, 4, 1, 3, 1, 4, -1},
    {2, 5, 1, 2, 8, 5, 2, 11, 8, 4, 5, 8, -1, -1, -1, -1},
    {0, 4, 11, 0, 11, 3, 4, 5, 11, 2, 11, 1, 5, 1, 11, -1},
    {0, 2, 5, 0, 5, 9, 2, 11, 5, 4, 5, 8, 11, 8, 5, -1},
    {9, 4, 5, 2, 11, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {2, 5, 10, 3, 5, 2, 3, 4, 5, 3, 8, 4, -1, -1, -1, -1},
    {5, 10, 2, 5, 2, 4, 4, 2, 0, -1, -1, -1, -1, -1, -1, -1},
    {3, 10, 2, 3, 5, 10, 3, 8, 5, 4, 5, 8, 0, 1, 9, -1},
    {5, 10, 2, 5, 2, 4, 1, 9, 2, 9, 4, 2, -1, -1, -1, -1},
    {8, 4, 5, 8, 5, 3, 3, 5, 1, -1, -1, -1, -1, -1, -1, -1},
    {0, 4, 5, 1, 0, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {8, 4, 5, 8, 5, 3, 9, 0, 5, 0, 3, 5, -1, -1, -1, -1},
    {9, 4, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {4, 11, 7, 4, 9, 11, 9, 10, 11, -1, -1, -1, -1, -1, -1, -1},
    {0, 8, 3, 4, 9, 7, 9, 11, 7, 9, 10, 11, -1, -1, -1, -1},
    {1, 10, 11, 1, 11, 4, 1, 4, 0, 7, 4, 11, -1, -1, -1, -1},
    {3, 1, 4, 3, 4, 8, 1, 10, 4, 7, 4, 11, 10, 11, 4, -1},
    {4, 11, 7, 9, 11, 4, 9, 2, 11, 9, 1, 2, -1, -1, -1, -1},
    {9, 7, 4, 9, 11, 7, 9, 1, 11, 2, 11, 1, 0, 8, 3, -1},
    {11, 7, 4, 11, 4, 2, 2, 4, 0, -1, -1, -1, -1, -1, -1, -1},
    {11, 7, 4, 11, 4, 2, 8, 3, 4, 3, 2, 4, -1, -1, -1, -1},
    {2, 9, 10, 2, 7, 9, 2, 3, 7, 7, 4, 9, -1, -1, -1, -1},
    {9, 10, 7, 9, 7, 4, 10, 2, 7, 8, 7, 0, 2, 0, 7, -1},
    {3, 7, 10, 3, 10, 2, 7, 4, 10, 1, 10, 0, 4, 0, 10, -1},
    {1, 10, 2, 8, 7, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {4, 9, 1, 4, 1, 7, 7, 1, 3, -1, -1, -1, -1, -1, -1, -1},
    {4, 9, 1, 4, 1, 7, 0, 8, 1, 8, 7, 1, -1, -1, -1, -1},
    {4, 0, 3, 7, 4, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {4, 8, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {9, 10, 8, 10, 11, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {3, 0, 9, 3, 9, 11, 11, 9, 10, -1, -1, -1, -1, -1, -1, -1},
    {0, 1, 10, 0, 10, 8, 8, 10, 11, -1, -1, -1, -1, -1, -1, -1},
    {3, 1, 10, 11, 3, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {1, 2, 11, 1, 11, 9, 9, 11, 8, -1, -1, -1, -1, -1, -1, -1},
    {3, 0, 9, 3, 9, 11, 1, 2, 9, 2, 11, 9, -1, -1, -1, -1},
    {0, 2, 11, 8, 0, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {3, 2, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {2, 3, 8, 2, 8, 10, 10, 8, 9, -1, -1, -1, -1, -1, -1, -1},
    {9, 10, 2, 0, 9, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {2, 3, 8, 2, 8, 10, 0, 1, 8, 1, 10, 8, -1, -1, -1, -1},
    {1, 10, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {1, 3, 8, 9, 1, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {0, 9, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {0, 3, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}
  };

  // ****************************************************************************
  // THREAD RELATED STUFF
  // ****************************************************************************

  // The heavy worker thread
  void IsoGen::run()
  {
    qDebug() << "start run()";
    int nx, ny, nz;

    if (m_grid->grid() == 0)
    {
      qDebug() << "m_grid == 0 => returning...";
      return;
    }

    if (!m_mutex.tryLock())
      return;

    // Clear vertex/normal-lists
    m_normList.clear();
    m_vertList.clear();

    // Interpolate if m_interpolate is true, otherwise use the faster access
    // methods available if we are iterating over the actual grid points.
    if (m_interpolate)
    {
      // Work out the number of steps needed to cover the cube
      nx = (m_max.x() - m_min.x()) / m_stepSize;
      ny = (m_max.y() - m_min.y()) / m_stepSize;
      nz = (m_max.z() - m_min.z()) / m_stepSize;

      // Just go to nX-1 as the cube extends one unit from the given point
      for(int x = 0; x < nx-1; ++x)
        for(int y = 0; y < ny-1; ++y)
          for(int z = 0; z < nz-1; ++z)
            vMarchCube1(m_min.x()+x*m_stepSize,
                        m_min.y()+y*m_stepSize,
                        m_min.z()+z*m_stepSize);

    }
    else
    {
      // Don't do any interpolation...
      m_grid->grid()->GetNumberOfPoints(nx, ny, nz);
      m_stepSize = (m_max.x() - m_min.x()) / nx;

      // Just go to nX-1 as the cube extends one unit from the given point
      for(int i = 0; i < nx-1; ++i)
        for(int j = 0; j < ny-1; ++j)
          for(int k = 0; k < nz-1; ++k)
            vMarchCube1(i, j, k);
    }

    // Save previous vertex/normal-lists for rendering
    m_normListCopy = m_normList;
    m_vertListCopy = m_vertList;

    m_mutex.unlock();
    qDebug() << "end run()";
  }

  // ****************************************************************************
  // PUBLIC TRIANGULATION STUFF
  // ****************************************************************************

  // Called from gldraw to initialize thread stuff
  void IsoGen::init(Grid *grid, const PainterDevice *pd, bool interpolate,
                    double stepSize)
  {
    qDebug() << "start init()";
    if (!m_mutex.tryLock())
      return;

    m_grid = grid;
    m_interpolate = interpolate;
    if (stepSize)
      m_stepSize = stepSize;
    else
    {
      // Work out the step size from the global quality level
      switch(pd->painter()->quality())
      {
      case 0:
        m_stepSize = 1.0;
        break;
      case 1:
        m_stepSize = 0.5;
        break;
      case 2:
        m_stepSize = 0.3;
        break;
      case 3:
        m_stepSize = 0.22;
        break;
      case 4:
        m_stepSize = 0.15;
        break;
      default:
        m_stepSize = 0.10;
      }
    }
    m_min = Vector3f(m_grid->grid()->GetOriginVector().x(),
                     m_grid->grid()->GetOriginVector().y(),
                     m_grid->grid()->GetOriginVector().z());
    // Work out the max coordinate too
    int nx, ny, nz;
    m_grid->grid()->GetNumberOfPoints(nx, ny, nz);
    double x[3], y[3], z[3];
    m_grid->grid()->GetAxes(x, y, z);
    m_max = Vector3f(m_min.x() + nx * x[0],
                     m_min.y() + ny * y[1],
                     m_min.z() + nz * z[2]);

    // Right now we are just using one tessellation method
    m_tessellation=&IsoGen::vMarchCube1;

    m_mutex.unlock();
    qDebug() << "end init()";
  }

  // ****************************************************************************
  // LOCAL TRIANGULATION STUFF
  // ****************************************************************************

  // vGetNormal() finds the gradient of the scalar field at a point
  // This gradient can be used as a very accurate vertex normal for lighting calculations
  void IsoGen::vGetNormal(Eigen::Vector3f &rfNormal, const float fX, const float fY, const float fZ)
  {
    rfNormal = Vector3f(m_grid->eval(fX-0.01f, fY, fZ) - m_grid->eval(fX+0.01f, fY, fZ),
        m_grid->eval(fX, fY-0.01f, fZ) - m_grid->eval(fX, fY+0.01f, fZ),
        m_grid->eval(fX, fY, fZ-0.01f) - m_grid->eval(fX, fY, fZ+0.01f));

    rfNormal.normalize();
  }

  // vMarchCube1 performs the Marching Cubes algorithm on a single cube
  void IsoGen::vMarchCube1(const float fX, const float fY, const float fZ)
  {
    long iTriangle, iEdge, iEdgeFlags, iFlagIndex=0;
    Vector3f asEdgeVertex[12] __attribute__((aligned(16)));
    Vector3f asEdgeNorm[12] __attribute__((aligned(16)));
    float fOffset __attribute__((aligned(16)));
    float afCubeValue[8] __attribute__((aligned(16)));

    // Check we have a valid grid
    if (m_grid->grid() == 0)
    {
      qDebug() << "No valid grid :-(";
      return;
    }

    // Make a local copy of the values at the cube's corners
    afCubeValue[0] = m_grid->eval(fX+a2fVertexOffset[0][0]*m_stepSize,
                                  fY+a2fVertexOffset[0][1]*m_stepSize,
                                  fZ+a2fVertexOffset[0][2]*m_stepSize);
    afCubeValue[1] = m_grid->eval(fX+a2fVertexOffset[1][0]*m_stepSize,
                                  fY+a2fVertexOffset[1][1]*m_stepSize,
                                  fZ+a2fVertexOffset[1][2]*m_stepSize);
    afCubeValue[2] = m_grid->eval(fX+a2fVertexOffset[2][0]*m_stepSize,
                                  fY+a2fVertexOffset[2][1]*m_stepSize,
                                  fZ+a2fVertexOffset[2][2]*m_stepSize);
    afCubeValue[3] = m_grid->eval(fX+a2fVertexOffset[3][0]*m_stepSize,
                                  fY+a2fVertexOffset[3][1]*m_stepSize,
                                  fZ+a2fVertexOffset[3][2]*m_stepSize);
    afCubeValue[4] = m_grid->eval(fX+a2fVertexOffset[4][0]*m_stepSize,
                                  fY+a2fVertexOffset[4][1]*m_stepSize,
                                  fZ+a2fVertexOffset[4][2]*m_stepSize);
    afCubeValue[5] = m_grid->eval(fX+a2fVertexOffset[5][0]*m_stepSize,
                                  fY+a2fVertexOffset[5][1]*m_stepSize,
                                  fZ+a2fVertexOffset[5][2]*m_stepSize);
    afCubeValue[6] = m_grid->eval(fX+a2fVertexOffset[6][0]*m_stepSize,
                                  fY+a2fVertexOffset[6][1]*m_stepSize,
                                  fZ+a2fVertexOffset[6][2]*m_stepSize);
    afCubeValue[7] = m_grid->eval(fX+a2fVertexOffset[7][0]*m_stepSize,
                                  fY+a2fVertexOffset[7][1]*m_stepSize,
                                  fZ+a2fVertexOffset[7][2]*m_stepSize);

    // Find which vertices are inside of the surface and which are outside
    if(afCubeValue[0] <= fTargetValue) iFlagIndex |= 1;
    if(afCubeValue[1] <= fTargetValue) iFlagIndex |= 2;
    if(afCubeValue[2] <= fTargetValue) iFlagIndex |= 4;
    if(afCubeValue[3] <= fTargetValue) iFlagIndex |= 8;
    if(afCubeValue[4] <= fTargetValue) iFlagIndex |= 16;
    if(afCubeValue[5] <= fTargetValue) iFlagIndex |= 32;
    if(afCubeValue[6] <= fTargetValue) iFlagIndex |= 64;
    if(afCubeValue[7] <= fTargetValue) iFlagIndex |= 128;

    // Find which edges are intersected by the surface
    // If the cube is entirely inside or outside of the surface, then there will be no intersections
    if(!(iEdgeFlags=aiCubeEdgeFlags[iFlagIndex]))
      return;

    // Find the point of intersection of the surface with each edge
    // Then find the normal to the surface at those points
    for(iEdge=0; iEdge<12; iEdge++)
    {
      //If there is an intersection on this edge
      if(iEdgeFlags & (1<<iEdge))
      {
        // fGetOffset is the approximate point of intersection of the surface between two points
        if( (fOffset = afCubeValue[a2iEdgeConnection[iEdge][1]] - afCubeValue[a2iEdgeConnection[iEdge][0]]) != 0.0f)
          fOffset = (fTargetValue - afCubeValue[a2iEdgeConnection[iEdge][0]]) / fOffset;
        else
          fOffset = 0.5f;

        asEdgeVertex[iEdge] = Vector3f(
          fX + (a2fVertexOffset[a2iEdgeConnection[iEdge][0]][0]
             + fOffset * a2fEdgeDirection[iEdge][0]) * m_stepSize
             - 0.5*m_stepSize,
          fY + (a2fVertexOffset[a2iEdgeConnection[iEdge][0]][1]
             + fOffset * a2fEdgeDirection[iEdge][1]) * m_stepSize
             - 0.5*m_stepSize,
          fZ + (a2fVertexOffset[a2iEdgeConnection[iEdge][0]][2]
             + fOffset * a2fEdgeDirection[iEdge][2]) * m_stepSize
             - 0.5*m_stepSize);
        vGetNormal(asEdgeNorm[iEdge], asEdgeVertex[iEdge].x(), asEdgeVertex[iEdge].y(), asEdgeVertex[iEdge].z());
      }
    }

    // Vertex/normal temporaries
    triangle normTmp;
    triangle vertTmp;

    // Draw the triangles that were found. There can be up to five per cube
    // "Abuse" free iEdgeFlags
    for(iTriangle=0; iTriangle<5; iTriangle++)
    {
      // "Abuse" free iEdge as placeholder for 3*iTriangle
      if(a2iTriangleConnectionTable[iFlagIndex][iEdge=3*iTriangle] < 0)
        break;

      iEdgeFlags = a2iTriangleConnectionTable[iFlagIndex][iEdge++];
      normTmp.p0 = asEdgeNorm[iEdgeFlags];
      vertTmp.p0 = asEdgeVertex[iEdgeFlags];

      iEdgeFlags = a2iTriangleConnectionTable[iFlagIndex][iEdge++];
      normTmp.p1 = asEdgeNorm[iEdgeFlags];
      vertTmp.p1 = asEdgeVertex[iEdgeFlags];

      iEdgeFlags = a2iTriangleConnectionTable[iFlagIndex][iEdge];
      normTmp.p2 = asEdgeNorm[iEdgeFlags];
      vertTmp.p2 = asEdgeVertex[iEdgeFlags];

      m_normList.append(normTmp);
      m_vertList.append(vertTmp);
    }
  } // vMarchCube1()

  // vMarchCube1 performs the Marching Cubes algorithm on a single cube
  void IsoGen::vMarchCube1(int i, int j, int k)
  {
    long iTriangle, iEdge, iEdgeFlags, iFlagIndex=0;
    Vector3f asEdgeVertex[12] __attribute__((aligned(16)));
    Vector3f asEdgeNorm[12] __attribute__((aligned(16)));
    float fOffset __attribute__((aligned(16)));
    float afCubeValue[8] __attribute__((aligned(16)));

    // Work out the cube edge location
    float fX = i * m_stepSize + m_min.x();
    float fY = j * m_stepSize + m_min.y();
    float fZ = k * m_stepSize + m_min.z();

    // Check we have a valid grid
    if (m_grid->grid() == 0)
    {
      qDebug() << "No valid grid :-(";
      return;
    }

    // Make a local copy of the values at the cube's corners
    afCubeValue[0] = m_grid->eval(i+a2iVertexOffset[0][0],
                                  j+a2iVertexOffset[0][1],
                                  k+a2iVertexOffset[0][2]);
    afCubeValue[1] = m_grid->eval(i+a2iVertexOffset[1][0],
                                  j+a2iVertexOffset[1][1],
                                  k+a2iVertexOffset[1][2]);
    afCubeValue[2] = m_grid->eval(i+a2iVertexOffset[2][0],
                                  j+a2iVertexOffset[2][1],
                                  k+a2iVertexOffset[2][2]);
    afCubeValue[3] = m_grid->eval(i+a2iVertexOffset[3][0],
                                  j+a2iVertexOffset[3][1],
                                  k+a2iVertexOffset[3][2]);
    afCubeValue[4] = m_grid->eval(i+a2iVertexOffset[4][0],
                                  j+a2iVertexOffset[4][1],
                                  k+a2iVertexOffset[4][2]);
    afCubeValue[5] = m_grid->eval(i+a2iVertexOffset[5][0],
                                  j+a2iVertexOffset[5][1],
                                  k+a2iVertexOffset[5][2]);
    afCubeValue[6] = m_grid->eval(i+a2iVertexOffset[6][0],
                                  j+a2iVertexOffset[6][1],
                                  k+a2iVertexOffset[6][2]);
    afCubeValue[7] = m_grid->eval(i+a2iVertexOffset[7][0],
                                  j+a2iVertexOffset[7][1],
                                  k+a2iVertexOffset[7][2]);

    // Find which vertices are inside of the surface and which are outside
    if(afCubeValue[0] <= fTargetValue) iFlagIndex |= 1;
    if(afCubeValue[1] <= fTargetValue) iFlagIndex |= 2;
    if(afCubeValue[2] <= fTargetValue) iFlagIndex |= 4;
    if(afCubeValue[3] <= fTargetValue) iFlagIndex |= 8;
    if(afCubeValue[4] <= fTargetValue) iFlagIndex |= 16;
    if(afCubeValue[5] <= fTargetValue) iFlagIndex |= 32;
    if(afCubeValue[6] <= fTargetValue) iFlagIndex |= 64;
    if(afCubeValue[7] <= fTargetValue) iFlagIndex |= 128;

    // Find which edges are intersected by the surface
    // If the cube is entirely inside or outside of the surface, then there will be no intersections
    if(!(iEdgeFlags=aiCubeEdgeFlags[iFlagIndex]))
      return;

    // Find the point of intersection of the surface with each edge
    // Then find the normal to the surface at those points
    for(iEdge=0; iEdge<12; iEdge++)
    {
      //If there is an intersection on this edge
      if(iEdgeFlags & (1<<iEdge))
      {
        // fGetOffset is the approximate point of intersection of the surface between two points
        if( (fOffset = afCubeValue[a2iEdgeConnection[iEdge][1]] - afCubeValue[a2iEdgeConnection[iEdge][0]]) != 0.0f)
          fOffset = (fTargetValue - afCubeValue[a2iEdgeConnection[iEdge][0]]) / fOffset;
        else
          fOffset = 0.5f;

        // The  - 0.5 * m_stepSize correction is only needed if for vMarchCube1(float,float,float)
        asEdgeVertex[iEdge] = Vector3f(
          fX + (a2fVertexOffset[a2iEdgeConnection[iEdge][0]][0]
             + fOffset * a2fEdgeDirection[iEdge][0]) * m_stepSize
             /*- 0.5*m_stepSize*/,
          fY + (a2fVertexOffset[a2iEdgeConnection[iEdge][0]][1]
             + fOffset * a2fEdgeDirection[iEdge][1]) * m_stepSize
             /*- 0.5*m_stepSize*/,
          fZ + (a2fVertexOffset[a2iEdgeConnection[iEdge][0]][2]
             + fOffset * a2fEdgeDirection[iEdge][2]) * m_stepSize
             /*- 0.5*m_stepSize*/);
        vGetNormal(asEdgeNorm[iEdge], asEdgeVertex[iEdge].x(), asEdgeVertex[iEdge].y(), asEdgeVertex[iEdge].z());
      }
    }

    // Vertex/normal temporaries
    triangle normTmp;
    triangle vertTmp;

    // Draw the triangles that were found. There can be up to five per cube
    // "Abuse" free iEdgeFlags
    for(iTriangle=0; iTriangle<5; iTriangle++)
    {
      // "Abuse" free iEdge as placeholder for 3*iTriangle
      if(a2iTriangleConnectionTable[iFlagIndex][iEdge=3*iTriangle] < 0)
        break;

      iEdgeFlags = a2iTriangleConnectionTable[iFlagIndex][iEdge++];
      normTmp.p0 = asEdgeNorm[iEdgeFlags];
      vertTmp.p0 = asEdgeVertex[iEdgeFlags];

      iEdgeFlags = a2iTriangleConnectionTable[iFlagIndex][iEdge++];
      normTmp.p1 = asEdgeNorm[iEdgeFlags];
      vertTmp.p1 = asEdgeVertex[iEdgeFlags];

      iEdgeFlags = a2iTriangleConnectionTable[iFlagIndex][iEdge];
      normTmp.p2 = asEdgeNorm[iEdgeFlags];
      vertTmp.p2 = asEdgeVertex[iEdgeFlags];

      m_normList.append(normTmp);
      m_vertList.append(vertTmp);
    }
  } // vMarchCube1()

  int IsoGen::numTriangles()
  {
    return m_vertListCopy.size();
  }

  triangle IsoGen::getTriangle(int i)
  {
    return m_vertListCopy[i];
  }

  triangle IsoGen::getNormal(int i)
  {
    return m_normListCopy[i];
  }

  // vMarchCube2 performs the Marching Tetrahedrons algorithm on a single cube by
/*  // making six calls to vMarchTetrahedron
void IsoGen::vMarchCube2(const float fX, const float fY, const float fZ)
  {
    long iVertex, iTetrahedron, iVertexInACube;
    Vector3f asCubePosition[8] __attribute__((aligned(16)));
    Vector3f asTetrahedronPosition[4] __attribute__((aligned(16)));
    float afCubeValue[8] __attribute__((aligned(16)));
    float afTetrahedronValue[4] __attribute__((aligned(16)));

    // Make a local copy of the cube's corner positions
    for(iVertex=0; iVertex<8; iVertex++)
    {
      asCubePosition[iVertex].fX = fX + a2fVertexOffset[iVertex][0] * fStepSize;
      asCubePosition[iVertex].fY = fY + a2fVertexOffset[iVertex][1] * fStepSize;
      asCubePosition[iVertex].fZ = fZ + a2fVertexOffset[iVertex][2] * fStepSize;
    }

    // Make a local copy of the cube's corner values
    for(iVertex=0; iVertex<8; iVertex++)
      afCubeValue[iVertex] = isoPar(asCubePosition[iVertex].fX,
                             asCubePosition[iVertex].fY,
                             asCubePosition[iVertex].fZ);

    for(iTetrahedron=0; iTetrahedron<6; iTetrahedron++)
    {
      for(iVertex=0; iVertex<4; iVertex++)
      {
        iVertexInACube = a2iTetrahedronsInACube[iTetrahedron][iVertex];
        asTetrahedronPosition[iVertex].fX = asCubePosition[iVertexInACube].fX;
        asTetrahedronPosition[iVertex].fY = asCubePosition[iVertexInACube].fY;
        asTetrahedronPosition[iVertex].fZ = asCubePosition[iVertexInACube].fZ;
        afTetrahedronValue[iVertex] = afCubeValue[iVertexInACube];
      }
      vMarchTetrahedron(asTetrahedronPosition, afTetrahedronValue);
    }
  } // vMarchCube2()

  // vMarchTetrahedron performs the Marching Tetrahedrons algorithm on a single tetrahedron
  void IsoGen::vMarchTetrahedron(GLvector *pasTetrahedronPosition, const float *pafTetrahedronValue)
  {
    long iEdge, iVert0, iVert1, iEdgeFlags, iFlagIndex=0;
    float fOffset __attribute__((aligned(16)));
    float fInvOffset __attribute__((aligned(16)));
    Vector3f asEdgeVertex[6] __attribute__((aligned(16)));
    Vector3f asEdgeNorm[6] __attribute__((aligned(16)));

    // Find which vertices are inside of the surface and which are outside
    if(pafTetrahedronValue[0] <= fTargetValue) iFlagIndex |= 1;
    if(pafTetrahedronValue[1] <= fTargetValue) iFlagIndex |= 2;
    if(pafTetrahedronValue[2] <= fTargetValue) iFlagIndex |= 4;
    if(pafTetrahedronValue[3] <= fTargetValue) iFlagIndex |= 8;

    // Find which edges are intersected by the surface
    // If the tetrahedron is entirely inside or outside of the surface, then there will be no intersections
    if(!(iEdgeFlags=aiTetrahedronEdgeFlags[iFlagIndex]))
      return;

    // Find the point of intersection of the surface with each edge
    // Then find the normal to the surface at those points
    for(iEdge=0; iEdge<6; iEdge++)
    {
      // If there is an intersection on this edge
      if(iEdgeFlags & (1<<iEdge))
      {
        iVert0 = a2iTetrahedronEdgeConnection[iEdge][0];
        iVert1 = a2iTetrahedronEdgeConnection[iEdge][1];

        // fOffset is the approximate point of intersection of the surface between two points
        if( (fOffset = pafTetrahedronValue[iVert1] - pafTetrahedronValue[iVert0]) != 0.0f)
          fOffset = (fTargetValue - pafTetrahedronValue[iVert0]) / fOffset;
        else
          fOffset = 0.5f;
        fInvOffset = 1.0f - fOffset;

        asEdgeVertex[iEdge].fX = fInvOffset * pasTetrahedronPosition[iVert0].fX + fOffset * pasTetrahedronPosition[iVert1].fX;
        asEdgeVertex[iEdge].fY = fInvOffset * pasTetrahedronPosition[iVert0].fY + fOffset * pasTetrahedronPosition[iVert1].fY;
        asEdgeVertex[iEdge].fZ = fInvOffset * pasTetrahedronPosition[iVert0].fZ + fOffset * pasTetrahedronPosition[iVert1].fZ;
        vGetNormal(asEdgeNorm[iEdge], asEdgeVertex[iEdge].fX, asEdgeVertex[iEdge].fY, asEdgeVertex[iEdge].fZ);
      }

      // Vertex/normal temporaries
      triangle normTmp;
      triangle vertTmp;

      // Draw the triangles that were found. There can be up to 2 per tetrahedron
      // "Abuse" free iVert0, iEdgeFlags
      for(iVert0=0; iVert0<2; iVert0++)
      {
        // "Abuse" free iEdge as placeholder for 3*iTriangle
        if(a2iTetrahedronTriangles[iFlagIndex][iEdge=3*iVert0] < 0)
          break;

        iEdgeFlags = a2iTetrahedronTriangles[iFlagIndex][iEdge++];
        normTmp.p0 = asEdgeNorm[iEdgeFlags];
        vertTmp.p0 = asEdgeVertex[iEdgeFlags];

        iEdgeFlags = a2iTetrahedronTriangles[iFlagIndex][iEdge++];
        normTmp.p1 = asEdgeNorm[iEdgeFlags];
        vertTmp.p1 = asEdgeVertex[iEdgeFlags];

        iEdgeFlags = a2iTetrahedronTriangles[iFlagIndex][iEdge];
        normTmp.p2 = asEdgeNorm[iEdgeFlags];
        vertTmp.p2 = asEdgeVertex[iEdgeFlags];

        normList.append(normTmp);
        vertList.append(vertTmp);
      }
    }
  } // vMarchTetrahedron()
*/
} // End namespace Avogadro
