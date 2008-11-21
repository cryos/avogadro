/**********************************************************************
  selectionitem.h - ProjectItem for named selections.

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

#ifndef PROJECTPLUGIN_H
#define PROJECTPLUGIN_H

#include <avogadro/global.h>
#include <avogadro/plugin.h>

#include <QtPlugin>
#include <QTreeWidgetItem>

namespace Avogadro {

  class GLWidget;
  class PrimitiveList;
  
  class ProjectItemPrivate;
  class A_EXPORT ProjectItem : public QTreeWidgetItem
  {
    public:
      enum Type {
        /**
         * Static types are not updated, these are items such as labels etc.
         */
        StaticType = QTreeWidgetItem::UserType,
        /**
         * Dynamic types can be updated (removed, changed, ...)
         */
        DynamicType
      };
      /**
       * Constructor
       */
      ProjectItem(QTreeWidgetItem *parent, Type = StaticType);
      /**
       * Constructor
       */
      ~ProjectItem();
      /**
       * Get the primitives for this item
       */
      PrimitiveList primitives() const;
      /**
       * Get the primitives for this item
       */
      void setPrimitives(PrimitiveList &primitives);
      
    private:
      ProjectItemPrivate * const d;
  };

  class ProjectPluginPrivate;   
  class A_EXPORT ProjectPlugin : public QObject, public Plugin
  {
    Q_OBJECT

    public:
      ProjectPlugin();
      /** 
       * Plugin Type 
       */
      Plugin::Type type() const;
 
      /** 
       * Plugin Type Name (Project Items)
       */
      QString typeName() const;
      /**
       * The name for this project item plugin
       */
      virtual QString name() const = 0;
      /**
       * Add the tree items for this project item to parent
       */
      virtual void setupModelData(GLWidget *widget, QTreeWidgetItem *parent) = 0;
      /**
       * @return a QWidget containing the settings or 0
       * if no settings widget is available.
       */
      virtual QWidget *settingsWidget();
      /**
       * Set the alias for this project plugin
       */
      void setAlias(const QString &alias);
      /**
       * @return The alias for this project plugin.
       */
      QString alias() const;
      /**
       * Write the engine settings so that they can be saved between sessions.
       */
      virtual void writeSettings(QSettings &settings) const;
      /**
       * Read in the settings that have been saved for the engine instance.
       */
      virtual void readSettings(QSettings &settings);

    private:
      ProjectPluginPrivate * const d; 
  };
 
} // end namespace Avogadro

#endif
