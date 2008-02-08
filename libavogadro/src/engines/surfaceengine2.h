/**********************************************************************
  SurfaceEngine - Engine for display of isosurfaces

  Copyright (C) 1991-2007 Heinz van Saanen
  Copyright (C) 2006-2007 Geoffrey R. Hutchison
  Copyright (C) 2008 Tim Vandermeersch
  Copyright (C) 2008 Marcus D. Hanwell

  This file (original) is part of the the function viewer Zhu3D.

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

#ifndef __SURFACEENGINE_H
#define __SURFACEENGINE_H
/*
#include <cmath>
#include <GL/gl.h>
#include "property.h"
#include "pinterface.h"
#include "debug.h"
*/

#include <avogadro/global.h>
#include <avogadro/engine.h>

#include <openbabel/math/vector3.h>

#include <QList>
#include <QThread>

// Vector structure (use OpenBabel::vector3)
//struct GLvector {
//	float fX;
//	float fY;
//	float fZ;
//} __attribute__((aligned(16)));



namespace Avogadro {
  
  //! Triangle structure
  struct Triangle 
  {
    OpenBabel::vector3 p0;
    OpenBabel::vector3 p1;
    OpenBabel::vector3 p2;
  };


  class ImplicitFunction;
  class Grid;
  class SurfacePrivateData;
  //! Surface Engine class.
  class SurfaceEngine : public Engine
  {
    Q_OBJECT
    AVOGADRO_ENGINE(tr("Surface"))

    public:
      //! Constructor
      SurfaceEngine(QObject *parent=0);
      //! Deconstructor
      ~SurfaceEngine();
      
      Engine *clone() const;

      //! \name Render Methods
      //@{
      bool renderOpaque(PainterDevice *pd);
      //@}

    
    protected:
      SurfacePrivateData *d;
      double m_min[3], m_max[3];

      void VDWSurface(Molecule *mol);
  };
  
  //! Does isosurface tessellation. Called from gldraw only
  class IsoGen : public QThread 
  {
    public:
      //! Constructor
      IsoGen(QObject *parent = 0) : QThread(parent) {};
      //! Deconstructor
      ~IsoGen() {};

      // Vertex/normal-lists
      std::vector<Triangle> normList;
      std::vector<Triangle> vertList;

      // Central functions
      void march(Grid* func, double size, OpenBabel::vector3 &min);
      int numTriangles(); //! Numer of triangles
      Triangle getTriangle(int i); //! Get Triangle i
      Triangle getNormal(int i); //! Get normals for triangle i
      
      void forceRun();
    protected:
      void run();

    private:
      OpenBabel::vector3 	m_min;
      double		m_fStepSize;		//! Grid density == 2.0f/sta.tgrids;
      unsigned int 	m_totTri;		//! Triangles calculated in total; currently not used
      
      Grid* 	m_func;	//! The implicit function
      
      // Constants/tables
      static const double	fTargetValue;
      static const int		a2fVertexOffset[8][3];
      static const unsigned int	a2iEdgeConnection[12][2];
      static const double 	a2fEdgeDirection[12][3];
      //static const unsigned int	a2iTetrahedronEdgeConnection[6][2];
      //static const unsigned int	a2iTetrahedronsInACube[6][4];
      //static const char  	aiTetrahedronEdgeFlags[16];
      //static const int  	a2iTetrahedronTriangles[16][7];
      static const unsigned int	aiCubeEdgeFlags[256];
      static const int		a2iTriangleConnectionTable[256][16];
      
      // Functions
      void vNormalizeVector(OpenBabel::vector3 &rfVectorResult, const OpenBabel::vector3 &rfVectorSource);
      void vGetNormal(OpenBabel::vector3 &rfNormal, const double fX, const double fY, const double fZ);
      void vMarchCube1(const double x, const double y, const double z);
      //void vMarchCube2(const double fX, const double fY, const double fZ);
      //void vMarchTetrahedron(vector3 *pasTetrahedronPosition, const double *pafTetrahedronValue);
      void (IsoGen::*tessellation)(const double x, const double y, const double z);
      
      // Multithreading stuff:
      // Using own class-instance of standard iso-parsers and own calc-function
      // Copies of Warp-instances are not reentrant/threadsafe by itself
      //FunctionParser isoInstance;
      //double isoPar(const double xpar, const double ypar, const double zpar);
  };

  //! Generates instances of our SurfaceEngine class
  class SurfaceEngineFactory : public QObject, public EngineFactory
  {
    Q_OBJECT
    Q_INTERFACES(Avogadro::EngineFactory)
    AVOGADRO_ENGINE_FACTORY(SurfaceEngine);

  };

} // end namespace Avogadro

#endif
