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

#include "forcefielddialog.h"

#include <QPushButton>
#include <QButtonGroup>
#include <QDebug>

#include <QFileDialog>
#include <QFile>

#include <QMessageBox>

#include <openbabel/plugin.h>

using namespace OpenBabel;

namespace Avogadro {

  ForceFieldDialog::ForceFieldDialog( QWidget *parent, Qt::WindowFlags f ) : QDialog( parent, f )
  {
    //  qDebug() << "ForceFieldDialog::ForceFieldDialog()" << endl;

    ui.setupUi(this);
    std::vector<std::string> forcefieldList;
    OBPlugin::ListAsVector("forcefields", "ids", forcefieldList);
    ui.ForceFieldComboBox->clear();
    for (unsigned int i = 0; i < forcefieldList.size(); ++i)
      ui.ForceFieldComboBox->addItem(forcefieldList[i].c_str());

    m_forceFieldID = ui.ForceFieldComboBox->findText("MMFF94");
    ui.ForceFieldComboBox->setCurrentIndex(m_forceFieldID);

    m_nSteps = 500; // a reasonable starting point (not too much time)
    ui.StepsSpinBox->setValue(m_nSteps);

    m_algorithm = 0; // steepest descent
    ui.AlgorithmComboBox->setCurrentIndex(m_algorithm);

    m_gradients = 1; // analytical
    ui.GradientsComboBox->setCurrentIndex(m_gradients);

    m_convergence = 7;
    ui.ConvergenceSpinBox->setValue(m_convergence);
  }

  ForceFieldDialog::~ForceFieldDialog()
  {
    //  qDebug() << "ForceFieldDialog::~ForceFieldDialog()" << endl;
  }

  void ForceFieldDialog::accept()
  {
    //  qDebug() << "ForceFieldDialog::accept()";
    qDebug() << "Force Field: " << ui.ForceFieldComboBox->currentIndex();
    qDebug() << "Nymber of steps: " << ui.StepsSpinBox->value();
    qDebug() << "Algorithm: " << ui.AlgorithmComboBox->currentIndex();
    qDebug() << "Gradients: " << ui.GradientsComboBox->currentIndex();

    m_forceFieldID = ui.ForceFieldComboBox->currentIndex();
    m_nSteps = ui.StepsSpinBox->value();
    m_algorithm = ui.AlgorithmComboBox->currentIndex();
    m_gradients = ui.GradientsComboBox->currentIndex();
    m_convergence = ui.ConvergenceSpinBox->value();

    hide();
  }

  void ForceFieldDialog::reject()
  {
    //  qDebug() << "ForceFieldDialog::reject()" << endl;

    ui.ForceFieldComboBox->setCurrentIndex(m_forceFieldID);
    ui.StepsSpinBox->setValue(m_nSteps);
    ui.AlgorithmComboBox->setCurrentIndex(m_algorithm);
    ui.GradientsComboBox->setCurrentIndex(m_gradients);
    ui.ConvergenceSpinBox->setValue(m_convergence);
  }

  int ForceFieldDialog::forceFieldID()
  {
    return m_forceFieldID;
  }

  int ForceFieldDialog::nSteps()
  {
    return m_nSteps;
  }

  int ForceFieldDialog::algorithm()
  {
    return m_algorithm;
  }

  int ForceFieldDialog::gradients()
  {
    return m_gradients;
  }

  int ForceFieldDialog::convergence()
  {
    return m_convergence;
  }
}

#include "forcefielddialog.moc"
