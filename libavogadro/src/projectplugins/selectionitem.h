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

#ifndef SELECTIONITEM_H
#define SELECTIONITEM_H

#include <avogadro/global.h>
#include <avogadro/plugin.h>
#include <avogadro/projectplugin.h>

namespace Avogadro {

  class A_EXPORT SelectionItems : public ProjectPlugin
  {
    Q_OBJECT

    public:
      SelectionItems() {}
      ~SelectionItems() {}

      QString name() const { return QObject::tr("Named Selections"); }
      void setupModelData(GLWidget *, ProjectItem *parent);
    
    public slots:
      void refresh();
  };
 
  class SelectionItemsFactory : public QObject, public PluginFactory
  {
      Q_OBJECT
      Q_INTERFACES(Avogadro::PluginFactory)

    public:
      Plugin *createInstance(QObject *parent = 0) { return new SelectionItems(); }
      Plugin::Type type() const { return Plugin::ProjectType; };
      QString name() const { return QObject::tr("Named Selections"); };
      QString description() const { return QObject::tr("Named Selections"); };
  };

 
} // end namespace Avogadro

#endif
