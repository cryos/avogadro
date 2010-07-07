/**********************************************************************
   OrbitalSettingsDialog -- Configure the Orbital Extension

  Copyright (C) 2010 David C. Lonie

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

#include "orbitalsettingsdialog.h"

#include <QDebug>

namespace Avogadro {

  OrbitalSettingsDialog::OrbitalSettingsDialog(OrbitalWidget* parent,
                                               Qt::WindowFlags f)
    : QDialog(parent, f),
      m_quality(OrbitalWidget::OQ_Low),
      m_isoval(0.02),
      m_HOMOFirst(false)
  {
    ui.setupUi(this);

    connect(ui.push_recalc, SIGNAL(clicked()),
            this, SLOT(calculateAllClicked()));

    connect(this, SIGNAL(calculateAll()),
            parent, SIGNAL(calculateAll()));
    connect(this, SIGNAL(defaultsUpdated(OrbitalWidget::OrbitalQuality, double, bool)),
            parent, SLOT(setDefaults(OrbitalWidget::OrbitalQuality, double, bool)));

  }

  OrbitalSettingsDialog::~OrbitalSettingsDialog()
  {
  }

  void OrbitalSettingsDialog::setDefaultQuality(OrbitalWidget::OrbitalQuality q)
  {
    ui.combo_quality->setCurrentIndex(q);
    m_quality = q;
  }

  void OrbitalSettingsDialog::setIsoValue(double i)
  {
    ui.spin_isoval->setValue(i);
    m_isoval = i;
  }

  void OrbitalSettingsDialog::setHOMOFirst(bool HOMOFirst)
  {
    ui.cb_HOMOFirst->setChecked(HOMOFirst);
    m_HOMOFirst = HOMOFirst;
  }

  void OrbitalSettingsDialog::updateDefaults()
  {
    m_quality = OrbitalWidget::OrbitalQuality(ui.combo_quality->currentIndex());
    m_isoval = ui.spin_isoval->value();
    m_HOMOFirst = ui.cb_HOMOFirst->isChecked();
    emit defaultsUpdated(m_quality, m_isoval, m_HOMOFirst);
  }

  void OrbitalSettingsDialog::accept()
  {
    updateDefaults();
    hide();
  }

  void OrbitalSettingsDialog::reject()
  {
    setDefaultQuality(m_quality);
    setIsoValue(m_isoval);
    setHOMOFirst(m_HOMOFirst);
    hide();
  }

  void OrbitalSettingsDialog::calculateAllClicked()
  {
    updateDefaults();
    emit calculateAll();
  }

}
