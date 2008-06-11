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

#include "supercelldialog.h"

#include <QPushButton>
#include <QButtonGroup>
#include <QDebug>

#include <QFileDialog>
#include <QFile>

#include <QMessageBox>

namespace Avogadro {

  SuperCellDialog::SuperCellDialog( QWidget *parent, Qt::WindowFlags f ):
    QDialog( parent, f ),
    m_millerH(1), m_millerK(0), m_millerL(0)
  {
    ui.setupUi(this);
    reject(); // set to current values
  }

  SuperCellDialog::~SuperCellDialog()
  {
  }

  void SuperCellDialog::buttonClicked(QAbstractButton *button)
  {
    if (button == NULL)
      return;
      
    if (button->text() == tr("OK")) {
      accept();
    } else if (button->text() == tr("Apply")) {
      reject();
    }
  }

  void SuperCellDialog::accept()
  {
    m_millerH = ui.millerHSpinBox->value();
    m_millerK = ui.millerKSpinBox->value();
    m_millerL = ui.millerLSpinBox->value();

    hide();
  }  

  void SuperCellDialog::reject()
  {
    // TODO: This doesn't necessarily restore the old values before the dialog was shown
    ui.millerHSpinBox->setValue(m_millerH);
    ui.millerKSpinBox->setValue(m_millerK);
    ui.millerLSpinBox->setValue(m_millerL);

    hide();
  }

  int SuperCellDialog::millerH()
  {
    return m_millerH;
  }

  void SuperCellDialog::millerH(int h)
  {
    m_millerH = h;
    ui.millerHSpinBox->setValue(m_millerH);
  }

  int SuperCellDialog::millerK()
  {
    return m_millerK;
  }

  void SuperCellDialog::millerK(int k)
  {
    m_millerK = k;
    ui.millerKSpinBox->setValue(m_millerK);
  }
  
  int SuperCellDialog::millerL()
  {
    return m_millerL;
  }

  void SuperCellDialog::millerL(int l)
  {
    m_millerL = l;
    ui.millerLSpinBox->setValue(m_millerL);
  }

}

#include "supercelldialog.moc"
