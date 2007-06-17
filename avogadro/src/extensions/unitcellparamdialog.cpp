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

UnitCellParamDialog::UnitCellParamDialog( QWidget *parent, Qt::WindowFlags f ):
QDialog( parent, f ),
m_aCells(0), m_bCells(0), m_cCells(0),
m_aLength(0.0), m_bLength(0.0), m_cLength(0.0),
m_alpha(90.0), m_beta(90.0), m_gamma(90.0)
{
  ui.setupUi(this);

  reject(); // set to current values
}

UnitCellParamDialog::~UnitCellParamDialog()
{
}

void UnitCellParamDialog::accept()
{
  m_aCells = ui.aCellSpinBox->value();
  m_bCells = ui.bCellSpinBox->value();
  m_cCells = ui.cCellSpinBox->value();

  m_aLength = ui.aLengthSpinBox->value();
  m_bLength = ui.bLengthSpinBox->value();
  m_cLength = ui.cLengthSpinBox->value();

  m_alpha = ui.alphaSpinBox->value();
  m_beta = ui.betaSpinBox->value();
  m_gamma = ui.gammaSpinBox->value();

  emit(unitCellDisplayChanged(m_aCells, m_bCells, m_cCells));
  emit(unitCellParametersChanged(m_aLength, m_bLength, m_cLength,
                                 m_alpha, m_beta, m_gamma));

  hide();
}

void UnitCellParamDialog::reject()
{
  ui.aCellSpinBox->setValue(m_aCells);
  ui.bCellSpinBox->setValue(m_bCells);
  ui.cCellSpinBox->setValue(m_cCells);

  ui.aLengthSpinBox->setValue(m_aLength);
  ui.bLengthSpinBox->setValue(m_bLength);
  ui.cLengthSpinBox->setValue(m_cLength);

  ui.alphaSpinBox->setValue(m_alpha);
  ui.betaSpinBox->setValue(m_beta);
  ui.gammaSpinBox->setValue(m_gamma);

  hide();
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

double UnitCellParamDialog::aLength()
{
  return m_aLength;
}

void UnitCellParamDialog::aLength(double a)
{
  m_aLength = a;
  ui.aLengthSpinBox->setValue(a);
}

double UnitCellParamDialog::bLength()
{
  return m_bLength;
}

void UnitCellParamDialog::bLength(double b)
{
  m_bLength = b;
  ui.bLengthSpinBox->setValue(b);
}

double UnitCellParamDialog::cLength()
{
  return m_cLength;
}

void UnitCellParamDialog::cLength(double c)
{
  m_cLength = c;
  ui.cLengthSpinBox->setValue(c);
}

double UnitCellParamDialog::alpha()
{
  return m_alpha;
}

void UnitCellParamDialog::alpha(double a)
{
  m_alpha = a;
  ui.alphaSpinBox->setValue(a);
}

double UnitCellParamDialog::beta()
{
  return m_beta;
}

void UnitCellParamDialog::beta(double b)
{
  m_beta = b;
  ui.betaSpinBox->setValue(b);
}

double UnitCellParamDialog::gamma()
{
  return m_gamma;
}

void UnitCellParamDialog::gamma(double g)
{
  m_gamma = g;
  ui.gammaSpinBox->setValue(g);
}

#include "unitcellparamdialog.moc"
