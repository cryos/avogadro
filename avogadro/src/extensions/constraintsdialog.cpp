/**********************************************************************
  ForceFieldDialog - Dialog for force field settings

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

#include "constraintsdialog.h"

#include <QPushButton>
#include <QButtonGroup>
#include <QDebug>

#include <QFileDialog>
#include <QFile>

#include <QMessageBox>

namespace Avogadro {

  ConstraintsDialog::ConstraintsDialog( QWidget *parent, Qt::WindowFlags f ) : QDialog( parent, f )
  {
    ui.setupUi(this);

    connect( ui.ConstraintsOK, SIGNAL( clicked() ), this, SLOT( acceptConstraints() ));
    connect( ui.ConstraintsAdd, SIGNAL( clicked() ), this, SLOT( addConstraint() ));
    connect( ui.ConstraintsDelete, SIGNAL( clicked() ), this, SLOT( deleteConstraint() ));
    connect( ui.ConstraintsDeleteAll, SIGNAL( clicked() ), this, SLOT( deleteAllConstraints() ));
    connect( ui.comboType, SIGNAL( currentIndexChanged(int) ), this, SLOT( comboTypeChanged(int) ));
    
    ui.editValue->setMinimum(0.0);
    ui.editValue->setMaximum(0.0);
    ui.editA->setMinimum(0);
    ui.editA->setMaximum(10000);
    ui.editB->setMinimum(0);
    ui.editB->setMaximum(0);
    ui.editC->setMinimum(0);
    ui.editC->setMaximum(0);
    ui.editD->setMinimum(0);
    ui.editD->setMaximum(0);
  }

  ConstraintsDialog::~ConstraintsDialog()
  {
  }

  void ConstraintsDialog::setModel(ConstraintsModel *model)
  {
    m_constraints = model;
    ui.ConstraintsTableView->setModel(m_constraints);
  }
  
  void ConstraintsDialog::setForceField(OpenBabel::OBForceField *forcefield)
  {
    m_forceField = forcefield;
  }
 
  void ConstraintsDialog::comboTypeChanged(int index)
  {
    std::cout << "comboTypeChanged()" << std::endl;
    
    switch (index) {
      case 0:
      case 1:
      case 2:
      case 3:
      case 4:
        ui.editValue->setMinimum(0.0);
        ui.editValue->setMaximum(0.0);
        ui.editA->setMaximum(10000);
        ui.editB->setMaximum(0);
        ui.editC->setMaximum(0);
        ui.editD->setMaximum(0);
        break;
      case 5:
	ui.editValue->setMinimum(0.0);
        ui.editValue->setMaximum(10.0);
	ui.editA->setMaximum(10000);
        ui.editB->setMaximum(10000);
        ui.editC->setMaximum(0);
        ui.editD->setMaximum(0);
        break;
      case 6:
       	ui.editValue->setMinimum(0.0);
        ui.editValue->setMaximum(180.0);
	ui.editA->setMaximum(10000);
        ui.editB->setMaximum(10000);
        ui.editC->setMaximum(10000);
        ui.editD->setMaximum(0);
	break;
      case 7:
        ui.editValue->setMinimum(-180.0);
        ui.editValue->setMaximum(180.0);
	ui.editA->setMaximum(10000);
        ui.editB->setMaximum(10000);
        ui.editC->setMaximum(10000);
        ui.editD->setMaximum(10000);
        break;
    }
  
  }

  void ConstraintsDialog::acceptConstraints()
  {
    m_forceField->SetConstraints(m_constraints->constraints());
    hide();
  }

  void ConstraintsDialog::deleteConstraint()
  {
    std::cout << "deleteConstraint(" << ui.ConstraintsTableView->currentIndex().row() << std::endl;
  }

  void ConstraintsDialog::deleteAllConstraints()
  {
    m_constraints->clear();
    
    this->update();
  }
  
  void ConstraintsDialog::addConstraint()
  {
    
    switch (ui.comboType->currentIndex()) {
      case 0: // Ignore
      case 1: // Atom 
        m_constraints->addAtomConstraint(ui.editA->value());
	break;
      case 2: // Atom X 
        m_constraints->addAtomXConstraint(ui.editA->value());
	break;
      case 3: // Atom Y 
        m_constraints->addAtomYConstraint(ui.editA->value());
	break;
      case 4: // Atom Z 
        m_constraints->addAtomZConstraint(ui.editA->value());
	break;
      case 5: // Bond
        m_constraints->addBondConstraint(ui.editA->value(),
	    ui.editB->value(), ui.editValue->value());
	break;
      case 6: // Angle
        m_constraints->addAngleConstraint(ui.editA->value(),
	    ui.editB->value(), ui.editC->value(), ui.editValue->value());
	break;
      case 7: // Torsion
        m_constraints->addTorsionConstraint(ui.editA->value(),
	    ui.editB->value(), ui.editC->value(), ui.editD->value(),
	    ui.editValue->value());
	break;
    }

  }


}

#include "constraintsdialog.moc"
