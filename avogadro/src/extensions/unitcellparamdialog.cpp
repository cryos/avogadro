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

#include "unitcellparamdialog.h"

#include <QPushButton>
#include <QButtonGroup>
#include <QDebug>

#include <QFileDialog>
#include <QFile>

#include <QMessageBox>

using namespace Avogadro;

UnitCellParamDialog::UnitCellParamDialog( QWidget *parent, Qt::WindowFlags f ) : QDialog( parent, f )
{
  ui.setupUi(this);

  ui.aCellSpinBox->setValue(m_aCells);
  ui.bCellSpinBox->setValue(m_bCells);
  ui.cCellSpinBox->setValue(m_cCells);
}

UnitCellParamDialog::~UnitCellParamDialog()
{
}

void UnitCellParamDialog::accept()
{
  m_aCells = ui.aCellSpinBox->value();
  m_bCells = ui.bCellSpinBox->value();
  m_cCells = ui.cCellSpinBox->value();

emit(unitCellsChanged(m_aCells, m_bCells, m_cCells));

  hide();
}

void UnitCellParamDialog::reject()
{
  ui.aCellSpinBox->setValue(m_aCells);
  ui.bCellSpinBox->setValue(m_bCells);
  ui.cCellSpinBox->setValue(m_cCells);
}

int UnitCellParamDialog::aCells()
{
  return m_aCells;
}

void UnitCellParamDialog::aCells(int a)
{
  m_aCells = a;
  ui.aCellSpinBox->setValue(m_aCells);
} 

int UnitCellParamDialog::bCells()
{
  return m_bCells;
}

void UnitCellParamDialog::bCells(int b)
{
  m_bCells = b;
  ui.bCellSpinBox->setValue(m_bCells);
} 

int UnitCellParamDialog::cCells()
{
  return m_cCells;
}

void UnitCellParamDialog::cCells(int c)
{
  m_cCells = c;
  ui.cCellSpinBox->setValue(m_cCells);
} 

#include "unitcellparamdialog.moc"
