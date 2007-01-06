/**********************************************************************
  Primitives - Wrapper class around the OpenBabel classes

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

#include "primitives.moc"

#include <QApplication>

using namespace Avogadro;

Molecule::Molecule(QObject *parent) 
  : QAbstractItemModel(NULL), OpenBabel::OBMol(), Primitive(MoleculeType) 
{
  // hack against qt const functions
  _self = this;
}

Atom * Molecule::CreateAtom()
{
  Atom *atom = new Atom();
  return(atom);
}

Bond * Molecule::CreateBond()
{
  Bond *bond = new Bond();
  return(bond);
}

Residue * Molecule::CreateResidue()
{
  Residue *residue = new Residue();
  return(residue);
}

Atom * Molecule::NewAtom()
{
  Atom *atom = (Atom *) OBMol::NewAtom();
  emit atomAdded(atom);
  return(atom);
}

Bond * Molecule::NewBond()
{
  Bond *bond = (Bond *) OBMol::NewBond();
  emit bondAdded(bond);
  return(bond);
}

Residue * Molecule::NewResidue()
{
  Residue *residue = (Residue *) OBMol::NewResidue();
 emit residueAdded(residue);
  return(residue);
}

QVariant Molecule::data(const QModelIndex &index, int role) const
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

Qt::ItemFlags Molecule::flags(const QModelIndex &index)
{
  if (!index.isValid())
    return Qt::ItemIsEnabled;

  return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant Molecule::headerData(int section, Qt::Orientation orientation, int role)
{
  if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
    return QVariant("Primitives List");

  return QVariant();
}

QModelIndex Molecule::index(int row, int column, const QModelIndex &parent) const
{
  Primitive *parentItem;
  Primitive *child = NULL;

  // if no parent assume parent is root and give mol as first node
  if (!parent.isValid())
  {
    if(row == 0)
    {
      child = _self;
    }
  }
  else
  {
    parentItem = static_cast<Primitive*>(parent.internalPointer());
    if (parentItem == _self)
    {
      child = getMoleculeRow(row);
      if(!child)
      {
        qWarning("%d:Error Getting Row Information",row);
        qApp->exit(8);
        exit(8);
      }
    }
  }

  if (child)
    return createIndex(row, column, child);
  else
    return QModelIndex();
}

Primitive *Molecule::getMoleculeRow(int row) const
{
  int natoms = NumAtoms();
  int nbonds = NumBonds();
  int nresidues = NumResidues();

  if(row < 0)
    return NULL;
  // atoms start at 1
  else if(row < natoms)
    return (Atom *)_self->GetAtom(row+1);
  // confusing but bonds (and i believe residues) start at 0
  else if(row < natoms + nbonds)
    return (Bond *)_self->GetBond(row-natoms);
  else if(row < natoms + nbonds + nresidues)
    return (Residue *)_self->GetResidue(row-natoms-nbonds);

  return NULL;
}

QModelIndex Molecule::parent(const QModelIndex &index) const
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
      parentItem = _self;
      break;
  }

  return createIndex(row, 0, parentItem);
}

int Molecule::rowCount(const QModelIndex &parent) const
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

  if(parentItem == _self)
  {
    return(NumAtoms() + NumBonds());
    return(NumAtoms() + NumBonds() + NumResidues());
  }

  return 0;
}

int Molecule::columnCount(const QModelIndex &parent) const
{
  return 1;
}
