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

#include <avogadro/primitive.h>
#include <avogadro/color.h>

#include <openbabel/math/vector3.h>
#include <openbabel/griddata.h>
#include <openbabel/grid.h>

#include <QGLWidget>
#include <QDebug>

using namespace std;
using namespace OpenBabel;
using namespace Eigen;

namespace Avogadro {

  SurfaceEngine::SurfaceEngine(QObject *parent) : Engine(parent)
  {
    setDescription(tr("Surface rendering"));
    m_grid = new Grid;
    m_isoGen = new IsoGen;
  }

  SurfaceEngine::~SurfaceEngine()
  {
    delete m_grid;
    delete m_isoGen;
  }

  Engine *SurfaceEngine::clone() const
  {
    SurfaceEngine *engine = new SurfaceEngine(parent());
    engine->setName(name());
    engine->setEnabled(isEnabled());

    return engine;
  }
  // We define a VDW surface here.
  // The isosurface finder declares values < 0 to be outside the surface
  // So values of 0.0 here equal the VDW surface of the molecule
  // + values = the distance inside the surface (i.e., closer to the atomic cente)
  // - values = the distance outside the surface (i.e., farther away)
  void SurfaceEngine::VDWSurface(Molecule *mol)
  {
//    if (m_grid->grid() != NULL) // we already calculated this
//      return;

    OBFloatGrid grid;
    // initialize a grid with spacing 0.333 angstroms between points, plus a padding of 2.5A.
    double spacing = 0.33333;
    double padding = 2.5;
    grid.Init(*mol, spacing, padding);
    double min[3], max[3];
    int xDim, yDim, zDim;

    grid.GetMin(min);
    m_min = Vector3f(min[0], min[1], min[2]);
    grid.GetMax(max);

    xDim = grid.GetXdim();
    yDim = grid.GetYdim();
    zDim = grid.GetZdim();

    vector3 coord;
    double distance, minDistance;
    double maxVal, minVal;
    maxVal = 0.0;
    minVal = 0.0;

    std::vector<double> values;
    values.resize(xDim * yDim * zDim);
    for (int k = 0; k < zDim; ++k) {
      coord.SetZ(min[2] + k * spacing);
      for (int j = 0; j < yDim; ++j) {
        coord.SetY(min[1] + j * spacing);
        for (int i = 0; i < xDim; ++i)
        {
          coord.SetX(min[0] + i * spacing);
          minDistance = 1.0E+10;
          FOR_ATOMS_OF_MOL(a, mol) {
            distance = sqrt(coord.distSq(a->GetVector()));
            distance -= etab.GetVdwRad(a->GetAtomicNum());

            if (distance < minDistance)
              minDistance = distance;
          } // end checking atoms
          // negative = away from molecule, 0 = vdw surface, positive = inside
          values.push_back(-1.0 * minDistance);
          if (-1.0 * minDistance > maxVal)
            maxVal = -1.0 * minDistance;
          if (-1.0 * minDistance < minVal)
            minVal = -1.0 * minDistance;

        } // x-axis
      } // y-axis
    } // z-axis

    qDebug() << " min: " << minVal << " max " << maxVal;

    OBGridData *vdwGrid = new OBGridData;
    double xAxis[3], yAxis[3], zAxis[3];
    xAxis[0] = spacing; xAxis[1] = 0.0;     xAxis[2] = 0.0;
    yAxis[0] = 0.0;     yAxis[1] = spacing; yAxis[2] = 0.0;
    zAxis[0] = 0.0;     zAxis[1] = 0.0;     zAxis[2] = spacing;

    vdwGrid->SetNumberOfPoints( xDim, yDim, zDim);
    vdwGrid->SetLimits(min, xAxis, yAxis, zAxis );
    vdwGrid->SetValues(values);

    m_grid->setGrid(vdwGrid);
  }

  bool SurfaceEngine::renderOpaque(PainterDevice *pd)
  {
    Molecule *mol = const_cast<Molecule *>(pd->molecule());

    VDWSurface(mol);

    qDebug() << " set surface ";

    m_grid->setIsoValue(0.001);
    m_isoGen->init(m_grid, 0.33333, m_min);
    m_isoGen->start();

    qDebug() << " rendering surface ";

    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glDisable(GL_LIGHTING);
    glDisable(GL_BLEND);
    glShadeModel(GL_SMOOTH);

//    glPushName(Primitive::SurfaceType);
//    glPushName(1);

//    glColor3f(1.0, 0.0, 0.0);
    qDebug() << "Number of triangles = " << m_isoGen->numTriangles();

    glBegin(GL_POINTS);
    for(int i=0; i < m_isoGen->numTriangles(); ++i)
    {
      qDebug() << "Drawing points...";
      triangle t = m_isoGen->getTriangle(i);
//       glBegin(GL_TRIANGLES);
//       NORMAL n0 = d->_isoFinder->get_normal(t.v0);
//       glNormal3f(n0.x, n0.y, n0.z);
      glVertex3f(t.p0.x(), t.p0.y(), t.p0.z());

//       NORMAL n1 = d->_isoFinder->get_normal(t.v1);
//       glNormal3f(n1.x, n1.y, n1.z);
      glVertex3f(t.p1.x(), t.p1.y(), t.p1.z());

//       NORMAL n2 = d->_isoFinder->get_normal(t.v2);
//       glNormal3f(n2.x, n2.y, n2.z);
      glVertex3f(t.p2.x(), t.p2.y(), t.p2.z());
    }
    glEnd();

    glPopAttrib();

    return true;
  }
}

#include "surfaceengine.moc"

Q_EXPORT_PLUGIN2(surfaceengine, Avogadro::SurfaceEngineFactory)
