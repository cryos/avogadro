/**********************************************************************
  VibrationWidget - Visualize and animate vibrational modes

  Copyright (C) 2009 by Geoffrey Hutchison
  Some portions Copyright (C) 2010 by Konstantin Tokarev

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.openmolecules.net/>

  Some code is based on Open Babel
  For more information, see <http://openbabel.sourceforge.net/>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation version 2 of the License.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
 ***********************************************************************/

#include "vibrationwidget.h"

#include <QtCore/QFile>
#include <QtCore/QDir>
#include <QtCore/QDebug>

#include <QtGui/QButtonGroup>
#include <QtGui/QDoubleValidator>
#include <QtGui/QFileDialog>
#include <QtGui/QHeaderView>
#include <QtGui/QProgressDialog>
#include <QtGui/QPushButton>

#include <avogadro/molecule.h>

#include <openbabel/mol.h>
#include <openbabel/generic.h>

using namespace OpenBabel;
using namespace std;

namespace Avogadro {

  VibrationWidget::VibrationWidget( QWidget *parent, Qt::WindowFlags f ) : 
    QWidget( parent, f ), m_filter(0), m_currentRow(0), m_widget(0),
    m_molecule(0), m_vibrations(0), m_indexMap(0)
  {    
    ui.setupUi(this);

    // Make sure the columns span the whole width of the table widget
    QHeaderView *horizontal = ui.vibrationTable->horizontalHeader();
    horizontal->setResizeMode(QHeaderView::Stretch);
    ui.editFilter->setValidator(new QDoubleValidator(0, 1e6, 10, ui.editFilter));

    m_indexMap = new std::vector<int>;

    connect(ui.editFilter, SIGNAL(textChanged(QString)),
            this, SLOT(changeFilter(QString)));

    connect(ui.vibrationTable, SIGNAL(currentCellChanged(int, int, int, int)),
            this, SLOT(currentCellChanged(int, int, int, int)));
    connect(ui.vibrationTable, SIGNAL(cellClicked(int, int)),
            this, SLOT(cellClicked(int, int)));

    connect(ui.scaleSlider, SIGNAL(valueChanged(int)),
            this, SLOT(setScale(int)));
    connect(ui.displayForcesCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(setDisplayForceVectors(bool)));
    connect(ui.normalizeDispCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(setNormalize(bool)));
    connect(ui.animationSpeedCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(setAnimationSpeed(bool)));
    connect(ui.animationButton, SIGNAL(clicked(bool)),
            this, SLOT(animateButtonClicked(bool)));
    connect(ui.pauseButton, SIGNAL(clicked(bool)),
            this, SLOT(pauseButtonClicked(bool)));

    connect(ui.spectraButton, SIGNAL(clicked()),
            this, SLOT(spectraButtonClicked()));    
  }

  VibrationWidget::~VibrationWidget()
  {
    // The real work is done in VibrationExtension
    emit selectedMode(-1); // stop animating
    hide();
  }

  void VibrationWidget::setMolecule(Molecule *molecule)
  {
    // update table
    ui.vibrationTable->clearContents();
      if (molecule == 0){
        ui.vibrationTable->setRowCount(0);
        ui.vibrationTable->horizontalHeader()->hide();
        return;
      }
    m_molecule = molecule;

    OBMol obmol = molecule->OBMol();
    m_vibrations = static_cast<OBVibrationData*>(obmol.GetData(OBGenericDataType::VibrationData));
    if (!m_vibrations) {
      ui.vibrationTable->setRowCount(0);
      ui.vibrationTable->horizontalHeader()->hide();
      //ui.exportButton->setEnabled(false);
      return;
    }

    ui.vibrationTable->horizontalHeader()->show();
    ui.vibrationTable->horizontalHeader()->setResizeMode(QHeaderView::Stretch);

    // OK, we have valid vibrations, so add them to the table
    vector<double> frequencies = m_vibrations->GetFrequencies();
    vector<double> intensities = m_vibrations->GetIntensities();
    m_frequencies = frequencies;
    m_intensities = intensities;
    #ifdef OPENBABEL_IS_NEWER_THAN_2_2_99
      vector<double> raman_activities = m_vibrations->GetRamanActivities();
      if (raman_activities.size() == 0) {
        //resize(274, height());
        ui.vibrationTable->setColumnCount(2);
        if(parentWidget())
          parentWidget()->setMinimumWidth(274);
      } else {
        //resize(310, height());
        ui.vibrationTable->setColumnCount(3);
        ui.vibrationTable->setHorizontalHeaderItem(2, new QTableWidgetItem("Activity"));
        if(parentWidget())
          parentWidget()->setMinimumWidth(310);
      }
    #else
        //resize(274, height());
        ui.vibrationTable->setColumnCount(2);
        if(parentWidget())
          parentWidget()->setMinimumWidth(274);
    #endif

    // Generate an index vector to map sorted indicies to the old indices
    m_indexMap->clear();
    for (uint i = 0; i < frequencies.size(); i++)
      m_indexMap->push_back(i);

    // Setup progress dialog, just in case it takes longer than 2 seconds
    QProgressDialog prog(tr("Sorting %1 vibrations by frequency...")
                         .arg(frequencies.size()), "", 0, frequencies.size());
    prog.setWindowModality(Qt::WindowModal);
    prog.setMinimumDuration(2000);
    prog.setCancelButton(0);

    // Simple selection sort
    double tmp;
    int tmp_int;
    for (uint i = 0; i < frequencies.size(); i++) {
      for (uint j = i; j < frequencies.size(); j++) {
        if (i == j) continue; // Save a bit of time...
        if (frequencies.at(j) < frequencies.at(i)) {
          tmp = frequencies.at(j);
          frequencies.at(j) = frequencies.at(i);
          frequencies.at(i) = tmp;
          tmp = intensities.at(j);
          intensities.at(j) = intensities.at(i);
          intensities.at(i) = tmp;
          tmp_int = m_indexMap->at(j);
          m_indexMap->at(j) = m_indexMap->at(i);
          m_indexMap->at(i) = tmp_int;
        }
      }
      // Update progress bar
      prog.setValue(i);
    }

    ui.vibrationTable->setRowCount(frequencies.size());
    QString format("%1");

    for (unsigned int row = 0; row < frequencies.size(); ++row) {
      QTableWidgetItem *newFreq = new QTableWidgetItem(format.arg(frequencies[row], 0, 'f', 2));
      newFreq->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);
      // Some codes don't provide intensity data. Display "-" in place of intensities.
      QTableWidgetItem *newInten;
      if (row >= intensities.size()) {
        newInten = new QTableWidgetItem("-");
      }
      else {
        newInten = new QTableWidgetItem(format.arg(intensities[row], 0, 'f', 3));
      }
      #ifdef OPENBABEL_IS_NEWER_THAN_2_2_99
        QTableWidgetItem *newRaman;
        if (row >= raman_activities.size()) {
          newRaman = new QTableWidgetItem("-");
        }
        else {
          newRaman = new QTableWidgetItem(format.arg(raman_activities[row], 0, 'f', 3));
        }
        newRaman->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);
      #endif
      newInten->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);
      ui.vibrationTable->setItem(row, 0, newFreq);
      ui.vibrationTable->setItem(row, 1, newInten);
      #ifdef OPENBABEL_IS_NEWER_THAN_2_2_99
        ui.vibrationTable->setItem(row, 2, newRaman);
      #endif
    }

    // enable export button
    //ui.exportButton->setEnabled(true);
  }

  void VibrationWidget::changeFilter(QString str)
  {      
    m_filter = str.toDouble();
    for (int i=0; i<m_frequencies.size(); i++) {
      if (i<m_intensities.size()) {
        if (m_intensities.at(i) > m_filter) {
          ui.vibrationTable->showRow(i);          
      }
        else
          ui.vibrationTable->hideRow(i);
      }
    }
  }

  void VibrationWidget::currentCellChanged(int row, int, int, int)
  {
    if (row != -1 && !ui.animationButton->isEnabled()) {
      ui.animationButton->setEnabled(true);
    }
    m_currentRow = row;
    if (row == -1) emit selectedMode(row);
    else emit selectedMode(m_indexMap->at(row));
  }

  void VibrationWidget::cellClicked(int row, int)
  {
    if (row != -1 && !ui.animationButton->isEnabled()) {
      ui.animationButton->setEnabled(true);
    }
    m_currentRow = row;
    if (row == -1) emit selectedMode(row);
    else emit selectedMode(m_indexMap->at(row));
  }

  void VibrationWidget::reject()
  {
    emit selectedMode(-1); // stop animating
    hide();
  }

  void VibrationWidget::setScale(int scale)
  {
    emit scaleUpdated(scale / 2.0);
  }

  void VibrationWidget::setScale(double scale)
  {
    emit scaleUpdated(scale);
  }

  void VibrationWidget::setNormalize(bool checked)
  {
    if (checked != ui.normalizeDispCheckBox->isChecked())
      ui.normalizeDispCheckBox->setChecked(checked);

    emit normalizeUpdated(checked);
  }

  void VibrationWidget::setDisplayForceVectors(bool checked)
  {
    if (checked != ui.displayForcesCheckBox->isChecked())
      ui.displayForcesCheckBox->setChecked(checked);

    emit forceVectorUpdated(checked);
  }

  void VibrationWidget::setAnimationSpeed(bool checked)
  {
    if (checked != ui.animationSpeedCheckBox->isChecked())
      ui.animationSpeedCheckBox->setChecked(checked);

    emit animationSpeedUpdated(checked);
  }

  void VibrationWidget::animateButtonClicked(bool)
  {
    if (ui.animationButton->text() == tr("Start &Animation")) {
      ui.animationButton->setText(tr("Stop &Animation"));
      ui.animationButton->setIcon(QIcon(":/amarok/icons/amarok_stop.png"));
      ui.pauseButton->setText(tr("Pause"));
      ui.pauseButton->setEnabled(true);
      if (m_currentRow == -1)
        emit selectedMode(m_currentRow);
      else
        emit selectedMode(m_indexMap->at(m_currentRow));
    } else {
      ui.animationButton->setText(tr("Start &Animation"));
      ui.animationButton->setIcon(QIcon(":/amarok/icons/amarok_play.png"));
      ui.pauseButton->setText(tr("Pause"));
      ui.pauseButton->setEnabled(false);
    }

    emit toggleAnimation();
  }

  void VibrationWidget::pauseButtonClicked(bool)
  {
    if (ui.pauseButton->text() == tr("Pause")) {
      ui.pauseButton->setText(tr("Continue"));
    } else {
      ui.pauseButton->setText(tr("Pause"));
    }

    emit pauseAnimation();
  }

  void VibrationWidget::spectraButtonClicked()
  {
    emit showSpectra();
  }
  
/*  void VibrationWidget::exportVibrationData(bool)
  {
    QFileInfo defaultFile(m_molecule->fileName());
    QString defaultPath = defaultFile.canonicalPath();
    if (defaultPath.isEmpty())
      defaultPath = QDir::homePath();

    QString defaultFileName = defaultPath + '/' + defaultFile.baseName() + ".tsv";
    QString filename 	= QFileDialog::getSaveFileName(this, tr("Export Vibrational Data"), defaultFileName, tr("Tab Separated Values (*.tsv)"));
    
    QFile file (filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
      qWarning() << "Cannot open file " << filename << " for writing!";
      return;
    }

    OBMol obmol = m_molecule->OBMol();
    m_vibrations = static_cast<OBVibrationData*>(obmol.GetData(OBGenericDataType::VibrationData));
    if (!m_vibrations) {
      qWarning("No vibration data, but export button is enabled? Something is broken.");
      return;
    }

    vector<double> frequencies = m_vibrations->GetFrequencies();
    vector<double> intensities = m_vibrations->GetIntensities();

    QTextStream out(&file);
    out << "Frequencies\tIntensities\n";

    QString format = "%1\t%2\n";
    for (unsigned int line = 0; line < frequencies.size(); ++line) {
      QString intensity;
      // we can't trust that we'll actually get intensities
      // some formats don't report them
      if (line >= intensities.size())
        intensity = '-';
      else
        intensity = QString("%L1").arg(intensities[line], 0, 'f', 2);

      out << format.arg(frequencies[line], 0, 'f', 2).arg(intensity);
    }
    
    file.close();

    return;
  }*/

} // namespace Avogadro

