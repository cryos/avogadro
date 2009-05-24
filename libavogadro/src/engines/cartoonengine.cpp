/**********************************************************************
  CartoonEngine - Engine for protein structures.

  Copyright (C) 2009 Tim Vandermeersch
  Some portions Copyright (C) 2007-2008 by Marcus D. Hanwell

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.openmolecules.net/>

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

#include "cartoonengine.h"

#include <avogadro/molecule.h>
#include <avogadro/atom.h>
#include <avogadro/bond.h>
#include <avogadro/residue.h>
#include <avogadro/color.h>
#include <avogadro/glwidget.h>
#include <avogadro/mesh.h>
#include <avogadro/painterdevice.h>
#include <avogadro/cartoonmeshgenerator.h>

#include <avogadro/protein.h>

#include <QMessageBox>
#include <QString>
#include <QDebug>

#include <Eigen/Geometry>

using Eigen::Vector3d;

namespace Avogadro {

  CartoonEngine::CartoonEngine(QObject *parent) : Engine(parent),
      m_mesh(0), m_settingsWidget(0)
  {
    // Initialise variables
    m_update = true;

    m_aHelix = 1.0; m_bHelix = 0.3; m_cHelix = 1.0;
    m_aSheet = 1.0; m_bSheet = 0.3; m_cSheet = 1.0;
    m_aLoop = 0.2; m_bLoop = 0.2; m_cLoop = 0.0;
  }

  Engine *CartoonEngine::clone() const
  {
    CartoonEngine *engine = new CartoonEngine(parent());
    engine->setAlias(alias());
    engine->m_aHelix = m_aHelix;
    engine->m_bHelix = m_bHelix;
    engine->m_cHelix = m_cHelix;
    engine->m_aSheet = m_aSheet;
    engine->m_bSheet = m_bSheet;
    engine->m_cSheet = m_cSheet;
    engine->m_aLoop = m_aLoop;
    engine->m_bLoop = m_bLoop;
    engine->m_cLoop = m_cLoop;
    engine->m_helixColor = m_helixColor;
    engine->m_sheetColor = m_sheetColor;
    engine->m_loopColor = m_loopColor;
    engine->setEnabled(isEnabled());
    return engine;
  }

  CartoonEngine::~CartoonEngine()
  {
  }
  
  void CartoonEngine::settingsWidgetDestroyed()
  {
    m_settingsWidget = 0;
  }
  
  QWidget* CartoonEngine::settingsWidget()
  {
    if (!m_settingsWidget) {
      m_settingsWidget = new CartoonSettingsWidget;

      // connect the signals
      connect(m_settingsWidget, SIGNAL(destroyed()), this, SLOT(settingsWidgetDestroyed()));
      connect(m_settingsWidget->aHelixSpin, SIGNAL(valueChanged(double)), this, SLOT(setHelixA(double)));
      connect(m_settingsWidget->bHelixSpin, SIGNAL(valueChanged(double)), this, SLOT(setHelixB(double)));
      connect(m_settingsWidget->cHelixSpin, SIGNAL(valueChanged(double)), this, SLOT(setHelixC(double)));
      
      connect(m_settingsWidget->aSheetSpin, SIGNAL(valueChanged(double)), this, SLOT(setSheetA(double)));
      connect(m_settingsWidget->bSheetSpin, SIGNAL(valueChanged(double)), this, SLOT(setSheetB(double)));
      connect(m_settingsWidget->cSheetSpin, SIGNAL(valueChanged(double)), this, SLOT(setSheetC(double)));

      connect(m_settingsWidget->aLoopSpin, SIGNAL(valueChanged(double)), this, SLOT(setLoopA(double)));
      connect(m_settingsWidget->bLoopSpin, SIGNAL(valueChanged(double)), this, SLOT(setLoopB(double)));
      connect(m_settingsWidget->cLoopSpin, SIGNAL(valueChanged(double)), this, SLOT(setLoopC(double)));
      
      connect(m_settingsWidget->helixColorButton, SIGNAL(colorChanged(QColor)), this, SLOT(setHelixColor(QColor)));
      connect(m_settingsWidget->sheetColorButton, SIGNAL(colorChanged(QColor)), this, SLOT(setSheetColor(QColor)));
      connect(m_settingsWidget->loopColorButton, SIGNAL(colorChanged(QColor)), this, SLOT(setLoopColor(QColor)));

      // set the parameters
      m_settingsWidget->aHelixSpin->setValue(m_aHelix);
      m_settingsWidget->bHelixSpin->setValue(m_bHelix);
      m_settingsWidget->cHelixSpin->setValue(m_cHelix);
      m_settingsWidget->aSheetSpin->setValue(m_aSheet);
      m_settingsWidget->bSheetSpin->setValue(m_bSheet);
      m_settingsWidget->cSheetSpin->setValue(m_cSheet);
      m_settingsWidget->aLoopSpin->setValue(m_aLoop);
      m_settingsWidget->bLoopSpin->setValue(m_bLoop);
      m_settingsWidget->cLoopSpin->setValue(m_cLoop);

      // set the colors
      QColor color;
      color.setRgb(m_helixColor.red(), m_helixColor.green(), m_helixColor.blue());
      m_settingsWidget->helixColorButton->setColor(color);
      color.setRgb(m_sheetColor.red(), m_sheetColor.green(), m_sheetColor.blue());
      m_settingsWidget->sheetColorButton->setColor(color);
      color.setRgb(m_loopColor.red(), m_loopColor.green(), m_loopColor.blue());
      m_settingsWidget->loopColorButton->setColor(color);
    }

    return m_settingsWidget;    
  }

  bool CartoonEngine::renderOpaque(PainterDevice *pd)
  {
    // Check if the mesh need updating before drawing it
    if (m_update) updateMesh(pd);

    if (m_mesh) {
      if (m_mesh->stable()) {
        pd->painter()->setColor(0.0, 0.0, 0.0, 1.0);
        pd->painter()->drawColorMesh(*m_mesh);
      }
    }

    // draw selected residues as wireframe
    glDisable(GL_LIGHTING);
    pd->colorMap()->setToSelectionColor();
    pd->painter()->setColor(pd->colorMap());
    foreach(Bond *b, bonds()) {
      if (!pd->isSelected(b))
        continue;
      const Atom* atom1 = b->beginAtom();
      const Vector3d & v1 = *atom1->pos();
      const Atom* atom2 = b->endAtom();
      const Vector3d & v2 = *atom2->pos();
      pd->painter()->drawLine(v1, v2, 2.0);
    }
    glEnable(GL_LIGHTING);


    return true;
  }

  bool CartoonEngine::renderQuick(PainterDevice *pd)
  {
    if (m_mesh) {
      if (m_mesh->stable()) {
        pd->painter()->drawMesh(*m_mesh);
      }
    }

    return true;
  }

  double CartoonEngine::radius(const PainterDevice *, const Primitive *) const
  {
    return 1.0;
  }

  void CartoonEngine::setPrimitives(const PrimitiveList &primitives)
  {
    Engine::setPrimitives(primitives);
    m_update = true;
  }

  void CartoonEngine::addPrimitive(Primitive *primitive)
  {
    Engine::addPrimitive(primitive);
    m_update = true;
  }

  void CartoonEngine::updatePrimitive(Primitive *)
  {
    m_update = true;
  }

  void CartoonEngine::removePrimitive(Primitive *primitive)
  {
    Engine::removePrimitive(primitive);
    m_update = true;
  }

  void CartoonEngine::updateMesh(PainterDevice *pd)
  {
    if (!isEnabled()) return;
    // Get a list of residues for the molecule
    const Molecule *molecule = pd->molecule();

    if (molecule->numResidues() == 0)
      return; // There's no use generating meshes for non-biomolecules

    Color *map = colorMap(); // possible custom color map
    if (!map) map = pd->colorMap(); // fall back to global color map
 
    if (!m_mesh) {
      Molecule *mol = (Molecule*) molecule;
      m_mesh = mol->addMesh();
    }
      
    CartoonMeshGenerator *generator = new CartoonMeshGenerator(molecule, m_mesh);

    generator->setHelixABC(m_aHelix, m_bHelix, m_cHelix);
    generator->setHelixColor(m_helixColor);
    generator->setSheetABC(m_aSheet, m_bSheet, m_cSheet);
    generator->setSheetColor(m_sheetColor);
    generator->setLoopABC(m_aLoop, m_bLoop, m_cLoop);
    generator->setLoopColor(m_loopColor);

    connect(generator, SIGNAL(finished()), this, SIGNAL(changed()));
    connect(generator, SIGNAL(finished()), generator, SLOT(deleteLater()));
    generator->start();

    m_update = false;
  }

  Engine::PrimitiveTypes CartoonEngine::primitiveTypes() const
  {
    return Engine::Molecules;
  }

  Engine::ColorTypes CartoonEngine::colorTypes() const
  {
    return Engine::NoColors;
  }

  void CartoonEngine::writeSettings(QSettings &settings) const
  {
    Engine::writeSettings(settings);
    settings.setValue("aHelix", m_aHelix);
    settings.setValue("bHelix", m_bHelix);
    settings.setValue("cHelix", m_cHelix);
    settings.setValue("aSheet", m_aSheet);
    settings.setValue("bSheet", m_bSheet);
    settings.setValue("cSheet", m_cSheet);
    settings.setValue("aLoop", m_aLoop);
    settings.setValue("bLoop", m_bLoop);
    settings.setValue("cLoop", m_cLoop);
    
    settings.setValue("cHelixColor", m_helixColor);
    settings.setValue("cSheetColor", m_sheetColor);
    settings.setValue("cLoopColor", m_loopColor);

    if (m_mesh)
      settings.setValue("meshId", static_cast<int>(m_mesh->id()));
  }

  void CartoonEngine::readSettings(QSettings &settings)
  {
    Engine::readSettings(settings);
    
    m_aHelix = settings.value("aHelix", 1.0).toDouble();
    m_bHelix = settings.value("bHelix", 0.3).toDouble();
    m_cHelix = settings.value("cHelix", 1.0).toDouble();
    m_aSheet = settings.value("aSheet", 1.0).toDouble();
    m_bSheet = settings.value("bSheet", 0.3).toDouble();
    m_cSheet = settings.value("cSheet", 1.0).toDouble();
    m_aLoop = settings.value("aLoop", 0.2).toDouble();
    m_bLoop = settings.value("bLoop", 0.2).toDouble();
    m_cLoop = settings.value("cLoop", 0.0).toDouble();
     
    m_helixColor = settings.value("cHelixColor", QColor(255,0,0)).value<QColor>();
    m_sheetColor = settings.value("cSheetColor", QColor(255,255,0)).value<QColor>();
    m_loopColor = settings.value("cLoopColor", QColor(0,255,0)).value<QColor>();

    if (m_molecule) {
      m_mesh = m_molecule->meshById(settings.value("meshId", 0).toInt());
    }
  }

  void CartoonEngine::setHelixA(double value) 
  { 
    m_aHelix = value; 
    m_update = true; 
    emit changed();
  }
  void CartoonEngine::setHelixB(double value) 
  { 
    m_bHelix = value; 
    m_update = true;
    emit changed();
  }
  void CartoonEngine::setHelixC(double value) 
  { 
    m_cHelix = value; 
    m_update = true; 
    emit changed();
  }
  void CartoonEngine::setSheetA(double value) 
  { m_aSheet = value; m_update = true; }
  void CartoonEngine::setSheetB(double value) 
  { 
    m_bSheet = value; 
    m_update = true; 
    emit changed();
  }
  void CartoonEngine::setSheetC(double value) 
  { 
    m_cSheet = value; 
    m_update = true; 
    emit changed();
  }
  void CartoonEngine::setLoopA(double value) 
  { 
    m_aLoop = value; 
    m_update = true; 
    emit changed();
  }
  void CartoonEngine::setLoopB(double value) 
  { 
    m_bLoop = value; 
    m_update = true; 
    emit changed();
  }
  void CartoonEngine::setLoopC(double value) 
  { 
    m_cLoop = value; 
    m_update = true; 
    emit changed();
  }
  void CartoonEngine::setHelixColor(QColor color)
  {
    m_helixColor = color;
    m_update = true;
    emit changed();
  }
  void CartoonEngine::setSheetColor(QColor color)
  {
    m_sheetColor = color;
    m_update = true;
    emit changed();
  }
  void CartoonEngine::setLoopColor(QColor color)
  {
    m_loopColor = color;
    m_update = true;
    emit changed();
  } 
 
}

Q_EXPORT_PLUGIN2(cartoonengine, Avogadro::CartoonEngineFactory)
