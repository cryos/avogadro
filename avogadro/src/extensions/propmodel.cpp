/**********************************************************************
  propmodel.cpp - Models to hold properties

  Copyright (C) 2007 by Tim Vandermeersch

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

#include "propmodel.h"
#include <avogadro/primitive.h>
#include <avogadro/color.h>
#include <avogadro/glwidget.h>

#include <openbabel/forcefield.h>

#include <QtGui>
#include <QProgressDialog>
#include <QWriteLocker>
#include <QMutex>
#include <QMutexLocker>

using namespace std;
using namespace OpenBabel;

namespace Avogadro
{
  //////////////////////////////////////////////////////////////////////////////
  //
  // A T O M S 
  //
  //////////////////////////////////////////////////////////////////////////////
 
  int AtomPropModel::rowCount(const QModelIndex &parent) const
  {
    return m_molecule->NumAtoms();
  }

  int AtomPropModel::columnCount(const QModelIndex &parent) const
  {
    return 8;
  }

  QVariant AtomPropModel::data(const QModelIndex &index, int role) const
  {
    if (!index.isValid())
      return QVariant();

    if (index.row() >= m_molecule->NumAtoms())
      return QVariant();

    OpenBabel::OBAtom *atom = m_molecule->GetAtom(index.row() + 1);

    if (role == Qt::DisplayRole)
      switch (index.column()) {
        case 0: // type
          return atom->GetType();
        case 1: // atomic number
          return atom->GetAtomicNum();
        case 2: // isotope
          return atom->GetIsotope();
	case 3: // formal Charge
          return atom->GetFormalCharge();
        case 4: // partial charge
          return atom->GetPartialCharge();
        case 5: // valence
          return atom->GetValence();
        case 6: // BOSum
          return atom->BOSum();
        case 7: // chirality
	  if (atom->IsClockwise())
	    return QString("R");
          else if (atom->IsAntiClockwise())
	    return QString("S");
	  else
	    return QString("");
      }
    else
      return QVariant();
  }
  
  QVariant AtomPropModel::headerData(int section, Qt::Orientation orientation, int role) const
  {
    if (role != Qt::DisplayRole)
      return QVariant();

    if (orientation == Qt::Horizontal) {
      switch (section) {
        case 0:
	  return QString("Type");
        case 1:
	  return QString("Atomic Number");
        case 2:
	  return QString("Isotope");
	case 3:
	  return QString("Formal Charge");
        case 4:
	  return QString("Partial Charge");
        case 5:
	  return QString("Valence");
        case 6:
	  return QString("BOSum");
        case 7:
	  return QString("Chirality");
      }
    } else
      return QString("Atom %1").arg(section + 1);
  }
 
  Qt::ItemFlags AtomPropModel::flags(const QModelIndex &index) const
  {
    if (!index.isValid())
      return Qt::ItemIsEnabled;
    
    switch (index.column()) {
      case 0: // type
        return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
      case 1: // atomic number
        return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
      case 2: // isotope
        return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
      case 3: // formal Charge
        return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
      case 4: // partial charge
        return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
      case 5: // valence
      case 6: // BOSum
      case 7: // chirality
        return QAbstractItemModel::flags(index);
    }
 
    return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
  }

  bool AtomPropModel::setData(const QModelIndex &index, const QVariant &value, int role) 
  {
    if (index.isValid() && role == Qt::EditRole) {
      OpenBabel::OBAtom *atom = m_molecule->GetAtom(index.row() + 1);
      std::string buff;
      
      switch (index.column()) {
        case 0: // type
          buff = (value.toString()).toStdString();
	  atom->SetType(buff);
	  m_molecule->update();
	  emit dataChanged(index, index);
          return true;
        case 1: // atomic number
	  atom->SetAtomicNum(value.toInt());
	  m_molecule->update();
	  emit dataChanged(index, index);
          return true;
        case 2: // isotope
	  atom->SetIsotope(value.toInt());
	  m_molecule->update();
	  emit dataChanged(index, index);
          return true;
	case 3: // formal Charge
	  atom->SetFormalCharge(value.toInt());
	  m_molecule->update();
	  emit dataChanged(index, index);
          return true;
        case 4: // partial charge
	  atom->SetPartialCharge(value.toDouble());
	  m_molecule->update();
	  emit dataChanged(index, index);
          return true;
        case 5: // valence
          return false;
        case 6: // BOSum
          return false;
        case 7: // chirality
          return false;
      }
    }
    
    return false;
  }

  void AtomPropModel::setMolecule(Molecule *molecule)
  {
    m_molecule = molecule;
  }
  
  //////////////////////////////////////////////////////////////////////////////
  //
  // B O N D S 
  //
  //////////////////////////////////////////////////////////////////////////////
  
  int BondPropModel::rowCount(const QModelIndex &parent) const
  {
    return m_molecule->NumBonds();
  }

  int BondPropModel::columnCount(const QModelIndex &parent) const
  {
    return 5;
  }

  QVariant BondPropModel::data(const QModelIndex &index, int role) const
  {
    if (!index.isValid())
      return QVariant();

    if (index.row() >= m_molecule->NumBonds())
      return QVariant();

    //OpenBabel::OBBond *bond = m_molecule->GetAtom(index.row() + 1);
    OpenBabel::OBBond *bond = m_molecule->GetBond(index.row());

    if (role == Qt::DisplayRole)
      switch (index.column()) {
        case 0: // atom 1
          return bond->GetBeginAtomIdx();
        case 1: // atom 2
          return bond->GetEndAtomIdx();
        case 2: // order
          return bond->GetBondOrder();
	case 3: // length
          return bond->GetLength();
        case 4: // rotatable
          return bond->IsRotor();
      }
    else
      return QVariant();
  }
  
  QVariant BondPropModel::headerData(int section, Qt::Orientation orientation, int role) const
  {
    if (role != Qt::DisplayRole)
      return QVariant();

    if (orientation == Qt::Horizontal) {
      switch (section) {
        case 0:
	  return QString("Atom dx 1");
        case 1:
	  return QString("Atom idx 2");
        case 2:
	  return QString("Bond Order");
	case 3:
	  return QString("Length");
        case 4:
	  return QString("Rotatable");
      }
    } else
      return QString("Bond %1").arg(section);
  }
 
  Qt::ItemFlags BondPropModel::flags(const QModelIndex &index) const
  {
    if (!index.isValid())
      return Qt::ItemIsEnabled;
    
    switch (index.column()) {
      case 0: // atom 1
        return QAbstractItemModel::flags(index); 
      case 1: // atom 2
        return QAbstractItemModel::flags(index);
      case 2: // order
        return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
      case 3: // length
        return QAbstractItemModel::flags(index);
      case 4: // rotatable
        return QAbstractItemModel::flags(index);
    }
 
    return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
  }


  bool BondPropModel::setData(const QModelIndex &index, const QVariant &value, int role) 
  {
    if (index.isValid() && role == Qt::EditRole) {
      OpenBabel::OBBond *bond = m_molecule->GetBond(index.row());
      
      switch (index.column()) {
        case 0: // atom 1
        case 1: // atom 2
          return false;
        case 2: // order
	  bond->SetBondOrder(value.toInt());
	  m_molecule->update();
	  emit dataChanged(index, index);
          return false;
	case 3: // length
          return false;
        case 4: // rotatable
          return false;
      }
    }
    
    return false;
  }

  void BondPropModel::setMolecule(Molecule *molecule)
  {
    m_molecule = molecule;
  }
  
  //////////////////////////////////////////////////////////////////////////////
  //
  // C A R T E S I A N
  //
  //////////////////////////////////////////////////////////////////////////////
 
  int CartesianModel::rowCount(const QModelIndex &parent) const
  {
    return m_molecule->NumAtoms();
  }

  int CartesianModel::columnCount(const QModelIndex &parent) const
  {
    return 3;
  }

  QVariant CartesianModel::data(const QModelIndex &index, int role) const
  {
    if (!index.isValid())
      return QVariant();

    if (index.row() >= m_molecule->NumAtoms())
      return QVariant();

    OpenBabel::OBAtom *atom = m_molecule->GetAtom(index.row() + 1);

    if (role == Qt::DisplayRole)
      switch (index.column()) {
        case 0: 
          return atom->GetX();
        case 1: 
          return atom->GetY();
        case 2: 
          return atom->GetZ();
      }
    else
      return QVariant();
  }
  
  QVariant CartesianModel::headerData(int section, Qt::Orientation orientation, int role) const
  {
    if (role != Qt::DisplayRole)
      return QVariant();

    if (orientation == Qt::Horizontal) {
      switch (section) {
        case 0:
	  return QString("X");
        case 1:
	  return QString("Y");
        case 2:
	  return QString("Z");
      }
    } else
      return QString("Atom %1").arg(section + 1);
  }
 
  Qt::ItemFlags CartesianModel::flags(const QModelIndex &index) const
  {
    if (!index.isValid())
      return Qt::ItemIsEnabled;
    
    return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
  }

  bool CartesianModel::setData(const QModelIndex &index, const QVariant &value, int role) 
  {
    if (index.isValid() && role == Qt::EditRole) {
      OpenBabel::OBAtom *atom = m_molecule->GetAtom(index.row() + 1);
      OpenBabel::vector3 coord = atom->GetVector();
      
      switch (index.column()) {
        case 0: 
	  coord.SetX(value.toDouble());
	  atom->SetVector(coord);
	  m_molecule->update();
	  emit dataChanged(index, index);
          return true;
        case 1: 
	  coord.SetY(value.toDouble());
	  atom->SetVector(coord);
	  m_molecule->update();
	  emit dataChanged(index, index);
          return true;
        case 2: 
	  coord.SetZ(value.toDouble());
	  atom->SetVector(coord);
	  m_molecule->update();
	  emit dataChanged(index, index);
          return true;
      }
    }
    
    return false;
  }

  void CartesianModel::setMolecule(Molecule *molecule)
  {
    m_molecule = molecule;
  }
  
  //////////////////////////////////////////////////////////////////////////////
  //
  // C O N F O R M E R
  //
  //////////////////////////////////////////////////////////////////////////////
 
  int ConformerModel::rowCount(const QModelIndex &parent) const
  {
    return m_molecule->NumConformers();
  }

  int ConformerModel::columnCount(const QModelIndex &parent) const
  {
    return 1;
  }

  QVariant ConformerModel::data(const QModelIndex &index, int role) const
  {
    if (!index.isValid())
      return QVariant();

    if (index.row() >= m_molecule->NumConformers())
      return QVariant();

    OpenBabel::OBAtom *atom = m_molecule->GetAtom(index.row() + 1);

    if (role == Qt::DisplayRole)
      switch (index.column()) {
        case 0: // energy
	    return 0.0;
      }
    else
      return QVariant();
  }
  
  QVariant ConformerModel::headerData(int section, Qt::Orientation orientation, int role) const
  {
    if (role != Qt::DisplayRole)
      return QVariant();

    if (orientation == Qt::Horizontal) {
      switch (section) {
        case 0:
	  return QString("Energy");
      }
    } else
      return QString("Conformer %1").arg(section + 1);
  }
 
  void ConformerModel::setMolecule(Molecule *molecule)
  {
    m_molecule = molecule;
  }
  
 

} // end namespace Avogadro

#include "propmodel.moc"
