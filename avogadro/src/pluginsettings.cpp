/**********************************************************************
  PluginSettings - Settings for Plugin Manager

  Copyright (C) 2008,2009 by Tim Vandermeersch

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.openmolecules.net/>

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

#include "pluginsettings.h"
#include "pluginitemmodel.h"
#include "pluginlistview.h"

#include <QPushButton>
#include <QMessageBox>
#include <QDebug>

namespace Avogadro {

  PluginSettings::PluginSettings( QWidget *parent, Qt::WindowFlags f ) : QWidget( parent, f )
  {
    ui.setupUi(this);

    PluginItemModel *model = new PluginItemModel(Plugin::EngineType);
    ui.listView->setModel(model);

    connect(ui.listView, SIGNAL(clicked(QModelIndex)),
        ui.listView, SLOT(selectPlugin(QModelIndex)));
    connect(ui.listView, SIGNAL(clicked(PluginItem*)),
        this, SLOT(selectPlugin(PluginItem*)));
    connect(ui.comboBox, SIGNAL(currentIndexChanged(int)),
        this, SLOT(selectPluginType(int)));
  }

  PluginSettings::~PluginSettings()
  {
  }

  void PluginSettings::selectPlugin( PluginItem *plugin )
  {
    QString text;

    text  = tr("Name: ") + plugin->name() + '\n';
    text  = tr("Identifier: ") + plugin->identifier() + '\n';
    text += tr("File: ") + plugin->fileName() + "\n\n";
    text += tr("Description:\n") + plugin->description() + '\n';

    ui.textEdit->setText(text);
  }

  void PluginSettings::selectPluginType( int index )
  {
    if (ui.listView->model())
      delete ui.listView->model();

    PluginItemModel *model = new PluginItemModel((Plugin::Type)index);
    ui.listView->setModel(model);
    ui.textEdit->setText("");
  }

  void PluginSettings::loadValues()
  {
    PluginItemModel *model = dynamic_cast<PluginItemModel*>(ui.listView->model());
    if (model)
      delete model;

    model = new PluginItemModel((Plugin::Type) ui.comboBox->currentIndex());
    ui.listView->setModel(model);
    ui.textEdit->setText("");
  }
  
  void PluginSettings::saveValues()
  {
    PluginItemModel *model = dynamic_cast<PluginItemModel*>(ui.listView->model());
    if (!model)
      return;
    if (!model->changed())
      return;
        
    QMessageBox::warning(this, tr("Plugin Manager"),
        tr("Avogadro needs to be restarted in order for the changes to take effect"));

    emit reloadPlugins();
  }

}

#include "pluginsettings.moc"
