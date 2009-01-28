/**********************************************************************
  PrimitiveItemModel - Model for representing primitives.

  Copyright (C) 2007 Donald Ephraim Curtis <dcurtis3@sourceforge.net>

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

#include "projecttreemodel.h"
#include "projecttreemodeldelegate.h"

#include <QTimer>
#include <QVector>
#include <QDebug>

#include <avogadro/atom.h>
#include <avogadro/bond.h>
#include <avogadro/residue.h>
#include <avogadro/molecule.h>

#include "projectdelegates/labeldelegate.h"
#include "projectdelegates/moleculedelegate.h"
#include "projectdelegates/atomdelegate.h"
#include "projectdelegates/bonddelegate.h"
#include "projectdelegates/residuedelegate.h"
#include "projectdelegates/selectiondelegate.h"

namespace Avogadro {

  class ProjectTreeModelPrivate
  {
    public:
      ProjectTreeModelPrivate() : glWidget(0), rootItem(0)
      {
      }

      GLWidget *glWidget;
      ProjectTreeItem *rootItem;

      QList<ProjectTreeModelDelegate*> delegates;
  };

  ProjectTreeModel::ProjectTreeModel(GLWidget *widget, QObject *parent) : 
      QAbstractItemModel(parent), d(new ProjectTreeModelPrivate)
  {
    d->glWidget = widget;

    QVector<QVariant> rootData;
    rootData << tr("tree");
    rootData << tr("index");
    d->rootItem = new ProjectTreeItem(rootData);
    d->rootItem->setTerminal(false);
    

    init();
  }

  ProjectTreeModel::~ProjectTreeModel()
  {
    foreach (ProjectTreeModelDelegate *delegate, d->delegates)
      delete delegate;

    delete d->rootItem;

    delete d;
  }

  ProjectTreeItem* ProjectTreeModel::item(const QModelIndex& index) const
  {
    if (index.isValid()) {
      ProjectTreeItem *item = static_cast<ProjectTreeItem*>(index.internalPointer());
      if (item) 
        return item;
    }
    return d->rootItem;
  }

  QModelIndex ProjectTreeModel::parent( const QModelIndex & index ) const
  {
    if(!index.isValid())
      return QModelIndex();

    ProjectTreeItem *childItem = item(index);
    ProjectTreeItem *parentItem = childItem->parent();

    if (parentItem == d->rootItem)
      return QModelIndex();

    return createIndex(parentItem->childNumber(), 0, parentItem);
  }

  int ProjectTreeModel::rowCount( const QModelIndex & parent ) const
  {
    ProjectTreeItem *parentItem = item(parent);
    return parentItem->childCount();
  }

  int ProjectTreeModel::columnCount( const QModelIndex & ) const
  {
    return d->rootItem->columnCount();
  }

  QVariant ProjectTreeModel::data (const QModelIndex & index, int role) const
  {
    if (!index.isValid())
      return QVariant();

    if (role != Qt::DisplayRole)
      return QVariant();

    ProjectTreeItem *projectItem = item(index);

    if (projectItem)
      if (index.column() < projectItem->columnCount())
        return projectItem->data(index.column());

    return QVariant();
  }
      
  QVariant ProjectTreeModel::headerData( int section, Qt::Orientation orientation, int role ) const
  {
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
      return d->rootItem->data(section);

    return QVariant();
  }

  Qt::ItemFlags ProjectTreeModel::flags ( const QModelIndex & index ) const
  {
    if(!index.isValid())
      return 0;

    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
  }

  QModelIndex ProjectTreeModel::index ( int row, int column, const QModelIndex & parent ) const
  {
    if(parent.isValid() && parent.column() != 0)
      return QModelIndex();

    ProjectTreeItem *parentItem = item(parent);
    ProjectTreeItem *childItem = parentItem->child(row);
    if (childItem)
      return createIndex(row, column, childItem);
    else
      return QModelIndex();
  }

  void ProjectTreeModel::init()
  {

    QList<ProjectTreeItem*> parents;
    QList<int> indentations;
    parents << d->rootItem;
    indentations << 0;
    
    QSettings settings;
    settings.beginGroup("projectTree");
    int size = settings.beginReadArray("items");
    
    if (size == 0) { // default (i.e., never started a project tree)
       // Start with a molecule delegate
       ProjectTreeModelDelegate *delegate = (ProjectTreeModelDelegate*) new MoleculeDelegate(this);
       delegate->initStructure(d->glWidget, parents.last());
       d->delegates.append(delegate);
    }
    else {
    for (int i = 0; i < size; ++i) {
      settings.setArrayIndex( i );
      int position = settings.value("indent").toInt();

      if (position > indentations.last()) {
        // The last child of the current parent is now the new parent

        // unless the current parent has no children.
        if (parents.last()->childCount() > 0) {
          parents << parents.last()->child(parents.last()->childCount()-1);
            indentations << position;
        }
      } else {
        while (position < indentations.last() && parents.count() > 0) {
          parents.pop_back();
          indentations.pop_back();
        }
      }
 
      ProjectTreeModelDelegate *delegate = 0;
      if (settings.value("name").toString() == tr("Label")) {
        delegate = (ProjectTreeModelDelegate*) new LabelDelegate(this);
     } else if (settings.value("name").toString() == tr("Molecule")) {
        delegate = (ProjectTreeModelDelegate*) new MoleculeDelegate(this);
      } else if (settings.value("name").toString() == tr("Bonds")) {
        delegate = (ProjectTreeModelDelegate*) new BondDelegate(this);
      } else if (settings.value("name").toString() == tr("Atoms")) {
        delegate = (ProjectTreeModelDelegate*) new AtomDelegate(this);
      } else if (settings.value("name").toString() == tr("Residues")) {
        delegate = (ProjectTreeModelDelegate*) new ResidueDelegate(this);
      } else if (settings.value("name").toString() == tr("User Selections")) {
        delegate = (ProjectTreeModelDelegate*) new SelectionDelegate(this);
      }

      if (delegate) {
        delegate->readSettings(settings);
        delegate->initStructure(d->glWidget, parents.last());
        d->delegates.append(delegate);
      }
      
    }
  }

    settings.endArray();
    settings.endGroup(); 
 
  }
      
  bool ProjectTreeModel::insertRows(ProjectTreeItem *parentItem, int position, int rows)
  {
    bool success;
    assert(position > -1);
    beginInsertRows(createIndex(parentItem->childNumber(), 0, parentItem), position, position + rows - 1);
    success = parentItem->insertChildren(position, rows, d->rootItem->columnCount());
    endInsertRows();

    return success;
  }

  bool ProjectTreeModel::removeRows(ProjectTreeItem *parentItem, int position, int rows)
  {
    bool success;
    assert(position > -1);
    beginRemoveRows(createIndex(parentItem->childNumber(),0,parentItem), position, position + rows - 1);
    success = parentItem->removeChildren(position, rows);
    endRemoveRows();
     
    return success;
  }
      
  void ProjectTreeModel::emitDataChanged(ProjectTreeItem *parentItem, int row)
  {
    QModelIndex left = createIndex(row, 0, parentItem->child(row));
    QModelIndex right = createIndex(row, d->rootItem->columnCount(), parentItem->child(row));
    emit dataChanged( left, right );
  }
   
  bool ProjectTreeModel::hasChildren(const QModelIndex &parent) const
  {
    ProjectTreeItem *parentItem = item(parent);
    return !parentItem->isTerminal();
  }

  bool ProjectTreeModel::canFetchMore(const QModelIndex& parent) const
  {
    // if we might have children, more data could possibly be fetched...
    return hasChildren(parent);
  }
      
  void ProjectTreeModel::fetchMore(const QModelIndex& parent)
  {
    if(!parent.isValid())
      return;
    
    ProjectTreeItem *parentItem = item(parent);
    if (!parentItem)
      return;

    foreach (ProjectTreeModelDelegate *delegate, d->delegates) {
      if (delegate->hasExpandableItem(parentItem)) {
        delegate->fetchMore(parentItem);
      }
    }
  }
      
  void ProjectTreeModel::importDelegate(ProjectTreeModelDelegate *delegate)
  {
    d->delegates.append(delegate);
  }


} // end namespace Avogadro

#include "projecttreemodel.moc"
