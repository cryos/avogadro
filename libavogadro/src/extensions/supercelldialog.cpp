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

#include "supercelldialog.h"

namespace Avogadro {

  SuperCellDialog::SuperCellDialog(QWidget *parent, Qt::WindowFlags f) :
    QDialog(parent, f), m_aCells(0), m_bCells(0), m_cCells(0)
  {
    ui.setupUi(this);
    reject();
    
    connect(ui.generateCell, SIGNAL(clicked()), this, SLOT(fillCellClicked()));

    connect(ui.aCellSpinBox, SIGNAL(valueChanged(int)), this, SLOT(valueChanged(int)));    
    connect(ui.bCellSpinBox, SIGNAL(valueChanged(int)), this, SLOT(valueChanged(int)));    
    connect(ui.cCellSpinBox, SIGNAL(valueChanged(int)), this, SLOT(valueChanged(int)));    
  }

  SuperCellDialog::~SuperCellDialog()
  {
  }

  void SuperCellDialog::valueChanged(int)
  {
    m_aCells = ui.aCellSpinBox->value();
    m_bCells = ui.bCellSpinBox->value();
    m_cCells = ui.cCellSpinBox->value();

    emit(cellDisplayChanged(m_aCells, m_bCells, m_cCells));
  }

  int SuperCellDialog::aCells()
  {
    return m_aCells;
  }

  void SuperCellDialog::aCells(int a)
  {
    m_aCells = a;
    ui.aCellSpinBox->setValue(m_aCells);
  }

  int SuperCellDialog::bCells()
  {
    return m_bCells;
  }

  void SuperCellDialog::bCells(int b)
  {
    m_bCells = b;
    ui.bCellSpinBox->setValue(m_bCells);
  }

  int SuperCellDialog::cCells()
  {
    return m_cCells;
  }

  void SuperCellDialog::cCells(int c)
  {
    m_cCells = c;
    ui.cCellSpinBox->setValue(m_cCells);
  }
  
  void SuperCellDialog::fillCellClicked()
  {
    emit fillCell();
  }
  
}

