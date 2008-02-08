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

/****************************************************************************
 **
 ** Most of the code for the Marching Cubes and Tetrahedras and the tables are
 ** from an exemplary implementation of Cory Gene Bloyd. In a first step I
 ** made just minor adoptions like reformatting, interfacing or optimizations
 ** in the code. Later on I made it reentrant and thread-safe with some more
 ** adaptions.
 **
 ** Thanks to Cory Gene Bloyd for publishing this excellent and understandable
 ** piece of software. For a description of the original algorithm look at:
 **
 ** http://astronomy.swin.edu.au/pbourke/modelling/polygonise/
 **
 ****************************************************************************/

/**********************************************************************
 Iso - Iso class for displaying surfaces using OpenGL

 Copyright (C) 2008      Marcus D. Hanwell
 
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

#ifndef ISO_H
#define ISO_H

#include <QList>
#include <QThread>
#include <avogadro/glwidget.h>
#include <openbabel/griddata.h>
#include <openbabel/grid.h>

namespace Avogadro
{
  /* Add implicit functions and a Grid class in order to use OpenBabel grids */
  class ImplicitFunction
  {
  public:
    virtual float eval(float, float, float) = 0;
    virtual ~ImplicitFunction() { }
  };
  
  // Attach to a grid and set
  class Grid : public ImplicitFunction
  {
  public:
    double m_iso;
    OpenBabel::OBGridData *m_gd;
    
    Grid(): m_iso(0.), m_gd(0) { ; }
    ~Grid()
    {
      if (m_gd)
      {
        delete m_gd;
        m_gd = 0;
      }
    }
    
    void setIsoValue(float i) { m_iso = i; }
    float isoValue() { return m_iso; }
    void setGrid(OpenBabel::OBGridData *gd) { m_gd = gd; }
    OpenBabel::OBGridData* grid() { return m_gd; }

    float eval(float x, float y, float z)
    {
      OpenBabel::vector3 v(x, y, z);
      return m_gd->GetValue(v);
    }
  };

  // Triangle structure
  struct triangle
  {
    Eigen::Vector3f p0;
    Eigen::Vector3f p1;
    Eigen::Vector3f p2;
  };

  // Does isosurface tessellation. Called from gldraw only
  class IsoGen : public QThread
  {

  public:

    // General stuff
    IsoGen(QObject *parent = 0) : QThread(parent) { ; }

    ~IsoGen() { ; }

    // Vertex/normal-lists
    QList<triangle> m_normList;
    QList<triangle> m_vertList;

    // Central functions
    void init(Grid *grid, double size, Eigen::Vector3f min);
    void start();
    int numTriangles();
    triangle getTriangle(int i);
    triangle getNormal(int i);

  protected:
    void run();

  private:
    Grid *m_grid; // OpenBabel Grid
    float m_fStepSize; // Grid density == 2.0f/sta.tgrids;
    long m_totTri; // Triangles calculated in total; currently not used
    Eigen::Vector3f m_min;

    // Constants/tables
    static const float fTargetValue;
    static const float a2fVertexOffset[8][3];
    static const long a2iEdgeConnection[12][2];
    static const float a2fEdgeDirection[12][3];
    static const long a2iTetrahedronEdgeConnection[6][2];
    static const long a2iTetrahedronsInACube[6][4];
    static const long aiTetrahedronEdgeFlags[16];
    static const long a2iTetrahedronTriangles[16][7];
    static const long aiCubeEdgeFlags[256];
    static const long a2iTriangleConnectionTable[256][16];

    // Functions
//    void vNormalizeVector(Eigen::Vector3f &rfResult, const Eigen::Vector3f &rfSource);
    void vGetNormal(Eigen::Vector3f &rfNormal, const float fX, const float fY,
        const float fZ);
    void vMarchCube1(const float fX, const float fY, const float fZ);
//    void vMarchCube2(const float fX, const float fY, const float fZ);
//    void vMarchTetrahedron(Eigen::Vector3f *pasTetrahedronPosition,
//        const float *pafTetrahedronValue);
    void (IsoGen::*m_tessellation)(const float fX, const float fY, const float fZ);

  // Multithreading stuff:
      // Using own class-instance of standard iso-parsers and own calc-function
      // Copies of Warp-instances are not reentrant/threadsafe by itself
//      FunctionParser isoInstance;
//      float isoPar(const float xpar, const float ypar, const float zpar);
    };

  }
  // ISO_H
#endif
