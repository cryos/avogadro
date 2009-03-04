/**********************************************************************
  VibrationDialog - Visualize and animate vibrational modes

  Copyright (C) 2009 by Geoffrey Hutchison

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

#include "vibrationdialog.h"

#include <QPushButton>
#include <QButtonGroup>
#include <QDebug>
#include <QFileDialog>
#include <QFile>
#include <QDir>

#include <QHeaderView>

#include <avogadro/molecule.h>
#include <openbabel/mol.h>
#include <openbabel/generic.h>

using namespace OpenBabel;
using namespace std;

namespace Avogadro {

  VibrationDialog::VibrationDialog( QWidget *parent, Qt::WindowFlags f ) : 
    QDialog( parent, f )
  {
    ui.setupUi(this);
    
    // Make sure the columns span the whole width of the table widget
    QHeaderView *horizontal = ui.vibrationTable->horizontalHeader();
    horizontal->setResizeMode(QHeaderView::Stretch);

    connect(ui.vibrationTable, SIGNAL(currentCellChanged(int, int, int, int)),
            this, SLOT(currentCellChanged(int, int, int, int)));
    connect(ui.vibrationTable, SIGNAL(cellClicked(int, int)),
            this, SLOT(cellClicked(int, int)));

    connect(ui.scaleSlider, SIGNAL(valueChanged(int)),
            this, SLOT(setScale(int)));
    connect(ui.displayForcesCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(setDisplayForceVectors(bool)));
    connect(ui.animationButton, SIGNAL(clicked(bool)),
            this, SLOT(animateButtonClicked(bool)));
    connect(ui.exportButton, SIGNAL(clicked(bool)),
	    this, SLOT(exportVibrationData(bool)));
  }

  VibrationDialog::~VibrationDialog()
  {
    // The real work is done in VibrationExtension
  }

  void VibrationDialog::setMolecule(Molecule *molecule)
  {
    m_molecule = molecule;

    // update table
    ui.vibrationTable->clearContents();

    OBMol obmol = molecule->OBMol();
    m_vibrations = static_cast<OBVibrationData*>(obmol.GetData(OBGenericDataType::VibrationData));
    if (!m_vibrations) {
      ui.vibrationTable->setRowCount(0);
      ui.exportButton->setEnabled(false);
      return;
    }

    // OK, we have valid vibrations, so add them to the table
    vector<double> frequencies = m_vibrations->GetFrequencies();
    vector<double> intensities = m_vibrations->GetIntensities();

    ui.vibrationTable->setRowCount(frequencies.size());
    QString format("%L1");

    for (unsigned int row = 0; row <= frequencies.size(); ++row) {
      QTableWidgetItem *newFreq = new QTableWidgetItem(format.arg(frequencies[row], 0, 'f', 1));
      newFreq->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);
      QTableWidgetItem *newInten = new QTableWidgetItem(format.arg(intensities[row], 0, 'f', 1));
      newInten->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);
      ui.vibrationTable->setItem(row, 0, newFreq);
      ui.vibrationTable->setItem(row, 1, newInten);
    }
    // enable export button
    ui.exportButton->setEnabled(true);
  }

  void VibrationDialog::accept()
  {
    emit selectedMode(-1); // stop animating
    hide();
  }

  void VibrationDialog::currentCellChanged(int, int, int row, int)
  {
    emit selectedMode(row);
  }

  void VibrationDialog::cellClicked(int row, int)
  {
    emit selectedMode(row);
  }

  void VibrationDialog::reject()
  {
    emit selectedMode(-1); // stop animating
    hide();
  }

  void VibrationDialog::setScale(int scale)
  {
    emit scaleUpdated(scale / 2.0);
  }

  void VibrationDialog::setDisplayForceVectors(bool checked)
  {
    emit setEnabledForceVector(checked);
  }

  void VibrationDialog::animateButtonClicked(bool)
  {
    if (ui.animationButton->text() == tr("Start Animation")) {
      ui.animationButton->setText(tr("Stop Animation"));
    } else {
      ui.animationButton->setText(tr("Start Animation"));
    }

    emit toggleAnimation();
  }
  
  void VibrationDialog::exportVibrationData(bool)
  {
    QFileInfo defaultFile(m_molecule->fileName());
    QString defaultPath = defaultFile.canonicalPath();
    if (defaultPath.isEmpty())
      defaultPath = QDir::homePath();

    QString defaultFileName = defaultPath + '/' + defaultFile.baseName() + ".tsv";
    QString filename 	= QFileDialog::getSaveFileName(this, tr("Export Vibrational Data"), defaultFileName, tr("Tab Separated Values (*.tsv)"));
    
    QFile file (filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
      qDebug() << "Cannot open file " << filename << " for writing!";
      return;
    }

    OBMol obmol = m_molecule->OBMol();
    m_vibrations = static_cast<OBVibrationData*>(obmol.GetData(OBGenericDataType::VibrationData));
    if (!m_vibrations) {
      qDebug("No vibration data, but export button is enabled? Something is broken.");
      return;
    }

    vector<double> frequencies = m_vibrations->GetFrequencies();

    vector<double> intensities = m_vibrations->GetIntensities();

    QTextStream out(&file);
    QString format = "%1\t%2\n";

    out << "Frequencies\tIntensities\n";

    for (unsigned int line = 0; line < frequencies.size(); ++line) {
      out << format.arg(frequencies[line], 0, 'f', 2).arg(intensities[line], 0, 'f', 2);
    }
    
    file.close();

    return;
  }
}

#include "vibrationdialog.moc"
