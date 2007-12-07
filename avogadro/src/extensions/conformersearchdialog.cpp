/**********************************************************************
  ConformerSearchDialog - Dialog for conformer searches

  Copyright (C) 2007 by Tim Vandermeersch

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.sourceforge.net/>

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
    //  qDebug() << "ConformerSearchDialog::ConformerSearchDialog()" << endl;

    ui.setupUi(this);

    connect(ui.systematicRadio, SIGNAL( toggled(bool) ), this, SLOT( systematicToggled(bool) ));
    connect(ui.randomRadio, SIGNAL( toggled(bool) ), this, SLOT( randomToggled(bool) ));
    connect(ui.weightedRadio, SIGNAL( toggled(bool) ), this, SLOT( weightedToggled(bool) ));

    m_method = 1; // systematic
    m_numConformers = 100;
    m_molecule = NULL;

    ui.numSpin->setValue(0);
    ui.systematicRadio->setChecked(true);
    ui.randomRadio->setChecked(false);
    ui.weightedRadio->setChecked(false);
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
      OpenBabel::OBRotorList rl;
      rl.Setup((OpenBabel::OBMol&)*m_molecule);
      OpenBabel::OBRotorIterator ri;
      OpenBabel::OBRotor *rotor = rl.BeginRotor(ri);
      int numConformers = 0;
      for (int i = 1; i < rl.Size() + 1; i++, rotor = rl.NextRotor(ri)) // foreach rotor
        for (unsigned int j = 0; j < rotor->GetResolution().size(); j++) { // foreach torsion
	  numConformers++;
        }
     
      ui.numSpin->setEnabled(false);
      ui.numSpin->setValue(numConformers);
    }
  }
  
  void ConformerSearchDialog::randomToggled(bool checked)
  {
    if (checked) {
      m_method = 2;
      ui.systematicRadio->setChecked(false);
      ui.randomRadio->setChecked(true);
      ui.weightedRadio->setChecked(false);
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
      ui.numSpin->setEnabled(true);
      ui.numSpin->setValue(100);
    }
  }
 
  void ConformerSearchDialog::showEvent(QShowEvent *event)
  {
    OpenBabel::OBRotorList rl;
    rl.Setup((OpenBabel::OBMol&)*m_molecule);
     
    if (rl.Size() > 10)
      weightedToggled(true);
    else
      systematicToggled(true);
 
    ui.atomLabel->setText(QString("Number of atoms: %1").arg(m_molecule->NumAtoms()));
    ui.bondsLabel->setText(QString("Number of rotatable bonds: %1").arg(rl.Size()));
  }
  
   QUndoCommand* ConformerSearchDialog::setup(Molecule *molecule, OpenBabel::OBForceField* forceField, 
      ConstraintsModel* constraints, QTextEdit *textEdit, int forceFieldID, 
      int nSteps, int algorithm, int gradients, int convergence)
  {
    m_molecule = molecule;
    
    m_forceFieldCommand = new ForceFieldCommand( m_molecule, forceField, constraints, textEdit, 
        forceFieldID, nSteps, algorithm, gradients, convergence, 0 );

    return NULL;
  }

  void ConformerSearchDialog::accept()
  {
    //  qDebug() << "ConformerSearchDialog::accept()";

    ((ForceFieldCommand*)m_forceFieldCommand)->setTask(m_method);
    m_forceFieldCommand->redo();
    
    m_numConformers = ui.numSpin->value();

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

#include "conformersearchdialog.moc"
