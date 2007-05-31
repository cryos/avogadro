/**********************************************************************
  Copyright (C) 2007 Geoffrey R. Hutchison

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

#include "periodictabledialog.h"

#include <QAbstractButton>
#include <QDialogButtonBox>

namespace Avogadro {

PeriodicTableDialog::PeriodicTableDialog(QWidget *parent)
    : QDialog(parent
#ifdef Q_WS_MAC
            , Qt::Tool
#endif
            )
{
  ui.setupUi(this);

  connect(ui.buttonBox, SIGNAL(clicked(QAbstractButton *)),
          this, SLOT(buttonClicked(QAbstractButton *)));
}

PeriodicTableDialog::~PeriodicTableDialog()
{
}

void PeriodicTableDialog::buttonClicked(QAbstractButton *button)
{
  QDialogButtonBox::ButtonRole role = ui.buttonBox->buttonRole(button);
  if(role == QDialogButtonBox::ApplyRole || role == QDialogButtonBox::AcceptRole) {
    //    emit elementChanged signal for current button
  }
}

} // end namespace Avogadro

#include "periodictabledialog.moc"
