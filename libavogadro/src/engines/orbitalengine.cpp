/**********************************************************************
  OrbitalEngine - Engine for display of isosurfaces

  Copyright (C) 2008 Geoffrey R. Hutchison
  Copyright (C) 2008 Marcus D. Hanwell
  Copyright (C) 2008 Tim Vandermeersch

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
#include "orbitalengine.h"

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

  OrbitalEngine::OrbitalEngine(QObject *parent) : Engine(parent), m_settingsWidget(0),
  m_alpha(0.5), m_stepSize(0.33333), m_padding(2.5), m_renderMode(0), m_colorMode(0)
  {
    setDescription(tr("Orbital rendering"));
    m_grid = new Grid;
    m_isoGen = new IsoGen;
    connect(m_isoGen, SIGNAL(finished()), this, SLOT(isoGenFinished()));
    m_color = Color(1.0, 0.0, 0.0, m_alpha);
  }

  OrbitalEngine::~OrbitalEngine()
  {
    delete m_grid;
    delete m_isoGen;
    
    // Delete the settings widget if it exists
    if(m_settingsWidget)
      m_settingsWidget->deleteLater();
  }

  Engine *OrbitalEngine::clone() const
  {
    OrbitalEngine *engine = new OrbitalEngine(parent());
    engine->setName(name());
    engine->setEnabled(isEnabled());

    return engine;
  }
  // We define a VDW surface here.
  // The isosurface finder declares values < 0 to be outside the surface
  // So values of 0.0 here equal the VDW surface of the molecule
  // + values = the distance inside the surface (i.e., closer to the atomic cente)
  // - values = the distance outside the surface (i.e., farther away)
  void OrbitalEngine::VDWSurface(Molecule *mol)
  {
//    if (m_grid->grid() != NULL) // we already calculated this
//      return;

    OBFloatGrid grid;
    // initialize a grid with spacing 0.333 angstroms between points, plus a padding of 2.5A.
    grid.Init(*mol, m_stepSize, m_padding);
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
    //values.resize(xDim * yDim * zDim);
    for (int k = 0; k < zDim; ++k) {
      coord.SetZ(min[2] + k * m_stepSize);
      for (int j = 0; j < yDim; ++j) {
        coord.SetY(min[1] + j * m_stepSize);
        for (int i = 0; i < xDim; ++i)
        {
          coord.SetX(min[0] + i * m_stepSize);
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
    xAxis[0] = m_stepSize; xAxis[1] = 0.0;        xAxis[2] = 0.0;
    yAxis[0] = 0.0;        yAxis[1] = m_stepSize; yAxis[2] = 0.0;
    zAxis[0] = 0.0;        zAxis[1] = 0.0;        zAxis[2] = m_stepSize;

    vdwGrid->SetNumberOfPoints( xDim, yDim, zDim);
    vdwGrid->SetLimits(min, xAxis, yAxis, zAxis );
    vdwGrid->SetValues(values);

    m_grid->setGrid(vdwGrid);
  }

  //                                          // 
  //     |    red    green     blue           //
  // 1.0 |...--+       +       +--...         //
  //     |      \     / \     /               //
  //     |       \   /   \   /                //
  //     |        \ /     \ /                 //
  //     |         X       X                  //
  //     |        / \     / \                 //
  //     |       /   \   /   \                //
  //     |      /     \ /     \               //
  // 0.0 +...--+-------+-------+--...-->      //
  //           a      0.0      b      energy
  //
  //  a = 20 * energy
  //  b = 20 * energy
  //
  Color OrbitalEngine::espColor(Molecule *mol, Vector3f &pos)
  {
    GLfloat red, green, blue;
    double energy = 0.0;
    vector3 p, dist;
    
    p.SetX(pos.x());
    p.SetY(pos.y());
    p.SetZ(pos.z());
    
    FOR_ATOMS_OF_MOL (atom, mol) {
      dist = atom->GetVector() - p;
      energy += atom->GetPartialCharge() / (dist.length()*dist.length());
    }

    if (energy < 0.0) {
      blue = -20.0*energy;
      if (blue >= 1.0) {
        return Color(0.0, 0.0, 1.0, m_alpha);
      }

      green = 1.0 - blue;
      return Color(0.0, green, blue, m_alpha);
    }

    if (energy > 0.0) {
      red = 20.0*energy;
      if (red >= 1.0) {
        return Color(1.0, 0.0, 0.0, m_alpha);
      }
      green = 1.0 - red;
      return Color(red, green, 0.0, m_alpha);
    }

    return Color(0.0, 1.0, 0.0, m_alpha);
  }
  
  bool OrbitalEngine::renderOpaque(PainterDevice *pd)
  {
    Molecule *mol = const_cast<Molecule *>(pd->molecule());

    if (!mol->HasData(OBGenericDataType::GridData)) {
      // ultimately allow the user to attach a new data file
      return false;
    } else {
      m_grid->SetGrid(static_cast<OBGridData *>(mol->GetData(OBGenericDataType::GridData)));
    }

    qDebug() << " set surface ";

    // For orbitals, we'll need to set this iso value and make sure it's
    // for +/- 0.001 for example
    // We may need some logic to check if a cube is an orbital or not...
    // (e.g., someone might bring in spin density = always positive)
    m_grid->setIsoValue(0.001);
    m_isoGen->init(m_grid, m_stepSize, m_min);
    m_isoGen->start();

    qDebug() << " rendering surface ";

    glPushAttrib(GL_ALL_ATTRIB_BITS);
//    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glShadeModel(GL_SMOOTH);

    pd->painter()->setColor(1.0, 0.0, 0.0, m_alpha);
//    glPushName(Primitive::SurfaceType);
//    glPushName(1);

    qDebug() << "Number of triangles = " << m_isoGen->numTriangles();

    switch (m_renderMode) {
    case 0:
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
      break;
    case 1:
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
      break;
    case 2:
      glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
      break;
    }
    
    glBegin(GL_TRIANGLES);
    if (m_colorMode == 1) { // ESP
      Color color;
      for(int i=0; i < m_isoGen->numTriangles(); ++i)
      {
        triangle t = m_isoGen->getTriangle(i);
        triangle n = m_isoGen->getNormal(i);
      
        color = espColor(mol, t.p0);
        color.applyAsMaterials();
        glNormal3fv(n.p0.array());
        glVertex3fv(t.p0.array());

        color = espColor(mol, t.p1);
        color.applyAsMaterials();
        glNormal3fv(n.p1.array());
        glVertex3fv(t.p1.array());
      
        color = espColor(mol, t.p2);
        color.applyAsMaterials();
        glNormal3fv(n.p2.array());
        glVertex3fv(t.p2.array());
      }
    } else { // RGB
      //glColor4f(m_color.red(), m_color.green(), m_color.blue(), m_alpha);
      m_color.applyAsMaterials();
      for(int i=0; i < m_isoGen->numTriangles(); ++i)
      {
        triangle t = m_isoGen->getTriangle(i);
        triangle n = m_isoGen->getNormal(i);
      
        glNormal3fv(n.p0.array());
        glVertex3fv(t.p0.array());

        glNormal3fv(n.p1.array());
        glVertex3fv(t.p1.array());
      
        glNormal3fv(n.p2.array());
        glVertex3fv(t.p2.array());
      }
    }
    glEnd();

    glPopAttrib();

    return true;
  }
  
  double OrbitalEngine::transparencyDepth() const
  {
    return 1.0;
  }

  Engine::EngineFlags OrbitalEngine::flags() const
  {
    return Engine::Transparent | Engine::Atoms;
  }
  
  void OrbitalEngine::setOpacity(int value)
  {
    m_alpha = 0.05 * value;
    emit changed();
  }
  
  void OrbitalEngine::setRenderMode(int value)
  {
    m_renderMode = value;
    emit changed();
  }
  
  void OrbitalEngine::setStepSize(double d)
  {
    m_stepSize = d;
    emit changed();
  }
  
  void OrbitalEngine::setPadding(double d)
  {
    m_padding = d;
    emit changed();
  }
  
  void OrbitalEngine::setColorMode(int value)
  {
    if (value == 1) {
      m_settingsWidget->RSpin->setMaximum(0.0);
      m_settingsWidget->GSpin->setMaximum(0.0);
      m_settingsWidget->BSpin->setMaximum(0.0);
    } else {
      m_settingsWidget->RSpin->setMaximum(1.0);
      m_settingsWidget->GSpin->setMaximum(1.0);
      m_settingsWidget->BSpin->setMaximum(1.0);
      m_settingsWidget->RSpin->setValue(1.0);
    }

    m_colorMode = value;
    emit changed();
  }
  
  void OrbitalEngine::setRed(double r)
  {
    m_color.set(r, m_color.green(), m_color.blue(), m_alpha);
    emit changed();
  }
  
  void OrbitalEngine::setGreen(double g)
  {
    m_color.set(m_color.red(), g, m_color.blue(), m_alpha);
    emit changed();
  }

  void OrbitalEngine::setBlue(double b)
  {
    m_color.set(m_color.red(), m_color.green(), b, m_alpha);
    emit changed();
  }

  QWidget* OrbitalEngine::settingsWidget()
  {
    if(!m_settingsWidget)
    {
      m_settingsWidget = new OrbitalSettingsWidget();
      connect(m_settingsWidget->opacitySlider, SIGNAL(valueChanged(int)), this, SLOT(setOpacity(int)));
      connect(m_settingsWidget->renderCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(setRenderMode(int)));
      connect(m_settingsWidget->stepSizeSpin, SIGNAL(valueChanged(double)), this, SLOT(setStepSize(double)));
      connect(m_settingsWidget->paddingSpin, SIGNAL(valueChanged(double)), this, SLOT(setPadding(double)));
      connect(m_settingsWidget->colorCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(setColorMode(int)));
      connect(m_settingsWidget->RSpin, SIGNAL(valueChanged(double)), this, SLOT(setRed(double)));
      connect(m_settingsWidget->GSpin, SIGNAL(valueChanged(double)), this, SLOT(setGreen(double)));
      connect(m_settingsWidget->BSpin, SIGNAL(valueChanged(double)), this, SLOT(setBlue(double)));
      connect(m_settingsWidget, SIGNAL(destroyed()), this, SLOT(settingsWidgetDestroyed()));
    }
    return m_settingsWidget;
  }
  
  void OrbitalEngine::isoGenFinished()
  {
    emit changed();
  }

  void OrbitalEngine::settingsWidgetDestroyed()
  {
    qDebug() << "Destroyed Settings Widget";
    m_settingsWidget = 0;
  }

  /*
  void OrbitalEngine::writeSettings(QSettings &settings) const
  {
    Engine::writeSettings(settings);
    settings.setValue("alpha", m_alpha);
    settings.setValue("stepSize", m_stepSize);
    settings.setValue("padding", m_padding);
    //settings.setValue("renderMode", m_renderMode);
    //settings.setValue("colorMode", m_colorMode);
  }

  void OrbitalEngine::readSettings(QSettings &settings)
  {
    Engine::readSettings(settings);
    //m_alpha = settings.value("alpha", 0.5).toDouble();
    m_stepSize = settings.value("stepSize", 0.33333).toDouble();
    m_padding = settings.value("padding", 2.5).toDouble();
    m_renderMode = 0;
    m_colorMode = 0;
  }
  */
}

#include "orbitalengine.moc"

Q_EXPORT_PLUGIN2(orbitalengine, Avogadro::OrbitalEngineFactory)
