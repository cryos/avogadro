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

#include "projecttreemodeldelegate.h"

#include <avogadro/primitivelist.h>

#include <QDebug>
#include <QString>
#include <QObject>

using namespace std;

namespace Avogadro
{

  class ProjectTreeModelDelegatePrivate 
  {
    public:
      ProjectTreeModel *model;
      QString alias;
      QVector<ProjectTreeItem*> expandableItems;
  };

  ProjectTreeModelDelegate::ProjectTreeModelDelegate(ProjectTreeModel *model) : d(new ProjectTreeModelDelegatePrivate)
  {
    d->model = model;
  }
  
  QWidget *ProjectTreeModelDelegate::settingsWidget()
  {
    return 0;
  }

  void ProjectTreeModelDelegate::setAlias(const QString &alias)
  {
    d->alias = alias;
  }

  QString ProjectTreeModelDelegate::alias() const
  {
    if(d->alias.isEmpty()) { return name(); }
    return d->alias;
  }

  void ProjectTreeModelDelegate::writeSettings(QSettings &settings) const
  {
    settings.setValue("name", name());
    settings.setValue("alias", alias());
  }
  
  void ProjectTreeModelDelegate::readSettings(QSettings &settings)
  {
    setAlias(settings.value("alias", name()).toString());
  }
      
  ProjectTreeItem* ProjectTreeModelDelegate::insertExpandableItem(ProjectTreeItem *parent)
  {
    // insert at the end
    int position = parent->childCount();
    d->model->insertRows(parent, position, 1);
    // retrieve the item
    ProjectTreeItem *item = parent->child(position);
    // set it to non-terminal
    item->setTerminal(false);
    // store it (for hasExpandableItem & fetchMore)
    d->expandableItems.append(item);

    return item;
  }
  
  bool ProjectTreeModelDelegate::hasExpandableItem(ProjectTreeItem *parent) const
  {
    return d->expandableItems.contains(parent);
  }
      
  ProjectTreeModel* ProjectTreeModelDelegate::model() const
  {
    return d->model;
  }
      
  void ProjectTreeModelDelegate::exportDelegate(ProjectTreeModelDelegate *delegate)
  {
    d->model->importDelegate(delegate);
  }

} // end namespace Avogadro

#include "projecttreemodeldelegate.moc"

