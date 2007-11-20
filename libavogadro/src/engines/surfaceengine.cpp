/**********************************************************************
  SurfaceEngine - Engine for display of isosurfaces

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

#include <config.h>
#include "surfaceengine.h"
#include "polygonizer.h"

#include <avogadro/primitive.h>
#include <avogadro/color.h>
#include <avogadro/glwidget.h>

#include <openbabel/math/vector3.h>
#include <openbabel/griddata.h>
#include <openbabel/grid.h>

#include <QtPlugin>

using namespace std;
using namespace OpenBabel;
using namespace Eigen;

namespace Avogadro {
  
  // This is a small "shim" between the OpenBabel grid classes and the Bloomenthal Polygonizer code
  // It mainly allows you to attach a grid and set an isovalue
  class Grid: public ImplicitFunction
    {
    public:
      double iso;
      OBGridData *gd;

      Grid(): iso(0.0), gd(NULL) {}
      ~Grid()
      {
        if (gd) {
          delete gd;
          gd = NULL;
        }
      }

      void SetIsovalue(double i) { iso = i; }
      double GetIsovalue() {
        return iso;
      }
      
      void SetGrid(OBGridData *g) { gd = g;}
      OBGridData* GetGrid() {
        return gd;
      }
      
      float eval (float x, float y, float z)
      {
        vector3 v(x, y, z);
        return gd->GetValue(v) - iso;
      }
      
      float GetValue (float x, float y, float z)
      {
        vector3 v(x, y, z);
        return gd->GetValue(v);        
      }
    };

  
  class SurfacePrivateData
    {
    public:
      Grid         _gridFunction; //<! 3D implicit function f(x,y,z) = ...
      Polygonizer* _isoFinder;    //<! Class to find isosurface where f(x,y,z) = isovalue
    };
  
  SurfaceEngine::SurfaceEngine(QObject *parent) : Engine(parent), d(new SurfacePrivateData)
  {
    setName(tr("Surface"));
    setDescription(tr("Surface rendering"));
  }
  
  SurfaceEngine::~SurfaceEngine()
  {
    delete d->_isoFinder;
    delete d;
  }

  // We define a VDW surface here.
  // The isosurface finder declares values < 0 to be outside the surface
  // So values of 0.0 here equal the VDW surface of the molecule
  // + values = the distance inside the surface (i.e., closer to the atomic cente)
  // - values = the distance outside the surface (i.e., farther away)
  void SurfaceEngine::VDWSurface(Molecule *mol)
  {
    if (d->_gridFunction.GetGrid() != NULL) // we already calculated this
      return;

    OBFloatGrid _grid;
    // initialize a grid with spacing 0.333 angstroms between points, plus a padding of 2.5A.
    double spacing = 0.33333;
    double padding = 2.5;
    _grid.Init(*mol, spacing, padding);
    double min[3], max[3];
    int xDim, yDim, zDim;
    
    _grid.GetMin(min);
    _grid.GetMax(max);
    
    xDim = _grid.GetXdim();
    yDim = _grid.GetYdim();
    zDim = _grid.GetZdim();
    
    vector3 coord;
    double distance, minDistance;
    double maxVal, minVal;
    maxVal = 0.0;
    minVal = 0.0;
    
    std::vector<double> _values;
    _values.resize(xDim * yDim * zDim);
    for (int k = 0; k < zDim; ++k) {
      coord.SetZ(min[2] + k * spacing);
      for (int j = 0; j < yDim; ++j) {
        coord.SetY(min[1] + j * spacing);
        for (int i = 0; i < xDim; ++i) {
          coord.SetX(min[0] + i * spacing);
          minDistance = 1.0E+10;
          FOR_ATOMS_OF_MOL(a, mol) {
            distance = sqrt(coord.distSq(a->GetVector()));
            distance -= etab.GetVdwRad(a->GetAtomicNum());
            
            if (distance < minDistance)
              minDistance = distance;
          } // end checking atoms
          // negative = away from molecule, 0 = vdw surface, positive = inside
          _values.push_back(-1.0 * minDistance);
          if (-1.0 * minDistance > maxVal)
            maxVal = -1.0 * minDistance;
          if (-1.0 * minDistance < minVal)
            minVal = -1.0 * minDistance;
          
        } // x-axis
      } // y-axis
    } // z-axis
    
    qDebug() << " min: " << minVal << " max " << maxVal;
    
    OBGridData *_vdwGrid = new OBGridData;
    double xAxis[3], yAxis[3], zAxis[3];
    xAxis[0] = spacing; xAxis[1] = 0.0;     xAxis[2] = 0.0;
    yAxis[0] = 0.0;     yAxis[1] = spacing; yAxis[2] = 0.0;
    zAxis[0] = 0.0;     zAxis[1] = 0.0;     zAxis[2] = spacing;
    
    _vdwGrid->SetNumberOfPoints( xDim, yDim, zDim);
    _vdwGrid->SetLimits( min, xAxis, yAxis, zAxis );
    _vdwGrid->SetValues(_values);
    
    d->_gridFunction.SetGrid(_vdwGrid);
  }
  
  bool SurfaceEngine::renderOpaque(PainterDevice *pd)
  {
    Molecule *mol = const_cast<Molecule *>(pd->molecule());
    
    VDWSurface(mol);
    
    qDebug() << " set surface ";
    
    d->_gridFunction.SetIsovalue(0.001);
    d->_isoFinder = new Polygonizer(&d->_gridFunction, 0.15, 30);
    d->_isoFinder->march(false, 0.,0.,0.); // marching cubes
    
    qDebug() << " rendering surface ";
    
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glDisable(GL_LIGHTING);
    glDisable(GL_BLEND);
    glShadeModel(GL_SMOOTH);
    
//    glPushName(Primitive::SurfaceType);
//    glPushName(1);
    
//    glColor3f(1.0, 0.0, 0.0);
    
    glBegin(GL_POINTS);
    for(int i=0; i < d->_isoFinder->no_triangles(); ++i)
    {
      TRIANGLE t = d->_isoFinder->get_triangle(i);
//       glBegin(GL_TRIANGLES);
//       NORMAL n0 = d->_isoFinder->get_normal(t.v0);
//       glNormal3f(n0.x, n0.y, n0.z);
      VERTEX v0 = d->_isoFinder->get_vertex(t.v0);
      glVertex3f(v0.x, v0.y, v0.z);
      
//       NORMAL n1 = d->_isoFinder->get_normal(t.v1);
//       glNormal3f(n1.x, n1.y, n1.z);
      VERTEX v1 = d->_isoFinder->get_vertex(t.v1);
      glVertex3f(v1.x, v1.y, v1.z);
      
//       NORMAL n2 = d->_isoFinder->get_normal(t.v2);
//       glNormal3f(n2.x, n2.y, n2.z);
      VERTEX v2 = d->_isoFinder->get_vertex(t.v2);
      glVertex3f(v2.x, v2.y, v2.z);
    }
    glEnd();    

    glPopAttrib();

    return true;
  }
}

#include "surfaceengine.moc"

Q_EXPORT_PLUGIN2(surfaceengine, Avogadro::SurfaceEngineFactory)
