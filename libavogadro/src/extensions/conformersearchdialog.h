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

#ifndef __CONFORMERSEARCHDIALOG_H
#define __CONFORMERSEARCHDIALOG_H

#include <QDialog>
#include <QButtonGroup>
#include <QModelIndex>

#include <avogadro/primitive.h>
#include <openbabel/forcefield.h>

#include "constraintsmodel.h"

#include "ui_conformersearchdialog.h"

namespace Avogadro
{
  class ConformerSearchDialog : public QDialog
  {
      Q_OBJECT

    public:
      //! Constructor
      explicit ConformerSearchDialog( QWidget *parent = 0, Qt::WindowFlags f = 0 );
      //! Desconstructor
      ~ConformerSearchDialog();

      void showEvent(QShowEvent *event);
      int method();
      int numConformers();
      
      QUndoCommand* setup(Molecule *molecule, OpenBabel::OBForceField* forceField, ConstraintsModel* constraints, 
      int forceFieldID, int nSteps, int algorithm, int gradients, int convergence);
 
    public slots:
      void accept();
      void reject();
      void systematicToggled(bool checked);
      void randomToggled(bool checked);
      void weightedToggled(bool checked);

    private:
      Ui::ConformerSearchDialog ui;

      int m_method;
      int m_numConformers;
      Molecule* m_molecule;
      QUndoCommand* m_forceFieldCommand;
  };
}

#endif
