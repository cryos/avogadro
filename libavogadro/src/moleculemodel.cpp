/**********************************************************************
  MoleculeModel - Molecule Model

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

#include "moleculemodel.moc"

#include <QDebug>
#include <QApplication>

using namespace Avogadro;

MoleculeModel::MoleculeModel(Molecule *molecule, QObject *parent)
  : QAbstractItemModel(parent)
{
  rootItem = new Primitive(Primitive::LastType);

  moleculeItem = molecule;
}

QVariant MoleculeModel::data(const QModelIndex &index, int role) const
{
  QVariant data;

  if(!index.isValid())
    return QVariant();

  if(role != Qt::DisplayRole)
    return QVariant();

  Primitive *item = static_cast<Primitive*>(index.internalPointer());
  QDataStream strstr;
  QString str;
  enum Primitive::Type type = item->getType();
  if(type == Primitive::MoleculeType)
  {
    data = "Molecule";
  }
  else if(type == Primitive::AtomType)
  {
    Atom *atom = (Atom*)item;
    str = tr("Atom ") + QString::number(atom->GetIdx());
    //strstr << "Atom " << atom->GetIdx() << " (" << atom->GetAtomicNum() << ")";
    data = str;
  }
  else if(type == Primitive::BondType)
  {
    Bond *bond = (Bond*)item;
    str = tr("Bond ") + QString::number(bond->GetIdx()) + tr(" (") + 
      QString::number(bond->GetBeginAtomIdx()) + tr(",") 
      + QString::number(bond->GetEndAtomIdx()) + tr(")");
    //strstr << "Bond " << bond->GetIdx() << " (" << bond->GetBeginAtomIdx() << "," << bond->GetEndAtomIdx() << ")";
    data = str;
  }
  else if(type == Primitive::ResidueType)
  {
    Residue *residue = (Residue*)item;
    str = tr("Residue ") + QString::number(residue->GetIdx());
    //strstr << "Residue " << residue->GetIdx();
    data = str;
  }
  else
  {
    data = "Unknown";
  }

  return data;
}

Qt::ItemFlags MoleculeModel::flags(const QModelIndex &index) const
{
  if (!index.isValid())
    return Qt::ItemIsEnabled;

  return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant MoleculeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
    return QVariant("Primitives List");

  return QVariant();
}

QModelIndex MoleculeModel::index(int row, int column, const QModelIndex &parent) const
{
  Primitive *parentItem;

  if (!parent.isValid())
    parentItem = rootItem;
  else
    parentItem = static_cast<Primitive*>(parent.internalPointer());

  // root item
  Primitive *child = NULL;
  if (parentItem == rootItem)
  {
    if(row == 0)
    {
      child = moleculeItem;
    }
  } 
  else if (parentItem == moleculeItem)
  {
    child = getMoleculeRow(row);
    if(!child)
    {
      qWarning("%d:Error Getting Row Information",row);
      qApp->exit(8);
      exit(8);
    }
  }


  if (child)
    return createIndex(row, column, child);
  else
    return QModelIndex();
}

Primitive *MoleculeModel::getMoleculeRow(int row) const
{
  int natoms = moleculeItem->NumAtoms();
  int nbonds = moleculeItem->NumBonds();
  int nresidues = moleculeItem->NumResidues();

  if(row < 0)
    return NULL;
  // atoms start at 1
  else if(row < natoms)
    return (Atom *)moleculeItem->GetAtom(row+1);
  // confusing but bonds (and i believe residues) start at 0
  else if(row < natoms + nbonds)
    return (Bond *)moleculeItem->GetBond(row-natoms);
  else if(row < natoms + nbonds + nresidues)
    return (Residue *)moleculeItem->GetResidue(row-natoms-nbonds);

  return NULL;
}

QModelIndex MoleculeModel::parent(const QModelIndex &index) const
{
  int row=0;
  if (!index.isValid())
    return QModelIndex();

  Primitive *childItem = static_cast<Primitive*>(index.internalPointer());
  Primitive *parentItem = NULL;
  switch(childItem->getType()) {
    case Primitive::LastType:
    case Primitive::MoleculeType:
      return QModelIndex();
      break;
    default:
      row = 0;
      parentItem = moleculeItem;
      break;
  }

  return createIndex(row, 0, parentItem);
}

int MoleculeModel::rowCount(const QModelIndex &parent) const
{
  Primitive *parentItem;

  if (!parent.isValid())
  {
    return 1;
  }
  else
  {
    parentItem = static_cast<Primitive*>(parent.internalPointer());
  }

  if(parentItem == moleculeItem)
  {
    return(moleculeItem->NumAtoms() + moleculeItem->NumBonds());
    return(moleculeItem->NumAtoms() + moleculeItem->NumBonds() + moleculeItem->NumResidues());
  }

  return 0;
}

int MoleculeModel::columnCount(const QModelIndex &parent) const
{
  return 1;
}
