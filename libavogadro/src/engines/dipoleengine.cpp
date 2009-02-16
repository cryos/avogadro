/**********************************************************************
  DipoleEngine - Engine to display a 3D vector such as the dipole moment

  Copyright (C) 2008      Marcus D. Hanwell

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

#include "dipoleengine.h"
#include <config.h>

#include <avogadro/color.h>
#include <avogadro/glwidget.h>
#include <avogadro/painterdevice.h>
#include <avogadro/atom.h>
#include <avogadro/molecule.h>

#include <QDebug>

using namespace Eigen;

namespace Avogadro {

  DipoleEngine::DipoleEngine(QObject *parent) : Engine(parent),
    m_dipoleType(0), m_settingsWidget(0)
  {
    setDescription(tr("Renders dipole moments and other 3D data"));

    m_dipole.x() = 0.0;
    m_dipole.y() = 0.0;
    m_dipole.z() = 0.0;
  }

  DipoleEngine::~DipoleEngine()
  {
    // Delete the settings widget if it exists
    if(m_settingsWidget)
      m_settingsWidget->deleteLater();
  }

  Engine* DipoleEngine::clone() const
  {
    DipoleEngine* engine = new DipoleEngine(parent());

    engine->setAlias(alias());
    engine->setEnabled(isEnabled());
    return engine;
  }

  bool DipoleEngine::renderOpaque(PainterDevice *pd)
  {
    Molecule *mol = const_cast<Molecule *>(pd->molecule());
    m_molecule = mol;

    updateDipole();

    Vector3d origin = Vector3d(0.0, 0.0, 0.0); // start at the origin
    Vector3d joint = 0.2 * m_dipole; // 80% along the length

    pd->painter()->setColor(1.0, 0.0, 0.0);
    pd->painter()->drawLine(m_dipole, joint, 3.0);
    pd->painter()->drawCone(joint, origin, 0.4);
    // TODO: add a "cross" line for the <--+ look to the dipole moment)

    return true;
  }

  double DipoleEngine::radius(const PainterDevice *, const Primitive *) const
  {
    return 0.0;
  }

  double DipoleEngine::transparencyDepth() const
  {
    return 1.0;
  }

  Engine::Layers DipoleEngine::layers() const
  {
    return Engine::Overlay;
  }

  Engine::PrimitiveTypes DipoleEngine::primitiveTypes() const
  {
    return Engine::NoPrimitives;
  }

  Engine::ColorTypes DipoleEngine::colorTypes() const
  {
    return Engine::NoColors;
  }

  QWidget *DipoleEngine::settingsWidget()
  {
    if(!m_settingsWidget)
    {
      m_settingsWidget = new DipoleSettingsWidget();
      m_settingsWidget->dipoleType->setCurrentIndex(m_dipoleType);
      connect(m_settingsWidget->dipoleType, SIGNAL(activated(int)),
              this, SLOT(setDipoleType(int)));
      connect(m_settingsWidget, SIGNAL(destroyed()),
              this, SLOT(settingsWidgetDestroyed()));
      connect(m_settingsWidget->xDipoleSpinBox, SIGNAL(valueChanged(double)),
              this, SLOT(updateDipole(double)));
      connect(m_settingsWidget->yDipoleSpinBox, SIGNAL(valueChanged(double)),
              this, SLOT(updateDipole(double)));
      connect(m_settingsWidget->zDipoleSpinBox, SIGNAL(valueChanged(double)),
              this, SLOT(updateDipole(double)));
    }
    return m_settingsWidget;
  }

  void DipoleEngine::setDipoleType(int value)
  {
    m_dipoleType = value;
    updateDipole();

    if (m_dipoleType != 1) { // not the custom version
      m_settingsWidget->customLabel->setEnabled(false);
      m_settingsWidget->xDipoleSpinBox->setEnabled(false);
      m_settingsWidget->yDipoleSpinBox->setEnabled(false);
      m_settingsWidget->zDipoleSpinBox->setEnabled(false);
    }
    else {
      m_settingsWidget->customLabel->setEnabled(true);
      m_settingsWidget->xDipoleSpinBox->setEnabled(true);
      m_settingsWidget->yDipoleSpinBox->setEnabled(true);
      m_settingsWidget->zDipoleSpinBox->setEnabled(true);
    }

    emit changed();
  }

  void DipoleEngine::updateDipole(double)
  {
    switch(m_dipoleType)
    {
      case 1: // custom
        m_dipole = Vector3d(m_settingsWidget->xDipoleSpinBox->value(),
                              m_settingsWidget->yDipoleSpinBox->value(),
                              m_settingsWidget->zDipoleSpinBox->value());
        break;

      default: // embedded OBGenericData type -- handle
        m_dipole = Vector3d(*m_molecule->dipoleMoment());
    }
    emit changed();
  }

  void DipoleEngine::settingsWidgetDestroyed()
  {
    qDebug() << "Destroyed Settings Widget";
    m_settingsWidget = 0;
  }

}

#include "dipoleengine.moc"

Q_EXPORT_PLUGIN2(DipoleEngine, Avogadro::DipoleEngineFactory)
