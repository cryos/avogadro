/**********************************************************************
  smartsitems.h - ProjectPlugin for smarts selections.

  Copyright (C) 2008 by Tim Vandermeersch

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

#ifndef SMARTSITEMS_H
#define SMARTSITEMS_H

#include <avogadro/global.h>
//#include <avogadro/plugin.h>
#include <avogadro/projectplugin.h>

#include <QString>

#include "ui_smartsitems.h"

namespace Avogadro {

  class Primitive;

  class SmartsSettingsWidget;
  class A_EXPORT SmartsItems : public ProjectPlugin
  {
    Q_OBJECT

    public:
      SmartsItems();
      ~SmartsItems();

      QString name() const { return QObject::tr("Smarts"); }
      
      void setupModelData(GLWidget *, QTreeWidgetItem *parent);
      
      QWidget *settingsWidget();
      void writeSettings(QSettings &settings) const;
      void readSettings(QSettings &settings);

      QString smarts() const;
      
      void update();
    
    public slots:
      void setSmarts(const QString &smarts);
      void primitiveSlot(Primitive*);

    private:
      SmartsSettingsWidget* m_settingsWidget;
      QString m_smarts;
      QTreeWidgetItem *m_label;
      GLWidget *m_widget;
  };
 
  class SmartsSettingsWidget : public QWidget, public Ui::SmartsSettingsWidget
  {
    public:
      SmartsSettingsWidget(QWidget *parent=0) : QWidget(parent) {
        setupUi(this);
      }
  };

  class SmartsItemsFactory : public QObject, public PluginFactory
  {
      Q_OBJECT
      Q_INTERFACES(Avogadro::PluginFactory)

    public:
      Plugin *createInstance(QObject *parent = 0) { return new SmartsItems(); }
      Plugin::Type type() const { return Plugin::ProjectType; };
      QString name() const { return QObject::tr("Smarts"); };
      QString description() const { return QObject::tr("Project item for smarts selections."); };
  };

 
} // end namespace Avogadro

#endif
