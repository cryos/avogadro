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
        return gd->GetValue(v)*gd->GetValue(v) - iso;
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

  bool SurfaceEngine::renderOpaque(PainterDevice *pd)
  {
    Molecule *mol = const_cast<Molecule *>(pd->molecule());
    
    if (!mol->HasData(OBGenericDataType::GridData))
      return false; // no surface data
    
    d->_gridFunction.SetGrid(static_cast<OBGridData *>(mol->GetData(OBGenericDataType::GridData)));
    d->_gridFunction.SetIsovalue(0.0);
    d->_isoFinder = new Polygonizer(&d->_gridFunction, 0.15, 30);
    d->_isoFinder->march(false, 0.,0.,0.); // marching cubes
    
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glDisable(GL_LIGHTING);
    glDisable(GL_BLEND);
    glShadeModel(GL_SMOOTH);
    
//    glPushName(Primitive::SurfaceType);
//    glPushName(1);
    
//    glColor3f(1.0, 0.0, 0.0);
    
    for(int i=0; i < d->_isoFinder->no_triangles(); ++i)
    {
      TRIANGLE t = d->_isoFinder->get_triangle(i);
      glBegin(GL_TRIANGLES);
      NORMAL n0 = d->_isoFinder->get_normal(t.v0);
      glNormal3f(n0.x, n0.y, n0.z);
      VERTEX v0 = d->_isoFinder->get_vertex(t.v0);
      glVertex3f(v0.x, v0.y, v0.z);
      
      NORMAL n1 = d->_isoFinder->get_normal(t.v1);
      glNormal3f(n1.x, n1.y, n1.z);
      VERTEX v1 = d->_isoFinder->get_vertex(t.v1);
      glVertex3f(v1.x, v1.y, v1.z);
      
      NORMAL n2 = d->_isoFinder->get_normal(t.v2);
      glNormal3f(n2.x, n2.y, n2.z);
      VERTEX v2 = d->_isoFinder->get_vertex(t.v2);
      glVertex3f(v2.x, v2.y, v2.z);
      glEnd();
    }
    
//    glPopName();
//    glPopName();
        
    glPopAttrib();

    return true;
  }
}

#include "surfaceengine.moc"

Q_EXPORT_PLUGIN2(surfaceengine, Avogadro::SurfaceEngineFactory)