/**********************************************************************
   OrbitalSettingsDialog -- Configure the Orbital Extension

  Copyright (C) 2010 David C. Lonie

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.cc/>

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
      m_boxPadding(2.5),
      m_HOMOFirst(false),
      m_limit_precalc(true),
      m_precalc_range(10)
  {
    ui.setupUi(this);

    connect(ui.push_recalc, SIGNAL(clicked()),
            this, SLOT(calculateAllClicked()));

    connect(this, SIGNAL(calculateAll()),
            parent, SIGNAL(calculateAll()));
    connect(this, SIGNAL(defaultsUpdated(OrbitalWidget::OrbitalQuality, double, double, bool)),
            parent, SLOT(setDefaults(OrbitalWidget::OrbitalQuality, double, double, bool)));
    connect(this, SIGNAL(precalcSettingsUpdated(bool,int)),
            parent, SLOT(setPrecalcSettings(bool,int)));
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

  void OrbitalSettingsDialog::setBoxPadding(double i)
  {
    ui.spin_boxPadding->setValue(i);
    m_boxPadding = i;
  }

  void OrbitalSettingsDialog::setHOMOFirst(bool HOMOFirst)
  {
    ui.cb_HOMOFirst->setChecked(HOMOFirst);
    m_HOMOFirst = HOMOFirst;
  }

  void OrbitalSettingsDialog::setLimitPrecalc(bool b)
  {
    ui.cb_limit_precalc->setChecked(b);
    m_limit_precalc = b;
  }

  void OrbitalSettingsDialog::setPrecalcRange(int r)
  {
    ui.spin_precalc_range->setValue(r);
    m_precalc_range = r;
  }

  void OrbitalSettingsDialog::updateDefaults()
  {
    m_quality = OrbitalWidget::OrbitalQuality(ui.combo_quality->currentIndex());
    m_isoval = ui.spin_isoval->value();
    m_boxPadding = ui.spin_boxPadding->value();
    m_HOMOFirst = ui.cb_HOMOFirst->isChecked();
    emit defaultsUpdated(m_quality, m_isoval, m_boxPadding, m_HOMOFirst);
  }

  void OrbitalSettingsDialog::updatePrecalcSettings()
  {
    m_limit_precalc = ui.cb_limit_precalc->isChecked();
    m_precalc_range = ui.spin_precalc_range->value();
    emit precalcSettingsUpdated(m_limit_precalc, m_precalc_range);
  }

  void OrbitalSettingsDialog::accept()
  {
    updateDefaults();
    updatePrecalcSettings();
    hide();
  }

  void OrbitalSettingsDialog::reject()
  {
    setDefaultQuality(m_quality);
    setIsoValue(m_isoval);
    setBoxPadding(m_boxPadding);
    setHOMOFirst(m_HOMOFirst);
    hide();
  }

  void OrbitalSettingsDialog::calculateAllClicked()
  {
    updateDefaults();
    emit calculateAll();
  }

}
