/**********************************************************************
  ConformerSearchDialog - Dialog for conformer searches

  Copyright (C) 2007 by Tim Vandermeersch

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.cc/>

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

#include "conformersearchdialog.h"
#include "forcefieldextension.h"

#include <openbabel/rotor.h>

#include <QPushButton>
#include <QButtonGroup>
#include <QDebug>

#include <QFileDialog>
#include <QFile>
#include <QString>
#include <QRadioButton>

#include <QMessageBox>

namespace Avogadro {

  ConformerSearchDialog::ConformerSearchDialog( QWidget *parent, Qt::WindowFlags f ) : QDialog( parent, f )
  {
    ui.setupUi(this);

    connect(ui.systematicRadio, SIGNAL( toggled(bool) ), this, SLOT( systematicToggled(bool) ));
    connect(ui.randomRadio, SIGNAL( toggled(bool) ), this, SLOT( randomToggled(bool) ));
    connect(ui.weightedRadio, SIGNAL( toggled(bool) ), this, SLOT( weightedToggled(bool) ));
    connect(ui.geneticRadio, SIGNAL( toggled(bool) ), this, SLOT( geneticToggled(bool) ));

    m_method = 1; // systematic
    m_numConformers = 100;
    m_molecule = NULL;

    ui.numSpin->setValue(0);
    ui.systematicRadio->setChecked(true);
    ui.randomRadio->setChecked(false);
    ui.weightedRadio->setChecked(false);
    ui.geneticRadio->setChecked(false);
    ui.childrenSpinBox->setEnabled(false);
    ui.mutabilitySpinBox->setEnabled(false);
    ui.convergenceSpinBox->setEnabled(false);
    ui.scoringComboBox->setEnabled(false);
  }

  ConformerSearchDialog::~ConformerSearchDialog()
  {
    //  qDebug() << "ConformerSearchDialog::~ConformerSearchDialog()" << endl;
  }

  void ConformerSearchDialog::systematicToggled(bool checked)
  {
    if (!m_molecule)
      return;

    if (checked) {
      m_method = 1;
      ui.systematicRadio->setChecked(true);
      ui.randomRadio->setChecked(false);
      ui.weightedRadio->setChecked(false);
      ui.geneticRadio->setChecked(false);
      ui.childrenSpinBox->setEnabled(false);
      ui.mutabilitySpinBox->setEnabled(false);
      ui.convergenceSpinBox->setEnabled(false);
      ui.scoringComboBox->setEnabled(false);

      OpenBabel::OBRotorList rl;
      OpenBabel::OBMol obmol = m_molecule->OBMol();
      rl.Setup(obmol);
      /*
      OpenBabel::OBRotorIterator ri;
      OpenBabel::OBRotor *rotor = rl.BeginRotor(ri);
      int numConformers = 0;
      for (int i = 1; i < rl.Size() + 1; i++, rotor = rl.NextRotor(ri)) // foreach rotor
        for (unsigned int j = 0; j < rotor->GetResolution().size(); j++) { // foreach torsion
	  numConformers++;
        }
      */
      OpenBabel::OBRotorKeys rotorKeys;
      OpenBabel::OBRotorIterator ri;
      OpenBabel::OBRotor *rotor = rl.BeginRotor(ri);
      for (size_t i = 1; i < rl.Size() + 1; ++i, rotor = rl.NextRotor(ri)) // foreach rotor
        rotorKeys.AddRotor(rotor->GetResolution().size());
    
      ui.numSpin->setEnabled(false);
      ui.numSpin->setValue(rotorKeys.NumKeys());
    }
  }
  
  void ConformerSearchDialog::randomToggled(bool checked)
  {
    if (checked) {
      m_method = 2;
      ui.systematicRadio->setChecked(false);
      ui.randomRadio->setChecked(true);
      ui.weightedRadio->setChecked(false);
      ui.geneticRadio->setChecked(false);
      ui.childrenSpinBox->setEnabled(false);
      ui.mutabilitySpinBox->setEnabled(false);
      ui.convergenceSpinBox->setEnabled(false);
      ui.scoringComboBox->setEnabled(false);
      ui.numSpin->setEnabled(true);
      ui.numSpin->setValue(100);
    }
  }
  
  void ConformerSearchDialog::weightedToggled(bool checked)
  {
    if (checked) {
      m_method = 3;
      ui.systematicRadio->setChecked(false);
      ui.randomRadio->setChecked(false);
      ui.weightedRadio->setChecked(true);
      ui.geneticRadio->setChecked(false);
      ui.childrenSpinBox->setEnabled(false);
      ui.mutabilitySpinBox->setEnabled(false);
      ui.convergenceSpinBox->setEnabled(false);
      ui.scoringComboBox->setEnabled(false);
      ui.numSpin->setEnabled(true);
      ui.numSpin->setValue(100);
    }
  }

  void ConformerSearchDialog::geneticToggled(bool checked)
  {
    if (checked) {
      m_method = 4;
      ui.systematicRadio->setChecked(false);
      ui.randomRadio->setChecked(false);
      ui.weightedRadio->setChecked(false);
      ui.geneticRadio->setChecked(true);
      ui.childrenSpinBox->setEnabled(true);
      ui.mutabilitySpinBox->setEnabled(true);
      ui.convergenceSpinBox->setEnabled(true);
      ui.scoringComboBox->setEnabled(true);
      ui.numSpin->setEnabled(true);
      ui.numSpin->setValue(50);
    }
  }
 
  void ConformerSearchDialog::showEvent(QShowEvent *)
  {
    OpenBabel::OBRotorList rl;
    OpenBabel::OBMol obmol = m_molecule->OBMol();
    rl.Setup(obmol);
     
    if (rl.Size() > 10)
      weightedToggled(true);
    else
      systematicToggled(true);
 
    ui.atomLabel->setText(QString(tr("Number of atoms: %1")).arg(m_molecule->numAtoms()));
    ui.bondsLabel->setText(QString(tr("Number of rotatable bonds: %1")).arg(rl.Size()));
  }
  
   QUndoCommand* ConformerSearchDialog::setup(Molecule *molecule, OpenBabel::OBForceField* forceField, 
      ConstraintsModel* constraints, int forceFieldID, 
      int nSteps, int algorithm, int convergence)
  {
    m_molecule = molecule;
    
    m_forceFieldCommand = new ForceFieldCommand( m_molecule, forceField, constraints, 
        forceFieldID, nSteps, algorithm, convergence, 0 );

    return NULL;
  }

  void ConformerSearchDialog::accept()
  {
    //  qDebug() << "ConformerSearchDialog::accept()";
    m_numConformers = ui.numSpin->value();

    static_cast<ForceFieldCommand*>(m_forceFieldCommand)->setTask(m_method);
    static_cast<ForceFieldCommand*>(m_forceFieldCommand)
      ->setNumConformers(m_numConformers);
    static_cast<ForceFieldCommand*>(m_forceFieldCommand)
      ->setNumChildren(ui.childrenSpinBox->value());
    static_cast<ForceFieldCommand*>(m_forceFieldCommand)
      ->setMutability(ui.mutabilitySpinBox->value());
    static_cast<ForceFieldCommand*>(m_forceFieldCommand)
      ->setConvergence(ui.convergenceSpinBox->value());
    static_cast<ForceFieldCommand*>(m_forceFieldCommand)
      ->setMethod(ui.scoringComboBox->currentIndex());
    m_forceFieldCommand->redo();
    

    hide();
  }

  void ConformerSearchDialog::reject()
  {
    //  qDebug() << "ConformerSearchDialog::reject()" << endl;
    hide();
  }

  int ConformerSearchDialog::numConformers()
  {
    return m_numConformers;
  }

  int ConformerSearchDialog::method()
  {
    return m_method;
  }
}

