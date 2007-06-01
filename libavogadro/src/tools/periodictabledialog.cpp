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
#include <QDebug>

namespace Avogadro {

PeriodicTableDialog::PeriodicTableDialog(QWidget *parent)
    : QDialog(parent
#ifdef Q_WS_MAC
            , Qt::Tool
#endif
            )
{
  ui.setupUi(this);

  elementGroup = new QButtonGroup(this);
  unsigned int element = 1;
  foreach(QToolButton *child, findChildren<QToolButton*>()) {
    elementGroup->addButton(child, element++);
    child->setCheckable(true);
  }

  connect(elementGroup, SIGNAL(buttonClicked(int)),
          this, SLOT(buttonClicked(int)));
}

PeriodicTableDialog::~PeriodicTableDialog()
{
}

void PeriodicTableDialog::setSelectedElement(int id)
{
  initialElement = id;
  currentElement = id;
  
  elementGroup->button(id)->setChecked(true);
}

void PeriodicTableDialog::buttonClicked(int id)
{
  qDebug() << " clicked button " << id;
  if (currentElement != id)
    emit elementChanged(id);
  currentElement = id;
}

} // end namespace Avogadro

#include "periodictabledialog.moc"
