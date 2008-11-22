/**********************************************************************
  selectionitem.h - Base class for ProjectItem plugins.

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

#include "projectplugin.h"
#include <avogadro/primitivelist.h>

#include <QDebug>
#include <QString>
#include <QObject>

using namespace std;

namespace Avogadro
{

 /* 
  class ProjectItemPrivate
  {
    public:
      PrimitiveList primitives;
  };
  
  ProjectItem::ProjectItem(QTreeWidgetItem *parent, Type type) : QTreeWidgetItem(parent, type), 
      d(new ProjectItemPrivate)
  {
  }

  ProjectItem::~ProjectItem()
  {
    //qDeleteAll(d->childs);
  }
  
  PrimitiveList ProjectItem::primitives() const
  {
    return d->primitives;
  }
  
  void ProjectItem::setPrimitives(PrimitiveList &primitives)
  {
    d->primitives = primitives;
  }
 */
  
  
  class ProjectPluginPrivate 
  {
    public:
      QString alias;
  };

  ProjectPlugin::ProjectPlugin() : d(new ProjectPluginPrivate)
  {
  }
  
  Plugin::Type ProjectPlugin::type() const
  { 
    return Plugin::ToolType; 
  }
  
  QString ProjectPlugin::typeName() const
  { 
    return QObject::tr("Project"); 
  }

  QWidget *ProjectPlugin::settingsWidget()
  {
    return 0;
  }

  void ProjectPlugin::setAlias(const QString &alias)
  {
    d->alias = alias;
  }

  QString ProjectPlugin::alias() const
  {
    if(d->alias.isEmpty()) { return name(); }
    return d->alias;
  }

  void ProjectPlugin::writeSettings(QSettings &settings) const
  {
    settings.setValue("name", name());
    settings.setValue("alias", alias());
  }
  
  void ProjectPlugin::readSettings(QSettings &settings)
  {
    setAlias(settings.value("alias", name()).toString());
  }

} // end namespace Avogadro

#include "projectplugin.moc"

