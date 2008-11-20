/**********************************************************************
  namedselectionmodel.cpp - Model to hold named selections

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

#include "namedselectionmodel.h"
#include <avogadro/molecule.h>
#include <avogadro/residue.h>
#include <avogadro/color.h>
#include <avogadro/glwidget.h>

#include <QDebug>

using namespace std;
using namespace OpenBabel;

namespace Avogadro
{

  ////////////////////////////////////////////////////////
  //
  // SelectTreeItem
  //
  ////////////////////////////////////////////////////////

  SelectTreeItem::SelectTreeItem(const QList<QVariant> &data, int type, SelectTreeItem *parent)
  {
    m_parentItem = parent;
    m_itemData = data;
    m_type = type;
  }

  SelectTreeItem::~SelectTreeItem()
  {
    qDeleteAll(m_childItems);
  }

  void SelectTreeItem::appendChild(SelectTreeItem *item)
  {
    m_childItems.append(item);
  }

  SelectTreeItem *SelectTreeItem::child(int row)
  {
    return m_childItems.value(row);
  }

  int SelectTreeItem::childCount() const
  {
    return m_childItems.count();
  }

  int SelectTreeItem::columnCount() const
  {
    return m_itemData.count();
  }

  QVariant SelectTreeItem::data(int column) const
  {
    return m_itemData.value(column);
  }

  SelectTreeItem *SelectTreeItem::parent()
  {
    return m_parentItem;
  }

  int SelectTreeItem::row() const
  {
    if (m_parentItem)
      return m_parentItem->m_childItems.indexOf(const_cast<SelectTreeItem*>(this));

    return 0;
  }

  int SelectTreeItem::type() const
  {
    return m_type;
  }

  ////////////////////////////////////////////////////////
  //
  // SelectTreeModel
  //
  ////////////////////////////////////////////////////////

  SelectTreeModel::SelectTreeModel(GLWidget *widget, QObject *parent)
    : QAbstractItemModel(parent)
  {
    QList<QVariant> rootData;
    rootData << "Title" << "Summary";
    rootItem = new SelectTreeItem(rootData, 0, 0);
    setupModelData(widget, rootItem);
  }

  SelectTreeModel::~SelectTreeModel()
  {
    delete rootItem;
  }

  int SelectTreeModel::columnCount(const QModelIndex &parent) const
  {
    if (parent.isValid())
        return static_cast<SelectTreeItem*>(parent.internalPointer())->columnCount();
    else
        return rootItem->columnCount();
  }

  QVariant SelectTreeModel::data(const QModelIndex &index, int role) const
  {
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole)
        return QVariant();

    SelectTreeItem *item = static_cast<SelectTreeItem*>(index.internalPointer());

    return item->data(index.column());
  }

  Qt::ItemFlags SelectTreeModel::flags(const QModelIndex &index) const
  {
    if (!index.isValid())
      return 0;

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
  }

  QVariant SelectTreeModel::headerData(int section, Qt::Orientation orientation,
                               int role) const
  {
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
      return rootItem->data(section);

    return QVariant();
  }

  QModelIndex SelectTreeModel::index(int row, int column, const QModelIndex &parent) const
  {
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    SelectTreeItem *parentItem;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<SelectTreeItem*>(parent.internalPointer());

    SelectTreeItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
  }

  QModelIndex SelectTreeModel::parent(const QModelIndex &index) const
  {
    if (!index.isValid())
        return QModelIndex();

    SelectTreeItem *childItem = static_cast<SelectTreeItem*>(index.internalPointer());
    SelectTreeItem *parentItem = childItem->parent();

    if (parentItem == rootItem)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
  }

  int SelectTreeModel::rowCount(const QModelIndex &parent) const
  {
    SelectTreeItem *parentItem;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<SelectTreeItem*>(parent.internalPointer());

    return parentItem->childCount();
  }

  void SelectTreeModel::addChainData(Molecule *molecule, QList<SelectTreeItem*> &parents)
  {
/*    QList<QVariant> chainsData;
    chainsData << "Chains";
    parents.last()->appendChild(new SelectTreeItem(chainsData, SelectTreeItemType::HeaderType, parents.last()));
    if (parents.last()->childCount() > 0)
      parents << parents.last()->child(parents.last()->childCount()-1);
    /// FIXME - what was this trying to do?
    QList<char> chains;
    FOR_RESIDUES_OF_MOL (res, molecule) {
      if (!chains.contains(res->GetChain()))
        chains.append(res->GetChain());
    }
    for (int i = 0; i < chains.size(); ++i) {
      QList<QVariant> chainData;
      chainData << QString("chain %1").arg(chains[i]);
      chainData << chains[i];
      parents.last()->appendChild(new SelectTreeItem(chainData, SelectTreeItemType::ChainType, parents.last()));
      if (parents.last()->childCount() > 0)
        parents << parents.last()->child(parents.last()->childCount()-1);

      addResidueData(molecule, chains[i], parents);

      parents.pop_back(); // chains X
    }

    parents.pop_back(); // Chains
*/
  }

  void SelectTreeModel::addResidueData(Molecule *molecule, char chain, QList<SelectTreeItem*> &parents)
  {
    /// FIXME - wasn't sure what this was doing either
/*    QList<QVariant> residueData;
    residueData << "Residues";
    parents.last()->appendChild(new SelectTreeItem(residueData, SelectTreeItemType::HeaderType, parents.last()));
    if (parents.last()->childCount() > 0)
      parents << parents.last()->child(parents.last()->childCount()-1);
    FOR_RESIDUES_OF_MOL (res, molecule) {
      if ((chain != 0) && (res->GetChain() != chain))
        continue;

      QList<QVariant> columnData;
      columnData << res->GetName().c_str();
      columnData << res->GetIdx();
      parents.last()->appendChild(new SelectTreeItem(columnData, SelectTreeItemType::ResidueType, parents.last()));
      if (parents.last()->childCount() > 0)
        parents << parents.last()->child(parents.last()->childCount()-1);
      FOR_ATOMS_OF_RESIDUE (atom, &*res) {
        QList<QVariant> atomData;
        atomData << atom->GetType();
        atomData << atom->GetIdx();
        parents.last()->appendChild(new SelectTreeItem(atomData, SelectTreeItemType::AtomType, parents.last()));
      }
      parents.pop_back(); // resname idex
    }
    parents.pop_back(); // Residues */
  }

  void SelectTreeModel::addSelectionData(GLWidget *widget, QList<SelectTreeItem*> &parents)
  {
    QList<QVariant> selectData;
    selectData << "User Selections";
    parents.last()->appendChild(new SelectTreeItem(selectData, SelectTreeItemType::HeaderType, parents.last()));
    if (parents.last()->childCount() > 0)
      parents << parents.last()->child(parents.last()->childCount()-1);

    for (int i = 0; i < widget->namedSelections().size(); ++i) {
      QList<QVariant> chainData;
      chainData << widget->namedSelections().at(i);
      parents.last()->appendChild(new SelectTreeItem(chainData, SelectTreeItemType::SelectionType, parents.last()));
    }

    parents.pop_back(); // User selections
  }


  void SelectTreeModel::setupModelData(GLWidget *widget, SelectTreeItem *parent)
  {
    QList<SelectTreeItem*> parents;
    parents << parent;
    //QList<int> indentations;
    //indentations << 0;
    //int number = 0;

    Molecule *molecule = widget->molecule();

    addChainData(molecule, parents);
    addResidueData(molecule, 0, parents);
    addSelectionData(widget, parents);
/*
    QList<QVariant> residueData;
    residueData << "residues";
    parents.last()->appendChild(new SelectTreeItem(residueData, parents.last()));
    if (parents.last()->childCount() > 0)
      parents << parents.last()->child(parents.last()->childCount()-1);
    QList<Residue*> residues = molecule->residues();
    foreach (Residue *res, residues) {
      QList<QVariant> columnData;
      columnData << res->name();
      columnData << res->index();
      parents.last()->appendChild(new SelectTreeItem(columnData, parents.last()));
      if (parents.last()->childCount() > 0)
        parents << parents.last()->child(parents.last()->childCount()-1);
      QList<Atom*> atoms = res->atoms();
      foreach (Atom *atom, atoms) {
        QList<QVariant> atomData;
//        atomData << atom->GetType();
        atomData << atom->index();
        parents.last()->appendChild(new SelectTreeItem(atomData, parents.last()));
      }
      parents.pop_back();
    }
*/
    /*
    while (number < lines.count()) {
        int position = 0;
        while (position < lines[number].length()) {
            if (lines[number].mid(position, 1) != " ")
                break;
            position++;
        }

        QString lineData = lines[number].mid(position).trimmed();

        if (!lineData.isEmpty()) {
            // Read the column data from the rest of the line.
            QStringList columnStrings = lineData.split("\t", QString::SkipEmptyParts);
            QList<QVariant> columnData;
            for (int column = 0; column < columnStrings.count(); ++column)
                columnData << columnStrings[column];

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

            // Append a new item to the current parent's list of children.
            parents.last()->appendChild(new SelectTreeItem(columnData, parents.last()));
        }

        number++;
    }
    */
  }

  ////////////////////////////////////////////////////////
  //
  // NamedSelectionModel (list)
  //
  ////////////////////////////////////////////////////////

  int NamedSelectionModel::rowCount(const QModelIndex &parent) const
  {
    Q_UNUSED(parent);

    return m_widget->namedSelections().size();
  }

  QVariant NamedSelectionModel::data(const QModelIndex &index, int role) const
  {
    if (!index.isValid())
      return QVariant();

    if (role != Qt::DisplayRole)
      return QVariant();

    if (index.row() >= m_widget->namedSelections().size())
      return QVariant();

    return m_widget->namedSelections().at(index.row());
  }

  QVariant NamedSelectionModel::headerData(int /*section*/, Qt::Orientation /*orientation*/, int role) const
  {
    if (role != Qt::DisplayRole)
      return QVariant();

    return tr("Selections");
  }

  Qt::ItemFlags NamedSelectionModel::flags(const QModelIndex &index) const
  {
    return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
  }

  bool NamedSelectionModel::setData(const QModelIndex &index, const QVariant &value, int role)
  {
    if (!index.isValid())
      return false;

    if (role != Qt::EditRole)
      return false;

    m_widget->renameNamedSelection(index.row(), value.toString());

    return true;
  }

} // end namespace Avogadro

#include "namedselectionmodel.moc"
