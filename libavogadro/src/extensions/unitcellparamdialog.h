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

#ifndef UNITCELLPARAMDIALOG_H
#define UNITCELLPARAMDIALOG_H

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
      explicit UnitCellParamDialog( QWidget *parent = 0, Qt::WindowFlags f = 0 );
      //! Desconstructor
      ~UnitCellParamDialog();

      int aCells();
      int bCells();
      int cCells();

      void aCells(int a);
      void bCells(int b);
      void cCells(int c);

      void aLength(double a);
      void bLength(double b);
      void cLength(double c);

      double aLength();
      double bLength();
      double cLength();

      void alpha(double a);
      void beta(double b);
      void gamma(double g);

      double alpha();
      double beta();
      double gamma();

    public slots:
      void accept();
      void reject();
      void valueChanged(int value);
      void valueChanged(double value);
      
      void buttonClicked(QAbstractButton *button);
      void deleteCellClicked();
      void fillCellClicked();

    signals:
      void unitCellDisplayChanged(int a, int b, int c);
      void unitCellParametersChanged(double a, double b, double c,
                                     double alpha, double beta, double gamma);

      void deleteUnitCell();
      void fillUnitCell();

    private:
      Ui::UnitCellParamDialog ui;

      int m_aCells;
      int m_bCells;
      int m_cCells;

      double m_aLength;
      double m_bLength;
      double m_cLength;

      double m_alpha;
      double m_beta;
      double m_gamma;
  };
}

#endif
