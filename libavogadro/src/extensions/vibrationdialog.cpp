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

    connect(ui.vibrationTable, SIGNAL(cellClicked(int, int)),
            this, SLOT(cellClicked(int, int)));
  }

  VibrationDialog::~VibrationDialog()
  {
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
  }

  void VibrationDialog::accept()
  {
    emit selectedMode(-1); // stop animating
    hide();
  }

  void VibrationDialog::cellClicked(int row, int column) {
    qDebug() << " clicked cell";
    emit selectedMode(row);
  }

  void VibrationDialog::reject()
  {
    emit selectedMode(-1); // stop animating
    hide();
  }
  
}

#include "vibrationdialog.moc"
