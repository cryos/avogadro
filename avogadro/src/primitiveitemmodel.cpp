/**********************************************************************
  PrimitiveItemModel - Model for representing primitives.

  Copyright (C) 2007 Donald Ephraim Curtis <dcurtis3@sourceforge.net>

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.openmolecules.net/>

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

#include "primitiveitemmodel.h"

#include <QVector>
#include <QDebug>

#include <avogadro/atom.h>
#include <avogadro/bond.h>
#include <avogadro/residue.h>
#include <avogadro/molecule.h>
#include <avogadro/engine.h>

#include <openbabel/mol.h>

namespace Avogadro {
  class PrimitiveItemModelPrivate
  {
    public:
      PrimitiveItemModelPrivate() : engine(0), molecule(0) {}

      Engine *engine;

      Molecule *molecule;

      // use this to optimize insert / delete
      QMap<int, Primitive::Type> rowTypeMap;

      // keep track of the model sizes for each parent
      QVector<int> size;

      /*
       * for Molecules we have to cache additions / subtractions
       * because when we get the signal we haven't actually added
       * the atom to the molecule, rather it's been created but not
       * yet added.
       */
      QVector<QVector<Primitive *> > moleculeCache;
  };

  PrimitiveItemModel::PrimitiveItemModel( Engine *engine, QObject *parent)
    : QAbstractItemModel(parent), d(new PrimitiveItemModelPrivate)
  {
    d->engine = engine;

    d->rowTypeMap.insert(0, Primitive::AtomType);
    d->rowTypeMap.insert(1, Primitive::BondType);
    d->rowTypeMap.insert(2, Primitive::ResidueType);

    d->size.resize(d->rowTypeMap.size());

    connect(engine, SIGNAL(changed()), this, SLOT(engineChanged()));
    PrimitiveList list = engine->primitives();

    d->size[0] = engine->atoms().size();
    d->size[1] = engine->bonds().size();
    d->size[2] = engine->primitives().subList(Primitive::ResidueType).size();
  }

  PrimitiveItemModel::PrimitiveItemModel(Molecule *molecule, QObject *parent)
    : QAbstractItemModel(parent), d(new PrimitiveItemModelPrivate)
  {
    d->molecule = molecule;

    d->rowTypeMap.insert(0, Primitive::AtomType);
    d->rowTypeMap.insert(1, Primitive::BondType);
    d->rowTypeMap.insert(2, Primitive::ResidueType);

    d->size.resize(d->rowTypeMap.size());
    d->moleculeCache.resize(d->rowTypeMap.size());

    d->size[0] = molecule->numAtoms();
    d->size[1] = molecule->numBonds();
    d->size[2] = molecule->numResidues();

    connect(molecule, SIGNAL(primitiveAdded(Primitive *)),
        this, SLOT(addPrimitive(Primitive *)));
    connect(molecule, SIGNAL(primitiveUpdated(Primitive *)),
        this, SLOT(updatePrimitive(Primitive *)));
    connect(molecule, SIGNAL(primitiveRemoved(Primitive *)),
        this, SLOT(removePrimitive(Primitive *)));
  }

  PrimitiveItemModel::~PrimitiveItemModel()
  {
      delete d;
  }

  void PrimitiveItemModel::addPrimitive(Primitive *primitive)
  {
    int parentRow = d->rowTypeMap.key(primitive->type());

    if(parentRow < d->size.size())
    {
      emit layoutAboutToBeChanged(); // we need to tell the view that the data is going to change

      int last = d->size[parentRow]++;
      beginInsertRows(createIndex(parentRow, 0, 0), last, last);
      if(d->molecule) {
        d->moleculeCache[parentRow].append(primitive);
      }
      endInsertRows();

      emit layoutChanged(); // we need to tell the view to refresh
    }
  }

  void PrimitiveItemModel::updatePrimitive(Primitive *primitive)
  {
    int parentRow = d->rowTypeMap.key(primitive->type());

    if(parentRow < d->size.size())
    {
      int row = primitiveIndex(primitive);
      emit dataChanged(createIndex(row, 0, primitive), createIndex(row, 0, primitive));
    }
  }

  void PrimitiveItemModel::removePrimitive(Primitive *primitive)
  {
    int parentRow = d->rowTypeMap.key(primitive->type());
    if(parentRow < d->size.size())
    {
      int row = primitiveIndex(primitive);
      //assert(row > -1);
      if (row < 0)
        return;
      emit layoutAboutToBeChanged(); // we need to tell the view that the data is going to change

      beginRemoveRows(createIndex(parentRow, 0, 0), row, row);
      if(d->molecule)
      {
        d->moleculeCache[parentRow].remove(row);
      }
      d->size[parentRow]--;
      endRemoveRows();

      emit layoutChanged(); // we need to tell the view to refresh
    }
  }

  int PrimitiveItemModel::primitiveIndex(Primitive *primitive)
  {
    if(d->molecule)
    {
      int parentRow = d->rowTypeMap.key(primitive->type());
      return d->moleculeCache[parentRow].indexOf(primitive);
    }
    else if (d->engine)
    {
      QList<Primitive *> subList = d->engine->primitives().subList(primitive->type());
      return subList.indexOf(primitive);
    }

    return -1;

  }

  void PrimitiveItemModel::engineChanged()
  {
    PrimitiveList list = d->engine->primitives();
    foreach(const int row, d->rowTypeMap.keys()) // krazy:exclude=foreach
    {
      Primitive::Type type = d->rowTypeMap[row];
      int newsize = list.count(type);
      int oldsize = d->size.at(row);
      if(newsize < oldsize)
      {
        d->size[row] = newsize;

        emit layoutAboutToBeChanged(); // we need to tell the view that the data is going to change

        beginRemoveRows(createIndex(row,0,0), newsize, oldsize-1);
        // this is a minor hack to simplify things although it doesn't currently update the view
        endRemoveRows();

        emit layoutChanged();
      }
      else if(newsize > oldsize)
      {
        d->size[row] = newsize;
        emit layoutAboutToBeChanged(); // we need to tell the view that the data is going to change
        beginInsertRows(createIndex(row,0,0), oldsize, newsize-1);
        endInsertRows();
        emit layoutChanged(); // we need to tell the view that the data is going to change
      }
    }
  }

  QModelIndex PrimitiveItemModel::parent( const QModelIndex & index ) const
  {
    if(!index.isValid())
    {
      return QModelIndex();
    }

    Primitive *primitive = static_cast<Primitive *>(index.internalPointer());
    if(primitive)
    {
      int row = d->rowTypeMap.key(primitive->type());
      return createIndex(row, 0, 0);
    }
    return QModelIndex();
  }

  int PrimitiveItemModel::rowCount( const QModelIndex & parent ) const
  {
    if(!parent.isValid())
    {
      return d->rowTypeMap.size();
    }

    Primitive *primitive = static_cast<Primitive *>(parent.internalPointer());
    if(!primitive)
    {
      return d->size[parent.row()];
    }

    return 0;
  }

  int PrimitiveItemModel::columnCount( const QModelIndex & ) const
  {
    return 1;
  }

  QVariant PrimitiveItemModel::data (const QModelIndex & index, int role) const
  {
    if(!index.isValid() || index.column() != 0)
    {
      return QVariant();
    }

    Primitive *primitive = static_cast<Primitive *>(index.internalPointer());
    if(primitive)
    {
      if(role == Qt::DisplayRole) {
        Primitive::Type type = primitive->type();

        QString str;
        if(type == Primitive::MoleculeType) {
          str = tr("Molecule");
        }
        else if(type == Primitive::AtomType) {
          Atom *atom = static_cast<Atom*>(primitive);
          str = tr("Atom %1 %L2", "%1 is element, %L2 is atom index").arg(QString(OpenBabel::etab.GetSymbol(atom->atomicNumber())))
            .arg(atom->index());
        }
        else if(type == Primitive::BondType){
          Bond *bond = static_cast<Bond*>(primitive);
          str = tr("Bond %L1", "%L1 is bond index").arg(bond->index());
          if (d->molecule) {
            const Atom *beginAtom = d->molecule->atomById(bond->beginAtomId());
            const Atom *endAtom = d->molecule->atomById(bond->endAtomId());
            str += QString(" (%L1, %L2)")
              .arg(beginAtom->index())
              .arg(endAtom->index());
          }
        } // end bond
        else if(type == Primitive::ResidueType) {
          Residue *residue = (Residue*)primitive;
          str = tr("Residue %1 %2", "%1 is residue name, %2 is residue index")
            .arg(residue->name())
            .arg(residue->number());
        }

        return str;

      } else if ( role == PrimitiveItemModel::PrimitiveRole ) {
        return qVariantFromValue(primitive);
      }
    }

    if(role == Qt::DisplayRole && index.row() < d->rowTypeMap.size())
    {
      Primitive::Type type = d->rowTypeMap[index.row()];
      if(type == Primitive::AtomType) {
        return tr("Atoms");
      } else if (type == Primitive::BondType) {
        return tr("Bonds");
      } else if (type == Primitive::ResidueType) {
        return tr("Residues");
      }

    }
    return QVariant();
  }

  Qt::ItemFlags PrimitiveItemModel::flags ( const QModelIndex & index ) const
  {
    if(!index.isValid()) {
      return 0;
    }

    Primitive *primitive = static_cast<Primitive *>(index.internalPointer());
    if(primitive)
    {
      return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    }

    return Qt::ItemIsEnabled;
  }

  QModelIndex PrimitiveItemModel::index ( int row, int column,
                                          const QModelIndex & parent ) const
  {
    if(!parent.isValid())
    {
      return createIndex(row, column);
    }

    Primitive *primitive = static_cast<Primitive *>(parent.internalPointer());
    if(primitive)
    {
      return QModelIndex();
    }

    if(d->engine)
    {
      QList<Primitive *> subList =
        d->engine->primitives().subList(d->rowTypeMap[parent.row()]);
      if(row < subList.size()) {
        return createIndex(row, column, subList.at(row));
      }
    } else if (d->molecule) {
      // Primitive::Type type = d->rowTypeMap[parent.row()];
      int parentRow = parent.row();

      Primitive *primitive;
      primitive = d->moleculeCache[parentRow].at(row);
      return createIndex(row, column, primitive);
    }

    return QModelIndex();
  }

} // end namespace Avogadro

#include "primitiveitemmodel.moc"
