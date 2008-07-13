/**********************************************************************
  ForceEngine - Display forces

  Copyright (C) 2008 by Tim Vandermeersch

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

#include "forceengine.h"

#include <config.h>
#include <avogadro/primitive.h>
#include <avogadro/color.h>
#include <avogadro/glwidget.h>

#include <openbabel/obiter.h>

#include <QMessageBox>
#include <QString>
#include <QDebug>

using namespace std;
using namespace OpenBabel;
using namespace Eigen;

namespace Avogadro {

  ForceEngine::ForceEngine(QObject *parent) : Engine(parent)
  {
    setDescription(tr("Renders forces on atoms"));
  }

  Engine *ForceEngine::clone() const
  {
    ForceEngine *engine = new ForceEngine(parent());
    engine->setAlias(alias());
    //engine->setWidth(m_width);
    //engine->setRadius(m_radius);
    //engine->setAngle(m_angle);
    engine->setEnabled(isEnabled());

    return engine;
  }

  ForceEngine::~ForceEngine()
  {
  }

  bool ForceEngine::renderOpaque(PainterDevice *pd)
  {
    Molecule *mol = const_cast<Molecule *>(pd->molecule());

    pd->painter()->setColor(0.0, 1.0, 0.0);

    if (!mol->HasData(OBGenericDataType::ConformerData))
      return false;
    
    OBConformerData *cd = (OBConformerData*) mol->GetData(OBGenericDataType::ConformerData);
    vector<vector<vector3> > allForces = cd->GetForces();
    
    if (!allForces.size())
      return false;
    vector<vector3> forces = allForces[0];

    if (forces.size() < mol->NumAtoms())
      return false;

    FOR_ATOMS_OF_MOL (atom, mol) {
      vector3 pos = atom->GetVector();
      vector3 force = forces[atom->GetIdx()-1];
      
      Vector3d v1 = Vector3d(pos.x(), pos.y(), pos.z()); // start point line
      Vector3d v2 = Vector3d(pos.x()+force.x(), pos.y()+force.y(), pos.z()+force.z()); // end point both
      Vector3d v3 = Vector3d(pos.x()+0.8*force.x(), pos.y()+0.8*force.y(), pos.z()+0.8*force.z()); // start point cone
      pd->painter()->drawLine(v1, v2, 2);
      pd->painter()->drawCone(v3, v2, 0.1);
    }
    
    return true;
  }

  /*
  QWidget* HBondEngine::settingsWidget()
  {
    if(!m_settingsWidget)
      {
        m_settingsWidget = new HBondSettingsWidget();
        connect(m_settingsWidget->widthSlider, SIGNAL(valueChanged(int)), this, SLOT(setWidth(int)));
        connect(m_settingsWidget->radiusSpin, SIGNAL(valueChanged(double)), this, SLOT(setRadius(double)));
        connect(m_settingsWidget->angleSpin, SIGNAL(valueChanged(double)), this, SLOT(setAngle(double)));
        connect(m_settingsWidget, SIGNAL(destroyed()), this, SLOT(settingsWidgetDestroyed()));
        m_settingsWidget->widthSlider->setValue(m_width);
        m_settingsWidget->radiusSpin->setValue(m_radius);
        m_settingsWidget->angleSpin->setValue(m_angle);
      }
    return m_settingsWidget;
  }
  
  void HBondEngine::setWidth(int value)
  {
    m_width = (double) value;
    emit changed();
  }
  
  void HBondEngine::setRadius(double value)
  {
    m_radius = value;
    emit changed();
  }
  
  void HBondEngine::setAngle(double value)
  {
    m_angle = value;
    emit changed();
  }


  void HBondEngine::settingsWidgetDestroyed()
  {
    qDebug() << "Destroyed Settings Widget";
    m_settingsWidget = 0;
  }
  
  void HBondEngine::writeSettings(QSettings &settings) const
  {
    Engine::writeSettings(settings);
    settings.setValue("width", m_width);
    settings.setValue("radius", m_radius);
    settings.setValue("angle", m_angle);
  }

  void HBondEngine::readSettings(QSettings &settings)
  {
    Engine::readSettings(settings);
    setWidth(settings.value("width", 2.0).toDouble());
    setRadius(settings.value("radius", 2.0).toDouble());
    setAngle(settings.value("angle", 120.0).toDouble());
    if (m_settingsWidget) {
      m_settingsWidget->widthSlider->setValue(m_width);
      m_settingsWidget->radiusSpin->setValue(m_radius);
      m_settingsWidget->angleSpin->setValue(m_angle);
    }
  }
  */
}

#include "forceengine.moc"

Q_EXPORT_PLUGIN2(forceengine, Avogadro::ForceEngineFactory)
