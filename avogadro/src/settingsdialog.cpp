/**********************************************************************
  SettingsDialog - Settings Dialog

  Copyright (C) 2007-2008 Donald Ephraim Curtis

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

#include "settingsdialog.h"

#include "mainwindow.h"

#include <QAbstractButton>
#include <QDialogButtonBox>
#include <QDebug>

namespace Avogadro {

  SettingsDialog::SettingsDialog(MainWindow *mainWindow) : QDialog(mainWindow)
  {
    m_mainWindow = mainWindow;
    ui.setupUi(this);

    loadValues();

    connect(ui.dialogButtonBox, SIGNAL(clicked(QAbstractButton *)),
        this, SLOT(buttonClicked(QAbstractButton *)));
    connect(ui.qualitySlider, SIGNAL(valueChanged(int)), this,
        SLOT(qualityChanged(int)));
    connect(ui.fogSlider, SIGNAL(valueChanged(int)), this,
        SLOT(fogChanged(int)));
  }

  void SettingsDialog::buttonClicked(QAbstractButton *button)
  {
    QDialogButtonBox::ButtonRole role = ui.dialogButtonBox->buttonRole(button);
    if(role == QDialogButtonBox::ApplyRole || role == QDialogButtonBox::AcceptRole)
    {
      saveValues();
    }
    else if (role == QDialogButtonBox::RejectRole)
    {
        loadValues();
    }
  }

  void SettingsDialog::saveValues()
  {
    m_mainWindow->setPainterQuality(ui.qualitySlider->value());
    m_mainWindow->setFogLevel(ui.fogSlider->value());
    m_mainWindow->setTabbedTools(ui.tabbedToolsCheck->isChecked());
    m_mainWindow->setAnimationsEnabled(ui.animationsCheck->isChecked());
    m_mainWindow->setToolsTabPosition((QTabWidget::TabPosition)ui.tabPositionCombo->currentIndex());
  }

  void SettingsDialog::loadValues()
  {
    ui.qualitySlider->setValue(m_mainWindow->painterQuality());
    ui.fogSlider->setValue(m_mainWindow->fogLevel());
    qualityChanged(m_mainWindow->painterQuality());
    ui.tabbedToolsCheck->setChecked(m_mainWindow->tabbedTools());
    ui.animationsCheck->setChecked(m_mainWindow->animationsEnabled());
    ui.tabPositionCombo->setCurrentIndex(m_mainWindow->toolsTabPosition());
  }

  void SettingsDialog::qualityChanged(int value)
  {
    switch (value) {
      case 0:
      case 1:
        ui.qualityValueLabel->setText(tr("Low", "Display quality setting"));
        break;
      case 2:
        ui.qualityValueLabel->setText(tr("Medium", "Display quality setting"));
        break;
      case 3:
      case 4:
        ui.qualityValueLabel->setText(tr("High", "Display quality setting"));
        break;
      default:
        ui.qualityValueLabel->setText(tr("Undefined", "Display quality setting"));
    }
  }

  void SettingsDialog::fogChanged(int value)
  {
    switch (value) {
      case 0:
        ui.fogValueLabel->setText(tr("None", "Level of fog in the scene"));
        break;
      case 1:
      case 2:
      case 3:
        ui.fogValueLabel->setText(tr("Some", "Level of fog in the scene"));
        break;
      case 4:
      case 5:
      case 6:
        ui.fogValueLabel->setText(tr("Mid", "Level of fog in the scene"));
        break;
      case 7:
      case 8:
      case 9:
        ui.fogValueLabel->setText(tr("Lots", "Level of fog in the scene"));
        break;
      default:
        ui.fogValueLabel->setText(tr("Undefined", "Level of fog in the scene"));
    }
  }

} // end namespace Avogadro

#include "settingsdialog.moc"
