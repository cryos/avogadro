/**********************************************************************
  propmodel.cpp - Models to hold properties

  Copyright (C) 2007 by Tim Vandermeersch

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.openmolecules.net/>

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
#include <avogadro/molecule.h>
#include <avogadro/atom.h>
#include <avogadro/bond.h>

#include <openbabel/mol.h>

#include <QDebug>

namespace Avogadro {

  using std::vector;
  using std::pair;
  using OpenBabel::triple;
  using OpenBabel::OBMol;
  using OpenBabel::OBAngleData;
  using OpenBabel::OBTorsionData;
  using OpenBabel::OBTorsion;
  using OpenBabel::OBGenericDataType::AngleData;
  using OpenBabel::OBGenericDataType::TorsionData;
  using OpenBabel::OBAtom;

  PropertiesModel::PropertiesModel(Type type, QObject *parent)
    : QAbstractTableModel(parent), m_type(type), m_rowCount(0), m_molecule(0)
  {
  }

  int PropertiesModel::rowCount(const QModelIndex &parent) const
  {
    Q_UNUSED(parent);

    if (m_type == AtomType) {
      return m_molecule->numAtoms();
    }
    else if (m_type == BondType) {
      return m_molecule->numBonds();
    }
    else if (m_type == CartesianType) {
      return m_molecule->numAtoms();
    }
    else if (m_type == ConformerType) {
      return m_molecule->numConformers();
    }
    else if (m_type == AngleType) {
      OBMol obmol = m_molecule->OBMol();
      obmol.FindAngles();
      OBAngleData *ad = static_cast<OBAngleData *>(obmol.GetData(AngleData));
      return ad->GetSize();
    }
    else if (m_type == TorsionType) {
      OBMol obmol = m_molecule->OBMol();
      obmol.FindTorsions();
      OBTorsionData *td = static_cast<OBTorsionData *>(obmol.GetData(TorsionData));
      vector<OBTorsion> torsions = td->GetData();
      vector<triple<OBAtom*,OBAtom*,double> > torsionADs;
      vector<OBTorsion>::iterator i;

      int rowCount = 0;
      for (i = torsions.begin(); i != torsions.end(); ++i) {
        torsionADs = i->GetADs();
        rowCount += torsionADs.size();
      }
      return rowCount;
    }
    return 0;
  }

  int PropertiesModel::columnCount(const QModelIndex &parent) const
  {
    Q_UNUSED(parent);
    switch (m_type) {
    case AtomType:
      return 4;
    case BondType:
      return 5;
    case AngleType:
      return 4;
    case TorsionType:
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
      if (static_cast<unsigned int>(index.row()) >= m_molecule->numAtoms())
        return QVariant();

      Atom *atom = m_molecule->atom(index.row());

      switch (index.column()) {
      case 0: // type
        {
          OBMol obmol = m_molecule->OBMol();
          OpenBabel::OBAtom *obatom = obmol.GetAtom(index.row() + 1);
          return obatom->GetType();
        }
      case 1: // atomic number
        return atom->atomicNumber();
      case 2: // partial charge
        return atom->partialCharge();
      case 3: // valence
        return atom->valence();
      }
    }
    else if (m_type == BondType) {
      if (static_cast<unsigned int>(index.row()) >= m_molecule->numBonds())
        return QVariant();

      OBMol obmol = m_molecule->OBMol();
      OpenBabel::OBBond *bond = obmol.GetBond(index.row());

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
    }
    else if (m_type == AngleType) {
      OBMol obmol = m_molecule->OBMol();
      obmol.FindAngles();
      OBAngleData *ad = static_cast<OBAngleData *>(obmol.GetData(AngleData));
      vector<vector<unsigned int> > angles;
      ad->FillAngleArray(angles);

      if ((unsigned int) index.row() >= angles.size())
        return QVariant();

      switch (index.column()) {
      case 0:
      case 1:
      case 2:
        return (angles[index.row()][index.column()] + 1);
      case 3:
        return obmol.GetAngle(obmol.GetAtom(angles[index.row()][1] + 1),
                              obmol.GetAtom(angles[index.row()][0] + 1),
                              obmol.GetAtom(angles[index.row()][2] + 1));
      }
    }
    else if (m_type == TorsionType) {
      OBMol obmol = m_molecule->OBMol();
      obmol.FindTorsions();
      OBTorsionData *td = static_cast<OBTorsionData *>(obmol.GetData(TorsionData));
      vector<OBTorsion> torsions = td->GetData();
      pair<OBAtom*,OBAtom*> torsionBC;
      vector<triple<OBAtom*,OBAtom*,double> > torsionADs;
      vector<OBTorsion>::iterator i;
      vector<triple<OBAtom*,OBAtom*,double> >::iterator j;

      int rowCount = 0;
      for (i = torsions.begin(); i != torsions.end(); ++i) {
        torsionBC = i->GetBC();
        torsionADs = i->GetADs();
        for (j = torsionADs.begin(); j != torsionADs.end(); ++j) {
          if (rowCount == index.row()) {
            switch (index.column()) {
            case 0:
              return j->first->GetIdx();
            case 1:
              return torsionBC.first->GetIdx();
            case 2:
              return torsionBC.second->GetIdx();
            case 3:
              return j->second->GetIdx();
            case 4:
              return obmol.GetTorsion(j->first, torsionBC.first, torsionBC.second, j->second);
              //return j->third;
            }
          }
          rowCount++;
        }
      }
    } else if (m_type == CartesianType) {
      if (static_cast<unsigned int>(index.row()) >= m_molecule->numAtoms())
        return QVariant();

      Atom *atom = m_molecule->atom(index.row());

      switch (index.column()) {
      case 0:
        return QString::number(atom->pos()->x(), 'f', 5);
      case 1:
        return QString::number(atom->pos()->y(), 'f', 5);
      case 2:
        return QString::number(atom->pos()->z(), 'f', 5);
      }
    } else if (m_type == ConformerType) {
      if (static_cast<unsigned int>(index.row()) >= m_molecule->numConformers())
        return QVariant();

      switch (index.column()) {
      case 0: // energy
        if ((unsigned int) index.row() >= m_molecule->energies().size())
          return QVariant();

        return m_molecule->energies().at(index.row());
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
          return tr("Type");
        case 1:
          return tr("Atomic Number");
        case 2:
          return tr("Partial Charge");
        case 3:
          return tr("Valence");
        }
      } else
        return tr("Atom %1").arg(section + 1);
    } else if (m_type == BondType) {
      if (orientation == Qt::Horizontal) {
        switch (section) {
        case 0:
          return tr("Start Atom");
        case 1:
          return tr("End Atom");
        case 2:
          return tr("Bond Order");
        case 3:
          return tr("Length (Å)");
        case 4:
          return tr("Rotatable");
        }
      } else
        // Bond ordering starts at 0
        return tr("Bond %1").arg(section + 1);
    } else if (m_type == AngleType) {
      if (orientation == Qt::Horizontal) {
        switch (section) {
        case 0:
          return tr("Start Atom");
        case 1:
          return tr("Vertex");
        case 2:
          return tr("End Atom");
        case 3:
          return tr("Angle (°)");
        }
      } else
        return tr("Angle %1").arg(section + 1);
    } else if (m_type == TorsionType) {
      if (orientation == Qt::Horizontal) {
        switch (section) {
        case 0:
        case 1:
        case 2:
        case 3:
          return tr("Atom Index %1").arg(section +1);
        case 4:
          return tr("Torsion (°)");
        }
      } else
        return tr("Torsion %1").arg(section + 1);
    } else if (m_type == CartesianType) {
      if (orientation == Qt::Horizontal) {
        switch (section) {
        case 0:
          return tr("X (Å)");
        case 1:
          return tr("Y (Å)");
        case 2:
          return tr("Z (Å)");
        }
      } else
        return tr("Atom %1").arg(section + 1);
    } else if (m_type == ConformerType) {
      if (orientation == Qt::Horizontal) {
        switch (section) {
        case 0:
          return tr("Energy");
        }
      } else
        return tr("Conformer %1").arg(section + 1);
    }

    return QVariant();
  }

  Qt::ItemFlags PropertiesModel::flags(const QModelIndex &index) const
  {
    if (!index.isValid())
      return Qt::ItemIsEnabled;

    if (m_type == AtomType) {
      switch (index.column()) {
      case 1: // atomic number
      case 2: // partial charge
        return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
      case 0: // type
      case 3: // valence
        return QAbstractItemModel::flags(index);
      }
    }
    else if (m_type == BondType) {
      return QAbstractItemModel::flags(index);
    }
    else if (m_type == AngleType) {
      return QAbstractItemModel::flags(index);
    }
    else if (m_type == TorsionType) {
      return QAbstractItemModel::flags(index);
    }
    else if (m_type == CartesianType) {
      return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
    }
    else if (m_type == ConformerType) {
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
      Atom *atom = m_molecule->atom(index.row());

      switch (index.column()) {
      case 1: // atomic number
        atom->setAtomicNumber(value.toInt());
        m_molecule->update();
        emit dataChanged(index, index);
        return true;
      case 2: // partial charge
        atom->setPartialCharge(value.toDouble());
        m_molecule->update();
        emit dataChanged(index, index);
        return true;
      case 0: // type
      case 3: // valence
      default:
        return false;
      }
    }
    else if (m_type == BondType) {
      switch (index.column()) {
      case 4: // length
        return false;
        break;
      default:
        return false;
      }
    }
    else if (m_type == CartesianType) {
      if (index.column() > 2)
        return false;

      Atom *atom = m_molecule->atom(index.row());
      Eigen::Vector3d pos = *atom->pos();
      pos[index.column()] = value.toDouble();
      atom->setPos(pos);

      m_molecule->update();
      emit dataChanged(index, index);
      return true;
    }
    else if (m_type == AngleType) {
            switch (index.column()) {
      case 4: // angle
        return false;
        break;
      default:
        return false;
      }
    }
    else if (m_type == TorsionType) {
      switch (index.column()) {
      case 5: // dihedral angle
        return false;
        break;
      default:
        return false;
      }
    }
    return false;
  }

  void PropertiesModel::setMolecule(Molecule *molecule)
  {
    m_molecule = molecule;
  }

  void PropertiesModel::atomAdded(Atom *atom)
  {
    if ( (m_type == AtomType) || (m_type == CartesianType) ) {
      // insert a new row at the end
      beginInsertRows(QModelIndex(), atom->index(), atom->index());
      endInsertRows();
    }
  }

  void PropertiesModel::atomRemoved(Atom *atom)
  {
    if ( (m_type == AtomType) || (m_type == CartesianType) )  {
      // delete the row for this atom
      beginRemoveRows(QModelIndex(), atom->index(), atom->index());
      endRemoveRows();
    }
  }

  void PropertiesModel::bondAdded(Bond *bond)
  {
    if ( (m_type == BondType) ) {
      // insert a new row at the end
      beginInsertRows(QModelIndex(), bond->index(), bond->index());
      endInsertRows();
    }
  }

  void PropertiesModel::bondRemoved(Bond *bond)
  {
    if ( (m_type == BondType) )  {
      // delete the row for this atom
      beginRemoveRows(QModelIndex(), bond->index(), bond->index());
      endRemoveRows();
    }
  }

  void PropertiesModel::moleculeChanged()
  {
    // Tear down the model and build it back up again
    // FIXME I think this is pretty hackish - is there a better way to handle it?
    //  We cannot know how many rows have been added or removed, just that it
    // was a big number and the molecule changed significantly.
    int rows = rowCount();
    for (int i = 0; i < rows; ++i) {
      beginRemoveRows(QModelIndex(), 0, 0);
      endRemoveRows();
    }
    beginInsertRows(QModelIndex(), 0, rowCount()-1);
    endInsertRows();
  }

  /*
      if (primitive->type() == Primitive::BondType) { // when you delete an atom, its bond will be deleted too
        m_molecule->FindAngles();
        OBAngleData *ad = (OBAngleData *) m_molecule->GetData(OBGenericDataType::AngleData);
        while (ad->GetSize() != m_rowCount) {
          beginInsertRows(QModelIndex(), 0, 0);
          endInsertRows();
      m_rowCount++;
    }
    int numRows = ad->GetSize() - m_rowCount;
    qDebug() << "PropertiesModel::primitiveAdded()" << endl;
    qDebug() << "    ad->GetSize() = " << ad->GetSize() << endl;
    qDebug() << "    rowCount() = " << rowCount() << endl;
    qDebug() << "    m_rowCount = " << m_rowCount << endl;
    qDebug() << "    numRows = " << numRows << endl;
        updateTable();
        m_rowCount = ad->GetSize();
      }
      */
  /*
      if (primitive->type() == Primitive::BondType) { // only new bonds can create new angles
        m_molecule->FindAngles();
        OBAngleData *ad = (OBAngleData *) m_molecule->GetData(OBGenericDataType::AngleData);
        while (ad->GetSize() != m_rowCount) {
          beginRemoveRows(QModelIndex(), 0, 0);
          endRemoveRows();
      m_rowCount--;
    }
    int numRows = m_rowCount - ad->GetSize();
    qDebug() << "PropertiesModel::primitiveRemoved()" << endl;
    qDebug() << "    ad->GetSize() = " << ad->GetSize() << endl;
    qDebug() << "    rowCount() = " << rowCount() << endl;
    qDebug() << "    m_rowCount = " << m_rowCount << endl;
    qDebug() << "    numRows = " << numRows << endl;
        updateTable();
        m_rowCount = ad->GetSize();
      }
      */

  void PropertiesModel::updateTable()
  {
    emit dataChanged(QAbstractItemModel::createIndex(0, 0),
                     QAbstractItemModel::createIndex(rowCount(), columnCount()));
  }

} // end namespace Avogadro

#include "propmodel.moc"
