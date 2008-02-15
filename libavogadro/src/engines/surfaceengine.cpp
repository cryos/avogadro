/**********************************************************************
  SurfaceEngine - Engine for display of isosurfaces

  Copyright (C) 2007 Geoffrey R. Hutchison
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

  SurfaceEngine::SurfaceEngine(QObject *parent) : Engine(parent), m_settingsWidget(0),
  m_alpha(0.5), m_stepSize(0.33333), m_padding(2.5), m_renderMode(0), m_colorMode(0)
  {
    setDescription(tr("Surface rendering"));
    //m_grid = new Grid;
    m_isoGen = new IsoGen;
    connect(m_isoGen, SIGNAL(finished()), this, SLOT(isoGenFinished()));
    m_color = Color(1.0, 0.0, 0.0, m_alpha);
    m_surfaceValid = false;
  }

  SurfaceEngine::~SurfaceEngine()
  {
    delete m_isoGen;
    
    // Delete the settings widget if it exists
    if(m_settingsWidget)
      m_settingsWidget->deleteLater();
  }

  Engine *SurfaceEngine::clone() const
  {
    SurfaceEngine *engine = new SurfaceEngine(parent());
    engine->setName(name());
    engine->setEnabled(isEnabled());

    return engine;
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
  Color SurfaceEngine::espColor(Molecule *mol, Vector3f &pos)
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
  
  bool SurfaceEngine::renderOpaque(PainterDevice *pd)
  {
    Molecule *mol = const_cast<Molecule *>(pd->molecule());

    if (!m_surfaceValid) {
      qDebug() << " set surface ";

      //m_grid->setIsoValue(0.001);
      PrimitiveList prims = primitives();
      m_isoGen->init(mol, prims, m_stepSize, m_padding, IsoGen::VDWsurfaceType);
      m_isoGen->start();
      m_surfaceValid = true;
    }

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
  
  double SurfaceEngine::transparencyDepth() const
  {
    return 1.0;
  }

  Engine::EngineFlags SurfaceEngine::flags() const
  {
    return Engine::Transparent | Engine::Atoms;
  }
  
  void SurfaceEngine::setOpacity(int value)
  {
    m_alpha = 0.05 * value;
    m_color.set(m_color.red(), m_color.green(), m_color.blue(), m_alpha);
    emit changed();
  }
  
  void SurfaceEngine::setRenderMode(int value)
  {
    m_renderMode = value;
    emit changed();
  }
  
  void SurfaceEngine::setStepSize(double d)
  {
    m_stepSize = d;
    m_surfaceValid = false;
    emit changed();
  }
  
  void SurfaceEngine::setPadding(double d)
  {
    m_padding = d;
    m_surfaceValid = false;
    emit changed();
  }
  
  void SurfaceEngine::setColorMode(int value)
  {
    if (value == 1) { // ESP
      m_settingsWidget->customColorButton->setEnabled(false);
    } else { // Custom color
      m_settingsWidget->customColorButton->setEnabled(true);
    }

    m_colorMode = value;
    emit changed();
  }
  
  void SurfaceEngine::setColor(QColor color)
  {
    m_color.set(color.redF(), color.greenF(), color.blueF(), m_alpha);
    emit changed();
  }
  
  QWidget* SurfaceEngine::settingsWidget()
  {
    if(!m_settingsWidget)
    {
      m_settingsWidget = new SurfaceSettingsWidget();
      connect(m_settingsWidget->opacitySlider, SIGNAL(valueChanged(int)), this, SLOT(setOpacity(int)));
      connect(m_settingsWidget->renderCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(setRenderMode(int)));
      connect(m_settingsWidget->stepSizeSpin, SIGNAL(valueChanged(double)), this, SLOT(setStepSize(double)));
      connect(m_settingsWidget->paddingSpin, SIGNAL(valueChanged(double)), this, SLOT(setPadding(double)));
      connect(m_settingsWidget->colorCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(setColorMode(int)));
      connect(m_settingsWidget->customColorButton, SIGNAL(colorChanged(QColor)), this, SLOT(setColor(QColor)));
      connect(m_settingsWidget, SIGNAL(destroyed()), this, SLOT(settingsWidgetDestroyed()));

      QColor initial;
      initial.setRgbF(m_color.red(), m_color.green(), m_color.blue());
      m_settingsWidget->customColorButton->setColor(initial);
    }
    return m_settingsWidget;
  }
  
  void SurfaceEngine::isoGenFinished()
  {
    emit changed();
  }

  void SurfaceEngine::setPrimitives(const PrimitiveList &primitives)
  {
    Engine::setPrimitives(primitives);
    m_surfaceValid = false;
    m_isoGen->quit();
  }

  void SurfaceEngine::addPrimitive(Primitive *primitive)
  {
    Engine::addPrimitive(primitive);
    if (primitive->type() == Primitive::AtomType) {
      m_surfaceValid = false;
      m_isoGen->quit();
    }
  }

  void SurfaceEngine::updatePrimitive(Primitive *primitive)
  {
    if ((primitive->type() == Primitive::AtomType) || (primitive->type() == Primitive::MoleculeType)) {
      m_surfaceValid = false;
      m_isoGen->quit();
    }
  }

  void SurfaceEngine::removePrimitive(Primitive *primitive)
  {
    if (primitive->type() == Primitive::AtomType) {
      Engine::removePrimitive(primitive);
      m_surfaceValid = false;
      m_isoGen->quit();
    }
  }


  void SurfaceEngine::settingsWidgetDestroyed()
  {
    qDebug() << "Destroyed Settings Widget";
    m_settingsWidget = 0;
  }

  /*
  void SurfaceEngine::writeSettings(QSettings &settings) const
  {
    Engine::writeSettings(settings);
    settings.setValue("alpha", m_alpha);
    settings.setValue("stepSize", m_stepSize);
    settings.setValue("padding", m_padding);
    //settings.setValue("renderMode", m_renderMode);
    //settings.setValue("colorMode", m_colorMode);
  }

  void SurfaceEngine::readSettings(QSettings &settings)
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

#include "surfaceengine.moc"

Q_EXPORT_PLUGIN2(surfaceengine, Avogadro::SurfaceEngineFactory)
