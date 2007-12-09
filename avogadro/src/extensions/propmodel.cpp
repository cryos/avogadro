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

#include <openbabel/mol.h>

#include <QtGui>
#include <QProgressDialog>
#include <QWriteLocker>
#include <QMutex>
#include <QMutexLocker>

using namespace std;
using namespace OpenBabel;

namespace Avogadro
{
  int PropertiesModel::rowCount(const QModelIndex &parent) const
  {
    Q_UNUSED(parent);
    switch (m_type) {
      case AtomType:
      case CartesianType:
        return m_molecule->NumAtoms();
      case BondType:
        return m_molecule->NumBonds();
      case ConformerType:
        return m_molecule->NumConformers();
    }

    return 0;
  }

  int PropertiesModel::columnCount(const QModelIndex &parent) const
  {
    Q_UNUSED(parent);
    switch (m_type) {
      case AtomType:
        return 8;
      case BondType:
        return 5;
      case CartesianType:
        return 3;
      case ConformerType:
        return 1;
    }

    return 0;
  }

  QVariant PropertiesModel::data(const QModelIndex &index, int role) const
  {
    if (!index.isValid())
      return QVariant();

    if (role != Qt::DisplayRole)
      return QVariant();
    
    if (m_type == AtomType) {
      if (static_cast<unsigned int>(index.row()) >= m_molecule->NumAtoms())
        return QVariant();

      OpenBabel::OBAtom *atom = m_molecule->GetAtom(index.row() + 1);

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
    } else if (m_type == BondType) {
      if (static_cast<unsigned int>(index.row()) >= m_molecule->NumBonds())
        return QVariant();

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
    } else if (m_type == CartesianType) {
      if (static_cast<unsigned int>(index.row()) >= m_molecule->NumAtoms())
        return QVariant();

      OpenBabel::OBAtom *atom = m_molecule->GetAtom(index.row() + 1);

      switch (index.column()) {
        case 0: 
          return atom->GetX();
        case 1: 
          return atom->GetY();
        case 2: 
          return atom->GetZ();
      }
    } else if (m_type == ConformerType) {
      if (index.row() >= m_molecule->NumConformers())
        return QVariant();

      switch (index.column()) {
        case 0: // energy
	    return m_molecule->GetEnergy();
      }
    } 
  
    return QVariant();
  }

  QVariant PropertiesModel::headerData(int section, Qt::Orientation orientation, int role) const
  {
    if (role != Qt::DisplayRole)
      return QVariant();

    if (m_type == AtomType) {
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
    } else if (m_type == BondType) {
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
        // Bond ordering starts at 0
        return QString("Bond %1").arg(section);
    } else if (m_type == CartesianType) {
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
    } else if (m_type == ConformerType) {
      if (orientation == Qt::Horizontal) {
        switch (section) {
          case 0:
	    return QString("Energy");
        }
      } else
        return QString("Conformer %1").arg(section + 1);
    }
    
    return QVariant();
  }
 
  Qt::ItemFlags PropertiesModel::flags(const QModelIndex &index) const
  {
    if (!index.isValid())
      return Qt::ItemIsEnabled;
    
    if (m_type == AtomType) {
      switch (index.column()) {
        case 0: // type
        case 1: // atomic number
        case 2: // isotope
        case 3: // formal Charge
        case 4: // partial charge
          return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
        case 5: // valence
        case 6: // BOSum
        case 7: // chirality
          return QAbstractItemModel::flags(index);
      }
    } else if (m_type == BondType) {
      switch (index.column()) {
        case 0: // atom 1
        case 1: // atom 2
        case 2: // order
        case 3: // length
        case 4: // rotatable
          return QAbstractItemModel::flags(index);
      }
    } else if (m_type == CartesianType) {
      return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
    } else if (m_type == ConformerType) {
      return QAbstractItemModel::flags(index);
    }
    
    
    return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
  }

  bool PropertiesModel::setData(const QModelIndex &index, const QVariant &value, int role) 
  {
    if (!index.isValid())
      return false; 
      
    if (role != Qt::EditRole)
      return false;
      
    if (m_type == AtomType) {
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
    } else if (m_type == BondType) {
      // OpenBabel::OBBond *bond = m_molecule->GetBond(index.row());
      
      switch (index.column()) {
        case 0: // atom 1
        case 1: // atom 2
        case 2: // order
	case 3: // length
        case 4: // rotatable
          return false;
      }
    } else if (m_type == CartesianType) {
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

  void PropertiesModel::setMolecule(Molecule *molecule)
  {
    m_molecule = molecule;
  }
  
  void PropertiesModel::updateTable()
  {
    QModelIndex idx = createIndex(0, 0);
    QModelIndex idx2;
    
    switch (m_type) {
      case AtomType:
        idx2 = createIndex(m_molecule->NumAtoms(), 7);
      case CartesianType:
        idx2 = createIndex(m_molecule->NumAtoms(), 2);
      case BondType:
        idx2 = createIndex(m_molecule->NumBonds(), 4);
      case ConformerType:
        idx2 = createIndex(m_molecule->NumConformers(), 0);
    }
 
    emit dataChanged(idx, idx2);
  }
  
  void PropertiesModel::primitiveAdded(Primitive *primitive)
  {
    if ( (primitive->type() == Primitive::AtomType) && ( (m_type == AtomType) || (m_type == CartesianType) ) ) {
      beginInsertRows(QModelIndex(), 0, 0);
      endInsertRows();
    } else if ( (primitive->type() == Primitive::BondType) && (m_type == BondType) ) {
      beginInsertRows(QModelIndex(), 0, 0);
      endInsertRows();
    }
    
    updateTable();
  }
  
  void PropertiesModel::primitiveRemoved(Primitive *primitive)
  {
    if ( (primitive->type() == Primitive::AtomType) && ( (m_type == AtomType) || (m_type == CartesianType) ) ) {
      beginRemoveRows(QModelIndex(), 0, 0);
      endRemoveRows();
    } else if ( (primitive->type() == Primitive::BondType) && (m_type == BondType) ) {
      beginRemoveRows(QModelIndex(), 0, 0);
      endRemoveRows();
    }
    
    updateTable();
  }

} // end namespace Avogadro

#include "propmodel.moc"
