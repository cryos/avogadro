/**********************************************************************
  ForceFieldDialog - Dialog for Docking

  Copyright (C) 2007 by Tim Vandermeersch

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

#include "pocketdialog.h"

#include <QPushButton>
#include <QButtonGroup>
#include <QDebug>

#include <QFileDialog>
#include <QFile>

#include <QMessageBox>

using namespace OpenBabel;
using namespace std;

namespace Avogadro {

  PocketDialog::PocketDialog( QWidget *parent, Qt::WindowFlags f ) : QDialog( parent, f )
  {
    ui.setupUi(this);
    
    connect( ui.methodCombo, SIGNAL( currentIndexChanged(int) ), this, SLOT( methodToggled(int) ));
    
    m_method = 0; // from selecteion
    m_radius = 10; 
    m_resname = QString(""); 

    ui.methodCombo->setCurrentIndex(m_method);
    ui.radiusSpin->setValue(m_radius);
    ui.resnameText->setText(m_resname);
    ui.resnameText->setEnabled(false);
  }

  PocketDialog::~PocketDialog()
  {
  }

  void PocketDialog::methodToggled(int index)
  {
    if (ui.methodCombo->currentIndex() == 0) // = from selection
      ui.resnameText->setEnabled(false);
    else  
      ui.resnameText->setEnabled(true);
  }

  void PocketDialog::accept()
  {
    m_method = ui.methodCombo->currentIndex();
    m_radius = ui.radiusSpin->value();
    m_resname = ui.resnameText->text();
    QList<Primitive *> selectedAtoms;
    
    if (m_method == 0) { // from selection
      vector<int> ligand;
      selectedAtoms = m_widget->selectedPrimitives();
      for (int i = 0; i < selectedAtoms.size(); ++i) {
        if (selectedAtoms[i]->type() == Primitive::AtomType) {
          Atom *atom = static_cast<Atom *>(selectedAtoms[i]);
	  ligand.push_back(atom->GetIdx());
	}
      }

      qDebug() << "ligand.size() = " << ligand.size() << endl;
      m_pocket = m_dock->CreatePocketFromLigand( *m_molecule, ligand, m_radius);
      qDebug() << "m_pocket.size() = " << m_pocket.size() << endl;
    } else if (m_method == 1) { // from resname
      std::string resname = std::string(m_resname.toStdString());
      m_pocket = m_dock->CreatePocketFromResidue( *m_molecule, resname, m_radius);
    }
    
    // select the pocket atoms 
    vector<int>::iterator j;

    for (j = m_pocket.begin(); j != m_pocket.end(); ++j)
      selectedAtoms.append(static_cast<Atom*>(m_molecule->GetAtom(*j)));

    m_widget->clearSelected();
    m_widget->setSelected(selectedAtoms, true);
    m_widget->update();
 
    hide();
  }

  void PocketDialog::reject()
  {
    ui.methodCombo->setCurrentIndex(m_method);
    ui.radiusSpin->setValue(m_radius);
    ui.resnameText->setText(m_resname);
    
    hide();
  }
  
}

