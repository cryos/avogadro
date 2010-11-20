/**********************************************************************
  DynamicDisplayExtension

  Copyright (C) 2010 David C. Lonie

  This library is free software; you can redistribute it and/or modify
  it under the terms of the GNU Library General Public License as
  published by the Free Software Foundation; either version 2.1 of the
  License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
 ***********************************************************************/

#include "dynamicdisplaydialog.h"

#include "dynamicdisplayextension.h"

#include <avogadro/molecule.h>
#include <avogadro/atom.h>

using namespace Avogadro;

namespace DynamicDisplay {

  DynamicDisplayDialog::DynamicDisplayDialog(QWidget *parent,
                                             DynamicDisplayExtension *ext)
    : QDialog( parent ),
      m_ext(ext)
  {
    ui.setupUi(this);

    // Extension connections
    connect(m_ext, SIGNAL(resetExtension()),
            this, SLOT(reset()));
    connect(m_ext, SIGNAL(updateExtension(Avogadro::Molecule*)),
            this, SLOT(update(Avogadro::Molecule*)));

    // Dialog connections
    connect(ui.push_hide, SIGNAL(clicked()),
            this, SLOT(hide()));
  }

  void DynamicDisplayDialog::reset()
  {
    // Reset the label
    ui.label_numHydrogens->setText(tr("N/A"));
  }

  void DynamicDisplayDialog::update(Molecule *mol)
  {
    // Count the number of hydrogen atoms:
    unsigned int count = 0;
    for (int i = 0; i < mol->atoms().size(); i++) {
      if (mol->atoms().at(i)->atomicNumber() == 1) {
        count++;
      }
    }

    // Display the number of hydrogens
    ui.label_numHydrogens->setText(QString::number(count));
  }

}
#include "dynamicdisplaydialog.moc"
