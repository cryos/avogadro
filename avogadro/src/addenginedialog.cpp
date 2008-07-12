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
#include <QtAlgorithms>
#include <QDebug>

namespace Avogadro {

  AddEngineDialog::AddEngineDialog(QWidget *parent) : QDialog(parent)
  {
    ui.setupUi(this);

    connect( ui.typeCombo, SIGNAL(currentIndexChanged (const QString)),
             this, SLOT(typeChanged(const QString)) );
  }

  Engine * AddEngineDialog::getEngine(QWidget *parent, const QList<PluginFactory *> &engineFactories)
  {
    AddEngineDialog dialog(parent);
    QStringList types;

    // We get the list from the PluginFactories in rendering order
    // So we re-sort alphabetically for users
    foreach(PluginFactory *factory, engineFactories)
      types.append(factory->name());
    
    qSort(types);
    
    foreach(const QString &type, types)
      dialog.addType(type);

    int accepted = dialog.exec();
    if(accepted)
    {
      // Find the engine in the list and instantiate it - needed now we sort the list
      Engine *engine = 0;
      foreach(PluginFactory *factory, engineFactories)
        if (factory->name() == types.at(dialog.typeIndex()))
          engine = (Engine *) factory->createInstance();

      // We should always be able to find the engine requested
      if (engine) {
        engine->setName(dialog.nameText());
        engine->setEnabled(true);
        engine->setDescription(dialog.descriptionText());
      }
      else
        qDebug() << "Error - engine not found in engineFactories.";

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
