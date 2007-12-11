/**********************************************************************
  ForceFieldDialog - Dialog for Docking

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

#include "liganddialog.h"

#include <QPushButton>
#include <QButtonGroup>
#include <QDebug>

#include <QFileDialog>
#include <QFile>

#include <QMessageBox>

using namespace OpenBabel;
using namespace std;

namespace Avogadro {

  LigandDialog::LigandDialog( QWidget *parent, Qt::WindowFlags f ) : QDialog( parent, f )
  {
    ui.setupUi(this);
    
    connect( ui.methodCombo, SIGNAL( currentIndexChanged(int) ), this, SLOT( methodToggled(int) ));
    
    m_method = 0; // from selecteion
    m_resname = QString(""); 

    ui.methodCombo->setCurrentIndex(m_method);
    ui.resnameText->setText(m_resname);
    ui.resnameText->setEnabled(false);
  }

  LigandDialog::~LigandDialog()
  {
  }

  void LigandDialog::methodToggled(int index)
  {
    if (ui.methodCombo->currentIndex() == 0) // = from selection
      ui.resnameText->setEnabled(false);
    else if (ui.methodCombo->currentIndex() == 1) // = from resname
      ui.resnameText->setEnabled(true);
    else if (ui.methodCombo->currentIndex() == 2) { // = from file
      m_fileName = QFileDialog::getOpenFileName(this, tr("Open ligand file"), ".", tr("Avogadro files (*.*)"));
      
      if (m_fileName.isEmpty()) {
        ui.methodCombo->setCurrentIndex(0);
        ui.resnameText->setEnabled(false);
      } else 
	hide();
    }
  }

  void LigandDialog::accept()
  {
    m_method = ui.methodCombo->currentIndex();
    m_resname = ui.resnameText->text();
    QList<Primitive *> selectedAtoms;
    
    if (m_method == 0) { // from selection
      m_ligand.clear();
      selectedAtoms = m_widget->selectedPrimitives();
      
      for (int i = 0; i < selectedAtoms.size(); ++i) {
        if (selectedAtoms[i]->type() == Primitive::AtomType) {
          Atom *atom = static_cast<Atom *>(selectedAtoms[i]);
	  m_ligand.push_back(atom->GetIdx());
	}
      }
    } else if (m_method == 1) { // from resname
      string resname = string(m_resname.toStdString());
      m_ligand = m_dock->CreateLigandFromResidue( *m_molecule, resname);
    }
    
    // select the ligand atoms 
    vector<int>::iterator j;

    for (j = m_ligand.begin(); j != m_ligand.end(); ++j)
      selectedAtoms.append(static_cast<Atom*>(m_molecule->GetAtom(*j)));

    m_widget->clearSelected();
    m_widget->setSelected(selectedAtoms, true);
    m_widget->update();
 
    hide();
  }

  void LigandDialog::reject()
  {
    ui.methodCombo->setCurrentIndex(m_method);
    ui.resnameText->setText(m_resname);
    
    hide();
  }
  
}

#include "liganddialog.moc"
