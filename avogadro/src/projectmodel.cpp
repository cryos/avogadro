/**********************************************************************
  ProjectModel - Project Model

  Copyright (C) 2006 by Geoffrey R. Hutchison
  Some portions Copyright (C) 2006 by Donald E. Curtis

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

#include "projectmodel.moc"

#include <avogadro/primitives.h>
#include <QDebug>

using namespace Avogadro;

  ProjectItem::ProjectItem(Primitive *primitive, QList<QVariant> data, ProjectItem *parent)
: parentItem(parent), userData(primitive), itemData(data)
{
  return;
}

  ProjectItem::ProjectItem(QList<QVariant> data, ProjectItem *parent) 
: userData(NULL), parentItem(parent), itemData(data)
{
  return;
}

ProjectItem::~ProjectItem()
{
  qDeleteAll(childItems);
}

void ProjectItem::appendChild(ProjectItem *child)
{
  qDebug() << child->data(0);
  childItems.append(child);
}

ProjectItem *ProjectItem::child(int row)
{
  return childItems.value(row);
}

int ProjectItem::childCount() const
{
  return childItems.size();
}

int ProjectItem::columnCount() const
{
  return itemData.size();
}

QVariant ProjectItem::data(int column) const
{
  return itemData.value(column);
}

int ProjectItem::row() const
{
  if(parentItem)
    return parentItem->childItems.indexOf(const_cast<ProjectItem*>(this));

  return 0;
}

ProjectItem *ProjectItem::parent()
{
  return parentItem;
}

ProjectModel::ProjectModel(Molecule *molecule, QObject *parent)
  : QAbstractItemModel(parent)
{
  QList<QVariant> rootData;
  rootData << "" << "" << "";
  rootItem = new ProjectItem(rootData);

  if(molecule)
  {
    QList<QVariant> moleculeData;
    moleculeData << "Molecule";
    ProjectItem * moleculeItem = new ProjectItem(molecule, moleculeData, rootItem);
    rootItem->appendChild(moleculeItem);

    QList<QVariant> atomsData;
    atomsData << "Atoms";
    ProjectItem * atomsItem = new ProjectItem(atomsData, moleculeItem);
    moleculeItem->appendChild(atomsItem);

    // add the atoms to the default queue
    std::vector<OpenBabel::OBNodeBase*>::iterator i;
    for(Atom *atom = (Atom*)molecule->BeginAtom(i); atom; atom = (Atom*)molecule->NextAtom(i))
    {
      QList<QVariant> atomData;
      atomData << atom->GetAtomicNum();
      ProjectItem *atomItem = new ProjectItem(atom, atomData, atomsItem);
      atomsItem->appendChild(atomItem);
    }

    QList<QVariant> bondsData;
    bondsData << "Bonds";
    ProjectItem * bondsItem = new ProjectItem(bondsData, moleculeItem);
    moleculeItem->appendChild(bondsItem);

    // add the bonds to the default queue
    std::vector<OpenBabel::OBEdgeBase*>::iterator j;
    for(Bond *bond = (Bond*)molecule->BeginBond(j); bond; bond = (Bond*)molecule->NextBond(j))
    {
      QList<QVariant> bondData;
      bondData << bond->GetBeginAtomIdx() << bond->GetEndAtomIdx();
      ProjectItem *bondItem = new ProjectItem(bond, bondData, bondsItem);
      bondsItem->appendChild(bondItem);
    }

    QList<QVariant> residuesData;
    residuesData << "Residues";
    ProjectItem * residuesItem = new ProjectItem(residuesData, moleculeItem);
    moleculeItem->appendChild(residuesItem);

    // add the residues to the default queue
    std::vector<OpenBabel::OBResidue*>::iterator k;
    for(Residue *residue = (Residue*)molecule->BeginResidue(k); residue;
        residue = (Residue *)molecule->NextResidue(k)) {
      QList<QVariant> residueData;
      residueData << residue->GetNumAtoms();
      ProjectItem *residueItem = new ProjectItem(residue, residueData, residuesItem);
      residuesItem->appendChild(residueItem);
    }
  } else {
    QList<QVariant> moleculeData;
    moleculeData << "No Molecule Loaded";
    ProjectItem * moleculeItem = new ProjectItem(molecule, moleculeData, rootItem);
    rootItem->appendChild(moleculeItem);
  }
}

QVariant ProjectModel::data(const QModelIndex &index, int role) const
{
  if(!index.isValid())
    return QVariant();

  if(role != Qt::DisplayRole)
    return QVariant();

  ProjectItem *item = static_cast<ProjectItem*>(index.internalPointer());

  return item->data(index.column());
}

Qt::ItemFlags ProjectModel::flags(const QModelIndex &index) const
{
  if (!index.isValid())
    return Qt::ItemIsEnabled;

  return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant ProjectModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
    return rootItem->data(section);

  return QVariant();
}

QModelIndex ProjectModel::index(int row, int column, const QModelIndex &parent) const
{
  ProjectItem *parentItem;

  if (!parent.isValid())
    parentItem = rootItem;
  else
    parentItem = static_cast<ProjectItem*>(parent.internalPointer());

  ProjectItem *childItem = parentItem->child(row);
  if (childItem)
    return createIndex(row, column, childItem);
  else
    return QModelIndex();
}

QModelIndex ProjectModel::parent(const QModelIndex &index) const
{
  if (!index.isValid())
    return QModelIndex();

  ProjectItem *childItem = static_cast<ProjectItem*>(index.internalPointer());
  ProjectItem *parentItem = childItem->parent();

  if (parentItem == rootItem)
    return QModelIndex();

  return createIndex(parentItem->row(), 0, parentItem);
}

int ProjectModel::rowCount(const QModelIndex &parent) const
{
  ProjectItem *parentItem;

  if (!parent.isValid())
    parentItem = rootItem;
  else
    parentItem = static_cast<ProjectItem*>(parent.internalPointer());

  return parentItem->childCount();
}

int ProjectModel::columnCount(const QModelIndex &parent) const
{
  if (parent.isValid())
    return static_cast<ProjectItem*>(parent.internalPointer())->columnCount();
  else
    return rootItem->columnCount();
}
