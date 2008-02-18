/**********************************************************************
  AddEngineDialog - Dialog to add an engine

  Copyright (C) 2007 Donald Ephraim Curtis

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.sourceforge.net/>

  Avogadro is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  Avogadro is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.
 **********************************************************************/

#include "addenginedialog.h"

#include <avogadro/engine.h>

#include <QAbstractButton>
#include <QDialogButtonBox>

namespace Avogadro {

  AddEngineDialog::AddEngineDialog(QWidget *parent) : QDialog(parent)
  {
    ui.setupUi(this);

    connect( ui.typeCombo, SIGNAL(currentIndexChanged (const QString)),
             this, SLOT(typeChanged(const QString)) );
  }

  Engine * AddEngineDialog::getEngine(QWidget *parent, const QList<EngineFactory *> &engineFactories)
  {
    AddEngineDialog dialog(parent);

    foreach(EngineFactory *factory, engineFactories)
    {
      dialog.addType(factory->type());
    }

    int accepted = dialog.exec();
    if(accepted)
    {
      Engine *engine = engineFactories.at(dialog.typeIndex())->createInstance();
      engine->setName(dialog.nameText());
      engine->setEnabled(true);
      engine->setDescription(dialog.descriptionText());
      return engine;
    }

    return 0;
  }

  void AddEngineDialog::addType(const QString &type)
  {
    ui.typeCombo->addItem(type);
  }

  int AddEngineDialog::typeIndex()
  {
    return ui.typeCombo->currentIndex();
  }

  QString AddEngineDialog::nameText()
  {
    return ui.nameEdit->text();
  }

  QString AddEngineDialog::descriptionText()
  {
    return ui.descriptionEdit->text();
  }

  void AddEngineDialog::typeChanged(const QString type)
  {
    ui.nameEdit->setText(type);
  }

} // end namespace Avogadro

#include "addenginedialog.moc"
