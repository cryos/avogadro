/**********************************************************************
  ForceFieldDialog - Dialog for force field settings

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

#ifndef FORCEFIELDDIALOG_H
#define FORCEFIELDDIALOG_H

#include <QDialog>
#include <QButtonGroup>
#include <QModelIndex>

#include "ui_forcefielddialog.h"

namespace Avogadro
{
  class ForceFieldDialog : public QDialog
  {
      Q_OBJECT

    public:
      //! Constructor
      explicit ForceFieldDialog( QWidget *parent = 0, Qt::WindowFlags f = 0 );
      //! Desconstructor
      ~ForceFieldDialog();

      int forceFieldID();
      int nSteps();
      int algorithm();
      int convergence();

    public slots:
      void accept();
      void reject();

    private:
      Ui::ForceFieldDialog ui;

      int m_forceFieldID;
      int m_nSteps;
      int m_algorithm;
      int m_convergence;
  };
}

#endif
