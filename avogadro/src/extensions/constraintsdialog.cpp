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

  }

  ConstraintsDialog::~ConstraintsDialog()
  {
  }

  void ConstraintsDialog::setModel(ConstraintsModel *model)
  {
    //ui.ConstraintsTableView->setModel(model);
  }

  void ConstraintsDialog::acceptConstraints()
  {
    hide();
  }

  void ConstraintsDialog::deleteConstraint()
  {
  }

  void ConstraintsDialog::addConstraint()
  {
  }


}

#include "constraintsdialog.moc"
