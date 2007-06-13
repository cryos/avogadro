/**********************************************************************
  UnitCellParamDialog - Dialog for crystallographic unit cell parameters

  Copyright (C) 2007 by Geoffrey R. Hutchison

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

#ifndef __UNITCELLPARAMDIALOG_H
#define __UNITCELLPARAMDIALOG_H

#include <QDialog>
#include <QButtonGroup>
#include <QModelIndex>

#include "ui_unitcellparamdialog.h"

namespace Avogadro
{
  class UnitCellParamDialog : public QDialog
  {
      Q_OBJECT

    public:
      //! Constructor
      UnitCellParamDialog( QWidget *parent = 0, Qt::WindowFlags f = 0 );
      //! Desconstructor
      ~UnitCellParamDialog();

      int aCells();
      int bCells();
      int cCells();
      
      void aCells(int a);
      void bCells(int b);
      void cCells(int c);

    public slots:
      void accept();
      void reject();
      
    signals:
      void unitCellsChanged(int a, int b, int c);

    private:
      Ui::UnitCellParamDialog ui;

      int m_aCells;
      int m_bCells;
      int m_cCells;
  };
}

#endif
