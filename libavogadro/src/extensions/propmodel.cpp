/**********************************************************************
  propmodel.cpp - Models to hold properties

  Copyright (C) 2007 by Tim Vandermeersch
  Copyright (C) 2009 by Konstantin Tokarev 

  propmodel.cpp - Models to hold properties

  Copyright (C) 2007 by Tim Vandermeersch
  Copyright (C) 2009 by Konstantin Tokarev

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
// for recursively setting bond lengths, angles, etc.
#include "../tools/skeletontree.h"

#include <avogadro/molecule.h>
#include <avogadro/atom.h>
#include <avogadro/bond.h>

#include <openbabel/mol.h>
#include <Eigen/Geometry>

#include <limits>

#include <QDebug>

namespace Avogadro {

  using std::numeric_limits;
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

  QString groupIndexString (Atom *a)
  {
    unsigned int gi = a->groupIndex();
    if (gi != 0) {
      return QString(OpenBabel::etab.GetSymbol(a->atomicNumber())) + QString("%L1").arg(gi);
    } else {
      return QString(OpenBabel::etab.GetSymbol(a->atomicNumber()));
    }
  }

  inline QString bondTypeString (Atom *a, Atom *b, int order)
  {
    Q_UNUSED(order)
    /*QString bond;
    if (order == 2) {
      bond = "=";
    } else if (order == 3) {
      bond = "\x2261"; //2261";
    } else {
      bond = "-";
    }*/
    return QString(OpenBabel::etab.GetSymbol(a->atomicNumber())) + '-' +
      QString(OpenBabel::etab.GetSymbol(b->atomicNumber()));
  }

  inline QString angleTypeString (Atom *a, Atom *b, Atom *c)
  {
    return QString(OpenBabel::etab.GetSymbol(a->atomicNumber())) +
      QString(OpenBabel::etab.GetSymbol(b->atomicNumber())) +
      QString(OpenBabel::etab.GetSymbol(c->atomicNumber()));
  }

  inline QString angleTypeString (Atom *a, Atom *b, Atom *c, Atom *d)
  {
    return QString(OpenBabel::etab.GetSymbol(a->atomicNumber())) + 
      QString(OpenBabel::etab.GetSymbol(b->atomicNumber())) +
      QString(OpenBabel::etab.GetSymbol(c->atomicNumber())) +
      QString(OpenBabel::etab.GetSymbol(d->atomicNumber()));
  }

  PropertiesModel::PropertiesModel(Type type, QObject *parent)
    : QAbstractTableModel(parent), m_type(type), m_rowCount(0), m_molecule(0),
      m_displayConformers(false), m_validCache(false)
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
    /*else if (m_type == CartesianType) {
      return m_molecule->numAtoms();
    }*/
    else if (m_type == ConformerType) {
      return numConformers();
    }
    else if (m_type == AngleType) {
      if (!m_validCache)
        updateCache();
      return m_angleData.size();
    }
    else if (m_type == TorsionType) {
      if (!m_validCache)
        updateCache();
      return m_torsionData.size();
    }
    return 0;
  }

  int PropertiesModel::columnCount(const QModelIndex &parent) const
  {
    Q_UNUSED(parent);
    switch (m_type) {
    case AtomType:
      return 5 + (numConformers()*3); // element, type, valence, formal charge, partial charge, [ x, y, z ] * numConformers
    case BondType:
      return 6 + numConformers() - 1;
    case AngleType:
      return 5 + numConformers() - 1;
    case TorsionType:
      return 6 + numConformers() - 1;
    /*case CartesianType:
      return 3;*/
    case ConformerType:
      return 1;
    }
    return 0;
  }

  QVariant PropertiesModel::data(const QModelIndex &index, int role) const
  {

    if (!index.isValid())
      return QVariant();

    // handle text alignments
    if (role == Qt::TextAlignmentRole) {
      /*if (m_type == CartesianType) {
        return Qt::AlignRight + Qt::AlignVCenter; // XYZ coordinates
      }
      else*/ if (m_type == ConformerType) {
        return Qt::AlignRight + Qt::AlignVCenter; // energies
      }
      else if (m_type == AtomType) {
        if (index.column() == 3)
          return Qt::AlignRight + Qt::AlignVCenter; // partial charge
        else
          return Qt::AlignHCenter + Qt::AlignVCenter;
      }
      else if (m_type == BondType) {
        if (index.column() >= 5)
          return Qt::AlignRight + Qt::AlignVCenter; // bond length
        else
          return Qt::AlignHCenter + Qt::AlignVCenter;
      }
      else if (m_type == AngleType) {
        if (index.column() >= 4)
          return Qt::AlignRight + Qt::AlignVCenter; // angle
        else
          return Qt::AlignHCenter + Qt::AlignVCenter;
      }
      else if (m_type == TorsionType) {
        if (index.column() >= 5)
          return Qt::AlignRight + Qt::AlignVCenter; // dihedral angle
        else
          return Qt::AlignHCenter + Qt::AlignVCenter;
      }
    }

    if (role != Qt::UserRole && role != Qt::DisplayRole)
      return QVariant();

    bool sortRole = (role == Qt::UserRole); // from the proxy model to handle floating-point

    if (m_type == AtomType) {
      if (static_cast<unsigned int>(index.row()) >= m_molecule->numAtoms() ||
          static_cast<unsigned int>(index.column()) > 5 + (3*numConformers()) )
        return QVariant();

      if (!m_validCache)
        updateCache();

      AtomColumn column=static_cast<AtomColumn>(index.column());
      QString format("%L1");

      // Return Data
      switch (column) {
      case AtomDataElement:
        return m_atomData.at(index.row()).at(0).toString();
      case AtomDataType:
        return m_atomData.at(index.row()).at(1);
      case AtomDataValence:
        return m_atomData.at(index.row()).at(2);
      case AtomDataFormalCharge:
        return m_atomData.at(index.row()).at(3);
      case AtomDataPartialCharge:
        if (sortRole)
          return m_atomData.at(index.row()).at(4);
        else
          return format.arg(m_atomData.at(index.row()).at(4).toDouble(), 0, 'f', 3);
      default:
	// Remainder determines if x,y or z
	unsigned int remainder=(index.column()-5) % 3;
	switch (remainder)
	  {
	    // x-coordinate
	  case 0:
	    if (sortRole)
	      return m_atomCoords.at( conformerFromIndex( index ) ).at(index.row()).x();
	    else
	      return format.arg(m_atomCoords.at( conformerFromIndex( index ) ).at(index.row()).x(), 0, 'f', 5);
	    // y-coordinate
	  case 1:
	    if (sortRole)
	      return m_atomCoords.at( conformerFromIndex( index ) ).at(index.row()).y();
	    else
	      return format.arg(m_atomCoords.at( conformerFromIndex( index ) ).at(index.row()).y(), 0, 'f', 5);
	    // z-coordinate
	  case 2:
	    if (sortRole)
	      return m_atomCoords.at( conformerFromIndex( index ) ).at(index.row()).z();
	    else
	      return format.arg(m_atomCoords.at( conformerFromIndex( index ) ).at(index.row()).z(), 0, 'f', 5);
	  default:
	    std::cerr << "ERROR IN COORDS!\n";
	    return QVariant();
	  } // end coordinate switch
      }
    }
    else if (m_type == BondType) {

      if (static_cast<unsigned int>(index.row()) >= m_molecule->numBonds() ||
          static_cast<unsigned int>(index.column()) > 5 + (3*numConformers()) )
        return QVariant();

      if (!m_validCache)
        updateCache();

      BondColumn column=static_cast<BondColumn>(index.column());
      switch (column) {
      case BondDataType:
        return m_bondData.at(index.row()).at(0).toString();
      case BondDataAtom1:
        return m_bondData.at(index.row()).at(1).toString();
      case BondDataAtom2:
        return m_bondData.at(index.row()).at(2).toString();
      case BondDataOrder: // unsigned int
        return m_bondData.at(index.row()).at(3);
      case BondDataRotatable:
        if (sortRole)
          return m_bondLengths.at( m_molecule->currentConformer() ).at( index.row() );
        else
          return m_bondData.at(index.row()).at(4).toString();
      default: // length
        return m_bondLengths.at( conformerFromIndex( index ) ).at(index.row());
      }
    }
    else if (m_type == AngleType) {

      if (!m_validCache)
        updateCache();

      if ((unsigned int) index.row() >= m_angleData.size() )
        return QVariant();

      AngleColumn column=static_cast<AngleColumn>(index.column());
      switch (column) {
      case AngleDataType:
	return m_angleData.at(index.row()).at(0).toString();
      case AngleDataStartAtom:
        return m_angleData.at(index.row()).at(1);
      case AngleDataVertex:
        return m_angleData.at(index.row()).at(2);
      case AngleDataEndAtom:
        return m_angleData.at(index.row()).at(3);
      default:
        QString format("%L1");
        double angle =  m_angleValues.at( conformerFromIndex( index ) ).at(index.row());
        if (sortRole)
          return angle;
        else
          return format.arg(angle, 0, 'f', 4);
      }
    }
    else if (m_type == TorsionType) {

      if (!m_validCache)
        updateCache();

      TorsionColumn column=static_cast<TorsionColumn>(index.column());
      switch (column) {
      case TorsionDataType:
	return m_torsionData.at(index.row()).at(0).toString();
      case TorsionDataAtom1:
        return m_torsionData.at(index.row()).at(1);
      case TorsionDataAtom2:
        return m_torsionData.at(index.row()).at(2);
      case TorsionDataAtom3:
        return m_torsionData.at(index.row()).at(3);
      case TorsionDataAtom4:
        return m_torsionData.at(index.row()).at(4);
      default:
        QString format("%L1");
        double torsion =  m_torsionValues.at( conformerFromIndex( index ) ).at(index.row());
        if (sortRole)
          return torsion;
        else
          return format.arg(torsion, 0, 'f', 4);
      }


    } /*else if (m_type == CartesianType) {
      if (static_cast<unsigned int>(index.row()) >= m_molecule->numAtoms())
        return QVariant();

      Atom *atom = m_molecule->atom(index.row());
      QString format("%L1");

      switch (index.column()) {
      case 0:
        if (sortRole)
          return atom->pos()->x();
        else
          return format.arg(atom->pos()->x(), 0, 'f', 5);
      case 1:
        if (sortRole)
          return atom->pos()->y();
        else
          return format.arg(atom->pos()->y(), 0, 'f', 5);
      case 2:
        if (sortRole)
          return atom->pos()->z();
        else
          return format.arg(atom->pos()->z(), 0, 'f', 5);
      }
    }*/ else if (m_type == ConformerType) {
      if (static_cast<unsigned int>(index.row()) >= numConformers())
        return QVariant();

      switch (index.column()) {
      case 0: // energy
        if ((unsigned int) index.row() >= m_molecule->energies().size())
          return QVariant();

        QString format("%L1");
        if (sortRole)
          return m_molecule->energies().at(index.row());
        else
          return format.arg(m_molecule->energies().at(index.row()), 0, 'f', 4);
      }
    }

    return QVariant();
  }

  QVariant PropertiesModel::headerData(int section, Qt::Orientation orientation, int role) const
  {
    // handle text alignments
    if (role == Qt::TextAlignmentRole) {
      if (orientation == Qt::Vertical) {
        return Qt::AlignHCenter; // XYZ coordinates
      }
    }

    if (role != Qt::DisplayRole)
      return QVariant();

    if (m_type == AtomType) {
      if (orientation == Qt::Horizontal) {

        unsigned int column = static_cast<AtomColumn>(section);
        switch (column ) {
        case AtomDataElement:
          return tr("Element");
        case AtomDataType:
          return tr("Type");
        case AtomDataValence:
          return tr("Valence");
        case AtomDataFormalCharge:
          return QString(tr("Formal Charge")).replace(" ","\n");
        case AtomDataPartialCharge:
          return QString(tr("Partial Charge")).replace(" ","\n");
        default:
          // 5 and above
          unsigned int myConformer=(column-5) / 3;
          unsigned int remainder=(column-5) % 3;

          QString CoordHeader;
          switch(remainder)
            {
            case 0:
              CoordHeader = QString("X %1").arg("(\xC5)");
              break;
            case 1:
              CoordHeader = QString("Y %1").arg("(\xC5)");
              break;
            case 2:
              CoordHeader = QString("Z %1").arg("(\xC5)");
              break;
	    default:
	      std::cerr << "AtomType remainder error in headerData: " << remainder << std::endl;
            }

          if( numConformers() > 1 )
            CoordHeader.prepend( QString("Conformer %1\n").arg( myConformer+1 ) );

          // Seems somewhat daft to have to convert a QString to *char to pass
          // it to Qt's tr function - am I missing something?
          return trUtf8(CoordHeader.toUtf8().data(), "in Angstrom");

        }
      } else
        return tr("Atom") + QString(" %1").arg(section + 1);
    } else if (m_type == BondType) {
      if (orientation == Qt::Horizontal) {
        unsigned int column = static_cast<BondColumn>(section);
        switch ( column ) {
        case BondDataType:
          return tr("Type");
        case BondDataAtom1:
          return tr("Start Atom");
        case BondDataAtom2:
          return tr("End Atom");
        case BondDataOrder:
          return tr("Bond Order");
        case BondDataRotatable:
          return tr("Rotatable");
        default: // A bond length
	  if( numConformers() > 1 )
	    return tr("Conformer %1\nLength %2", "in Angstrom").arg(column-4).arg("(\xC5)");
	  else
	    return tr("Length %1", "in Angstrom").arg("(\xC5)");
        }
      } else
        // Bond ordering starts at 0
        return tr("Bond") + QString(" %1").arg(section + 1);
    } else if (m_type == AngleType) {
      if (orientation == Qt::Horizontal) {
	unsigned int column= static_cast<AngleColumn>(section);
        switch (column) {
        case AngleDataType:
          return tr("Type");
        case AngleDataStartAtom:
          return tr("Start Atom");
        case AngleDataVertex:
          return tr("Vertex");
        case AngleDataEndAtom:
          return tr("End Atom");
	default:
	  if( numConformers() > 1 )
	    return tr("Conformer %1\nAngle %2", "Degree symbol").arg(column-3).arg("(\xB0)");
	  else
	    return tr("Angle %1", "Degree symbol").arg("(\xB0)");
        }
      } else
        return tr("Angle") + QString(" %1").arg(section + 1);
    } else if (m_type == TorsionType) {
      if (orientation == Qt::Horizontal) {
	unsigned int column= static_cast<TorsionColumn>(section);
        switch (section) {
        case 0:
          return tr("Type");
        case TorsionDataAtom1:
        case TorsionDataAtom2:
        case TorsionDataAtom3:
        case TorsionDataAtom4:
          return tr("Atom %1").arg(column);
	default:
	  if( numConformers() > 1 )
	    return trUtf8("Conformer %1\nTorsion %2", "Degree symbol").arg(column-4).arg("(\xB0)");
	  else
	    return trUtf8("Torsion %1", "Degree symbol").arg("(\xB0)");

        }
      } else
        return tr("Torsion") + QString(" %1").arg(section + 1);
    } /*else if (m_type == CartesianType) {
      if (orientation == Qt::Horizontal) {
        switch (section) {
        case 0:
          return trUtf8("X %1", "in Angstrom").arg("(\xC5)");
        case 1:
          return trUtf8("Y %1", "in Angstrom").arg("(\xC5)");
        case 2:
          return trUtf8("Z  %1", "in Angstrom").arg("(\xC5)");
        }
      } else
        return tr("Atom %1").arg(section + 1);
    }*/ else if (m_type == ConformerType) {
      if (orientation == Qt::Horizontal) {
        switch (section) {
        case 0:
          return tr("Energy");
        }
      } else
        return tr("Conformer") + QString(" %1").arg(section + 1);
    }

    return QVariant();
  }

  Qt::ItemFlags PropertiesModel::flags(const QModelIndex &index) const
  {
    if (!index.isValid())
      return Qt::ItemIsEnabled;

    if (m_type == AtomType) {
      switch ( static_cast<AtomColumn>(index.column()) ) {
      case AtomDataType:
      case AtomDataValence:
        return QAbstractItemModel::flags(index);
      case AtomDataElement:
      case AtomDataFormalCharge:
      case AtomDataPartialCharge:
      default: // Covers any coordinates
        return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
      }
    }
    else if (m_type == BondType) {
      switch ( static_cast<BondColumn>(index.column()) ) {
      case BondDataType:
      case BondDataAtom1:
      case BondDataAtom2:
      case BondDataOrder:
      case BondDataRotatable:
        return QAbstractItemModel::flags(index);
      default: // lengths
        return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
      }
    }
    else if (m_type == AngleType) {
      switch ( static_cast<AngleColumn>(index.column()) ) {
      case AngleDataType:
      case AngleDataStartAtom:
      case AngleDataVertex:
      case AngleDataEndAtom:
        return QAbstractItemModel::flags(index);
      default: // an angle
        return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
      }
    }
    else if (m_type == TorsionType) {
      switch ( static_cast<TorsionColumn>(index.column()) ) {
      case TorsionDataType:
      case TorsionDataAtom1:
      case TorsionDataAtom2:
      case TorsionDataAtom3:
      case TorsionDataAtom4:
        return QAbstractItemModel::flags(index);
      default: // dihedral angle
        return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
      }
    }
    /*else if (m_type == CartesianType) {
      return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
    }*/
    else if (m_type == ConformerType) {
      return QAbstractItemModel::flags(index);
    }

    return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
  }

  bool PropertiesModel::setData(const QModelIndex &index,
                                const QVariant &value,
                                int role)
  {
    if (!index.isValid())
      return false;

    if (role != Qt::EditRole)
      return false;

    // If an item is actually editable, we should invalidate the cache
    // We can still use the cached data -- we just invalidate now
    // So that we can call "return" and have the cache invalid when we leave
    m_validCache = false;

    if (m_type == AtomType) {
      Atom *atom = m_molecule->atom(index.row());
      Eigen::Vector3d pos = *atom->pos();

      //switch (index.column()) {
      switch ( static_cast<AtomColumn>( index.column() ) ) {
      case AtomDataElement: {// atomic number
        // Try first as a number
        bool ok;
        int atomicNumber = value.toInt(&ok);
        if (ok)
          atom->setAtomicNumber(atomicNumber);
        else
          atom->setAtomicNumber(OpenBabel::etab.GetAtomicNum(value.toString().toAscii()));

        m_molecule->update();
        emit dataChanged(index, index);
        return true;
      }
      case AtomDataType:
      case AtomDataValence:
        return false;
      case AtomDataFormalCharge: {// formal charge
        bool ok;
        int formalCharge = value.toInt(&ok);
        if (ok)
          atom->setFormalCharge(formalCharge);
        return true;
      }
      case AtomDataPartialCharge: // partial charge
        atom->setPartialCharge(value.toDouble());
        m_molecule->update();
        emit dataChanged(index, index);
        return true;
      default: // A coordinate
        {
	  // Possible mutiple conformers, so we use the modulus to determine if x,y, or z
          pos [ (index.column()-5) % 3 ] = value.toDouble();
          atom->setPos(pos);
          m_molecule->update();
          emit dataChanged(index, index);
          return true;
        }
      }
    }
    /*else if (m_type == CartesianType) {
      if (index.column() > 2)
        return false;

      Atom *atom = m_molecule->atom(index.row());
      Eigen::Vector3d pos = *atom->pos();
      pos[index.column()] = value.toDouble();
      atom->setPos(pos);

      m_molecule->update();
      emit dataChanged(index, index);
      return true;
    }*/
    else if (m_type == BondType) {

      switch ( static_cast<BondColumn>(index.column()) ) {
      case BondDataType:
      case  BondDataAtom1:
      case  BondDataAtom2:
      case  BondDataOrder:
      case  BondDataRotatable:
	return false;
      default: // a length

	Bond *bond = m_molecule->bond(index.row());
	Eigen::Vector3d bondDirection = *(bond->beginPos()) - *(bond->endPos());
	double lengthScale;
	SkeletonTree zMatrixTree;

        lengthScale = (value.toDouble() - bond->length()) / bond->length();
        // scale our bond vector to match the new length
        bondDirection *= lengthScale;
        zMatrixTree.populate(bond->beginAtom(), bond, m_molecule);
        zMatrixTree.skeletonTranslate(bondDirection);
        m_molecule->update();
        emit dataChanged(index, index);
        return true;
      }


    }
    else if (m_type == AngleType) {

      vector<vector<unsigned int> > angles = conformerAngles( conformerFromIndex( index ) );
      Atom *startAtom = m_molecule->atom((angles[index.row()][1]));
      Atom *vertex = m_molecule->atom((angles[index.row()][0]));
      Atom *endAtom = m_molecule->atom((angles[index.row()][2]));
      Bond *bond = startAtom->bond(vertex);
      SkeletonTree zMatrixTree;
      Eigen::Vector3d abVector, bcVector, crossProductVector;
      double rotationAdjustment;

      double initialAngle = m_angleValues.at( m_molecule->currentConformer() ).at( index.row() );

      switch ( static_cast<AngleColumn>(index.column()) )
	{
	case AngleDataType:
	case AngleDataStartAtom:
	case AngleDataVertex:
	case AngleDataEndAtom:
	  return false;
	default: // an angle
	  abVector = *(startAtom->pos()) - *(vertex->pos());
	  bcVector = *(endAtom->pos()) - *(vertex->pos());
	  crossProductVector = abVector.cross(bcVector).normalized();
	  rotationAdjustment = (value.toDouble() - initialAngle) * cDegToRad;
	  zMatrixTree.populate(vertex, bond, m_molecule);
	  zMatrixTree.skeletonRotate(rotationAdjustment, crossProductVector, *(vertex->pos()));
	  m_molecule->update();
	  emit dataChanged(index, index);
	  return true;
	}
    }
    else if (m_type == TorsionType) {

      // Dihedral angles (torsions) are defined like so:
      // a
      //  \b-c
      //      \d

      vector<vector<unsigned int> > torsions = conformerTorsions( conformerFromIndex( index ) );

      Atom *b = m_molecule->atom((torsions[index.row()][1]));
      Atom *c = m_molecule->atom((torsions[index.row()][2]));
      Bond *bond = b->bond(c);
      SkeletonTree zMatrixTree;
      Eigen::Vector3d bcVector;
      double rotationAdjustment;

      double initialAngle = m_torsionValues.at( m_molecule->currentConformer() ).at(  index.row() );
      if (numeric_limits<double>::has_infinity &&
          initialAngle == numeric_limits<double>::infinity()) {
        initialAngle = 0.0;
      }

      switch ( static_cast<TorsionColumn>(index.column()) ) {
      case TorsionDataType:
      case TorsionDataAtom1:
      case TorsionDataAtom2:
      case TorsionDataAtom3:
      case TorsionDataAtom4:
        return false;
      default: // dihedral angle
        bcVector = (*(b->pos()) - *(c->pos())).normalized();
        rotationAdjustment = (value.toDouble() - initialAngle) * cDegToRad;

        zMatrixTree.populate(b, bond, m_molecule);
        zMatrixTree.skeletonRotate(rotationAdjustment, bcVector, *(b->pos()));
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
    m_validCache = false;
  }

  void PropertiesModel::setDisplayConformers(bool display)
  {
    m_displayConformers = display;
  }


  void PropertiesModel::atomAdded(Atom *atom)
  {
    if ( (m_type == AtomType) /*|| (m_type == CartesianType)*/ ) {
      // insert a new row at the end
      beginInsertRows(QModelIndex(), atom->index(), atom->index());
      endInsertRows();
    }
    m_validCache = false;
  }

  void PropertiesModel::atomRemoved(Atom *atom)
  {
    if ( (m_type == AtomType) /*|| (m_type == CartesianType)*/ )  {
      // delete the row for this atom
      beginRemoveRows(QModelIndex(), atom->index(), atom->index());
      endRemoveRows();
    }
    m_validCache = false;
  }

  void PropertiesModel::bondAdded(Bond *bond)
  {
    if ( (m_type == BondType) ) {
      // insert a new row at the end
      beginInsertRows(QModelIndex(), bond->index(), bond->index());
      endInsertRows();
    }
    m_validCache = false;
  }

  void PropertiesModel::bondRemoved(Bond *bond)
  {
    if ( (m_type == BondType) )  {
      // delete the row for this atom
      beginRemoveRows(QModelIndex(), bond->index(), bond->index());
      endRemoveRows();
    }
    m_validCache = false;
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

    m_validCache = false;
  }

  void PropertiesModel::updateTable()
  {

    emit dataChanged(QAbstractItemModel::createIndex(0, 0),
                     QAbstractItemModel::createIndex(rowCount(), columnCount()));

    // jmht - not sure we need to invalidate the cache when updating the table
    //m_validCache = false;
  }


  void PropertiesModel::clearCache( ) const
  {
    // Clear out the old data structures
    if (m_type == AtomType)
      {
	m_atomData.clear();
	m_atomCoords.clear();
      }
    else if (m_type == BondType)
      {
	m_bondData.clear();
	m_bondLengths.clear();
      }
    else if (m_type == AngleType)
      {
	m_angleData.clear();
	m_angleValues.clear();
	m_angles.clear();
      }
    else if (m_type == TorsionType)
      {
	m_torsionData.clear();
	m_torsionValues.clear();
	m_torsions.clear();
      }

    m_validCache = false;

  } // end clearCache

  void PropertiesModel::updateCache() const
  {

    /*
     * Loop through all conformers building up the data structures
     * For the first conformer we create the data structures that are
     * shared by all conformers
     */

    // Clear out the old data structures
    clearCache();

    Molecule *copy_molecule;
    if ( numConformers() > 1 )
      {
	// If we are dealing with multiple conformers, we create a
	// copy of the main molecule to get the data from.
	// We create a copy as we need to loop through the conformers
	// to generate the data, and this would alter the displayed molecule
	copy_molecule = new Molecule();
	*copy_molecule = *m_molecule;

	// Currently, we can't be sure this has been done
	copy_molecule->calculateGroupIndices();
      }
    else
      {
	// copy_molecule is just an alias for the original molecule
	copy_molecule = m_molecule;
      }

    std::vector<QVariant> tmpQVariantVector; // For holding vectors before they are added to the main data structures
    std::vector<OpenBabel::vector3> coordListTmp; // For holding atom coordinates
    std::vector<double> valueListTmp; // For holding atom coordinates or angles

    // Loop over conformers and create the data structures
    for ( unsigned int numConf=0; numConf < numConformers(); numConf++ )
      {

	if ( numConformers() > 1 )
	  copy_molecule->setConformer(numConf);

	OBMol *myOBMol = new OBMol(copy_molecule->OBMol());

	if (m_type == AtomType)
	  {
	    coordListTmp.clear();
	    // Loop over atoms
	    for (unsigned int j=0; j < myOBMol->NumAtoms(); j++ )
	      {
		OBAtom *obatom = myOBMol->GetAtom(j+1);

		// For the first conformer collect all the shared data
		if ( numConf == 0 )
		  {
		    tmpQVariantVector.clear();

		    // Element : Type : Valence : Formal Charge : Partial Charge
		    tmpQVariantVector.push_back(QVariant(OpenBabel::etab.GetSymbol(obatom->GetAtomicNum())));
		    tmpQVariantVector.push_back(QVariant(obatom->GetType()));
		    tmpQVariantVector.push_back(QVariant(obatom->GetValence()));
		    tmpQVariantVector.push_back(QVariant(obatom->GetFormalCharge()));
		    tmpQVariantVector.push_back(QVariant(obatom->GetPartialCharge()));

		    // Now add to atom data list
		    m_atomData.push_back( tmpQVariantVector );
		  }

		// Add coordinate to the list for this conformer
		coordListTmp.push_back( obatom->GetVector() );
	      } // end loop over atoms

	    // Add coordinates for all conformers
	    m_atomCoords.push_back( coordListTmp );

	  }
        else if (m_type == BondType)
          {
            OpenBabel::OBBond *bond;
            valueListTmp.clear();
            for (unsigned int j=0; j < myOBMol->NumBonds(); j++ )
              {
		bond = myOBMol->GetBond(j);
		// For first conformer, add all the shared data
		if ( numConf==0)
		  {
		    tmpQVariantVector.clear();

		    // Type
		    tmpQVariantVector.push_back(
						QVariant(bondTypeString(copy_molecule->atom(bond->GetBeginAtomIdx()-1),
									copy_molecule->atom(bond->GetEndAtomIdx()-1), 
									bond->GetBondOrder()))
						);
		    //Atom1
		    tmpQVariantVector.push_back(
						QVariant(groupIndexString(copy_molecule->atom(bond->GetBeginAtomIdx()-1)))
						);
		    //Atom2
		    tmpQVariantVector.push_back(
						QVariant(groupIndexString(copy_molecule->atom(bond->GetEndAtomIdx()-1)))
						);
		    // Order
		    tmpQVariantVector.push_back( QVariant( bond->GetBondOrder() ) );            
		    // Rotatable
		    if (bond->IsRotor())
		      tmpQVariantVector.push_back( QVariant( tr("Yes")) );
		    else
		      tmpQVariantVector.push_back( QVariant( tr("No")) );

		    m_bondData.push_back( tmpQVariantVector );

		  } // end numConf 0

		// Add lengths for all conformers
                valueListTmp.push_back( bond->GetLength() );
              }

            m_bondLengths.push_back( valueListTmp );
          }
        else if (m_type == AngleType)
          {
	    myOBMol->FindAngles();
	    OBAngleData *ad = static_cast<OBAngleData *>(myOBMol->GetData(AngleData));
	    vector<vector<unsigned int> > angles;
	    ad->FillAngleArray(angles);
	    double angle;
	    m_angles.push_back( angles );

            valueListTmp.clear();
	    // Loop over the angles
            for (unsigned int j=0; j < ad->GetSize(); j++ )
              {

		// Data shared by all conformers
		if ( numConf== 0 )
		  {
		    tmpQVariantVector.clear();
		    // Type
		    tmpQVariantVector.push_back( QVariant(
							  angleTypeString(copy_molecule->atom(angles[j][1]),
									  copy_molecule->atom(angles[j][0]),
									  copy_molecule->atom(angles[j][2]))
							  ) );
		    // Start atom
		    tmpQVariantVector.push_back( QVariant(
							  groupIndexString(copy_molecule->atom(angles[j][1]))
							  ));

		    // vertex -- yes, angles are filled by Open Babel with the vertex first
		    tmpQVariantVector.push_back( QVariant(
							  groupIndexString(copy_molecule->atom(angles[j][0]))
							  ));
		    // End atom
		    tmpQVariantVector.push_back( QVariant(
							  groupIndexString(copy_molecule->atom(angles[j][2]))
							  ));
		    m_angleData.push_back( tmpQVariantVector );
		  }

		angle = myOBMol->GetAngle(myOBMol->GetAtom(angles[j][1] + 1),
					  myOBMol->GetAtom(angles[j][0] + 1),
					  myOBMol->GetAtom(angles[j][2] + 1));
		if (numeric_limits<double>::has_infinity &&
		    angle == numeric_limits<double>::infinity())
		  angle = 0.0;
                valueListTmp.push_back( angle );
              }

            m_angleValues.push_back( valueListTmp );
	  }
        else if (m_type == TorsionType)
          {
	    myOBMol->FindTorsions();
	    OBTorsionData *td = static_cast<OBTorsionData *>(myOBMol->GetData(TorsionData));
	    vector<OBTorsion> torsions = td->GetData();
	    pair<OBAtom*,OBAtom*> torsionBC;
	    vector<triple<OBAtom*,OBAtom*,double> > torsionADs;
	    vector<OBTorsion>::iterator i;
	    vector<triple<OBAtom*,OBAtom*,double> >::iterator j;

	    // Add to the list for all conformers
	    vector<vector<unsigned int> > mytorsions;
	    td->FillTorsionArray(mytorsions);
	    m_torsions.push_back( mytorsions );

	    double dihedralAngle;
            valueListTmp.clear();
	    for (i = torsions.begin(); i != torsions.end(); ++i) {
	      torsionBC = i->GetBC();
	      torsionADs = i->GetADs();
	      for (j = torsionADs.begin(); j != torsionADs.end(); ++j) {

		// Add data shared by all conformers
		if (numConf==0)
		  {

		    tmpQVariantVector.clear();
		    //Type
		    tmpQVariantVector.push_back( QVariant(
							  angleTypeString(copy_molecule->atom(j->first->GetIdx()-1),
									  copy_molecule->atom(torsionBC.first->GetIdx()-1),
									  copy_molecule->atom(torsionBC.second->GetIdx()-1),
									  copy_molecule->atom(j->second->GetIdx()-1))
							  ));
		    //Atom1
		    tmpQVariantVector.push_back( QVariant(
							  groupIndexString(copy_molecule->atom(j->first->GetIdx()-1))
							  ));
		    //Atom2
		    tmpQVariantVector.push_back( QVariant(
							  groupIndexString(copy_molecule->atom(torsionBC.first->GetIdx()-1))
							  ));
		    //Atom3
		    tmpQVariantVector.push_back( QVariant(
							  groupIndexString(copy_molecule->atom(torsionBC.second->GetIdx()-1))
							  ));
		    //Atom4
		    tmpQVariantVector.push_back( QVariant(
							  groupIndexString(copy_molecule->atom(j->second->GetIdx()-1))
							  ));
		    // Now add final data structure
		    m_torsionData.push_back( tmpQVariantVector );
		  }

		dihedralAngle = myOBMol->GetTorsion(j->first,
						    torsionBC.first,
						    torsionBC.second,
						    j->second);
		if (numeric_limits<double>::has_infinity &&
		    dihedralAngle == numeric_limits<double>::infinity())
		  dihedralAngle = 0.0;
		valueListTmp.push_back(dihedralAngle);
	      }
	    }
            m_torsionValues.push_back( valueListTmp );
	  } // end TorsionType

      } // end loop over conformers

    // Delete our copy of the molecule if we made one
    if ( numConformers() > 1 )
      delete copy_molecule;

    m_validCache = true;

  } // end updateCache


  unsigned int PropertiesModel::numConformers() const
  {
    if (! m_displayConformers )
      return 1;
    else
      return m_molecule->numConformers();

  } //end numConformers

  unsigned int PropertiesModel::conformerFromIndex(const QModelIndex &index) const
  {
    if (m_type == AtomType) {
      if (static_cast<unsigned int>(index.row()) >= m_molecule->numAtoms() ||
          static_cast<unsigned int>(index.column()) > 5 + (3*numConformers()) )
        return 0;
      if ( index.column() < 5 )
	return 0;
      else
	return (index.column()-5) / 3; // Conformer number
    }
    else if (m_type == BondType) {
      if (static_cast<unsigned int>(index.row()) >= m_molecule->numBonds() ||
          static_cast<unsigned int>(index.column()) > 5 + (3*numConformers()) )
        return 0;
      if ( index.column() < 5 )
	return 0;
      else
	return index.column()-5;
    }
    else if (m_type == AngleType) {
      if ( index.column() < 4 )
	return 0;
      else
	return index.column()-4;
    }
    else if (m_type == TorsionType) {
      if ( index.column() < 5 )
	return 0;
      else
	return index.column()-5;
    }
    else if (m_type == ConformerType) {
      if (index.row() >= static_cast<int>(numConformers()))
	return 0;
      else
	return index.row();
    }
    else
      return 0;
  }


  std::vector< std::vector<unsigned int> > PropertiesModel::conformerAngles( unsigned int conformer )
  {
    // Return the list of angles for a conformer
    if (!m_validCache)
      updateCache();

    if ( conformer > m_angles.size() )
      {
	std::cerr << "conformerAngles bad conformer index\n";
	std::vector< std::vector<unsigned int> > angles;
	return angles;
      }

    return m_angles.at( conformer );
  } // end conformerAngles

  std::vector< std::vector<unsigned int> > PropertiesModel::conformerTorsions( unsigned int conformer )
  {

    // Return the list of torsions for a conformer
    if (!m_validCache)
      updateCache();

    if ( conformer > m_torsions.size() )
      {
	std::cerr << "conformerTorsions bad conformer index!\n";
	std::vector< std::vector<unsigned int> > torsions;
	return torsions;
      }

    return m_torsions.at( conformer );
  } // end conformerTorsions


} // end namespace Avogadro
