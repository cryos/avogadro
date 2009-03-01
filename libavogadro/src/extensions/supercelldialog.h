/**********************************************************************
  SuperCellDialog - Dialog for building crystallographic super cells

  Copyright (C) 2009 Marcus D. Hanwell

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.openmolecules.net/>

  This library is free software; you can redistribute it and/or modify
  it under the terms of the GNU Library General Public License as
  published by the Free Software Foundation; either version 2.1 of the
  License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
 ***********************************************************************/

#ifndef SUPERCELLDIALOG_H
#define SUPERCELLDIALOG_H

#include "ui_supercelldialog.h"

#include <QDialog>

namespace Avogadro
{
  class SuperCellDialog : public QDialog
  {
  Q_OBJECT

  public:
    explicit SuperCellDialog( QWidget *parent = 0, Qt::WindowFlags f = 0 );
    ~SuperCellDialog();

    int aCells();
    int bCells();
    int cCells();

    void aCells(int a);
    void bCells(int b);
    void cCells(int c);

  public slots:
    void valueChanged(int value);

    void fillCellClicked();

  signals:
    void cellDisplayChanged(int a, int b, int c);
    void fillCell();

  private:
    Ui::SuperCellDialog ui;

    int m_aCells;
    int m_bCells;
    int m_cCells;
  };
}

#endif
