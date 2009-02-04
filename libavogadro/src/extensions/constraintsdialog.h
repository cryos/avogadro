/**********************************************************************
  ConstraintsDialog - Dialog for force field constraint settings

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

#ifndef CONSTRAINTSDIALOG_H
#define CONSTRAINTSDIALOG_H

#include "constraintsmodel.h"

#include <QDialog>
#include <QButtonGroup>
#include <QModelIndex>
#include <QTableView>

#include "ui_constraintsdialog.h"

namespace Avogadro
{
  class ConstraintsDialog : public QDialog
  {
      Q_OBJECT

    public:
      //! Constructor
      explicit ConstraintsDialog( QWidget *parent = 0, Qt::WindowFlags f = 0 );
      //! Desconstructor
      ~ConstraintsDialog();

      void showEvent(QShowEvent *event);
      void setModel(ConstraintsModel *model);
      void setMolecule(Molecule *molecule);

    public slots:
      void acceptConstraints();
      void deleteConstraint();
      void deleteAllConstraints();
      void addConstraint();
      void comboTypeChanged(int);

    private:
      Ui::ConstraintsDialog ui;
      Molecule* m_molecule;
      ConstraintsModel *m_constraints;
      OpenBabel::OBForceField* m_forceField; // needed to transfer info between AutoOpt and FF Extension
  };
}

#endif
