/**********************************************************************
  SuperCellDialog - Dialog for building crystallographic "super cells"

  Copyright (C) 2007-2008 by Geoffrey R. Hutchison

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

#ifndef SUPERCELLDIALOG_H
#define SUPERCELLDIALOG_H

#include <QDialog>
#include <QButtonGroup>
#include <QModelIndex>

#include "ui_supercelldialog.h"

namespace Avogadro
{
  class SuperCellDialog : public QDialog
  {
      Q_OBJECT

    public:
      //! Constructor
      explicit SuperCellDialog( QWidget *parent = 0, Qt::WindowFlags f = 0 );
      //! Destructor
      ~SuperCellDialog();

      int millerH();
      int millerK();
      int millerL();

      void millerH(int h);
      void millerK(int k);
      void millerL(int l);
      

    public slots:
      void accept();
      void reject();
      
      void buttonClicked(QAbstractButton *button);

    private:
      Ui::SuperCellDialog ui;

      int m_millerH;
      int m_millerK;
      int m_millerL;

  };
}

#endif
