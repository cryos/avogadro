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

#include <avogadro/primitiveitemmodel.h>

#include <QString>
#include <QDebug>
#include <QTreeView>

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

  };

  PrimitiveItemModel::PrimitiveItemModel( Engine *engine, QObject *parent) : QAbstractItemModel(parent), d(new PrimitiveItemModelPrivate)
  {
    d->engine = engine;

    d->rowTypeMap.insert(0, Primitive::AtomType);
    d->rowTypeMap.insert(1, Primitive::BondType);
    d->rowTypeMap.insert(2, Primitive::ResidueType);

    d->size.resize(d->rowTypeMap.size());

    connect(engine, SIGNAL(changed()), this, SLOT(engineChanged()));
    PrimitiveList list = engine->primitives();
    foreach(int row, d->rowTypeMap.keys())
    {
      d->size[row] = list.size(d->rowTypeMap[row]);
    }
  }

  PrimitiveItemModel::PrimitiveItemModel( Molecule *molecule, QObject *parent) : QAbstractItemModel(parent), d(new PrimitiveItemModelPrivate)
  {
    d->molecule = molecule;

    d->rowTypeMap.insert(0, Primitive::AtomType);
    d->rowTypeMap.insert(1, Primitive::BondType);
    d->rowTypeMap.insert(2, Primitive::ResidueType);

    d->size.resize(d->rowTypeMap.size());

    d->size[0] = molecule->NumAtoms();
    d->size[1] = molecule->NumBonds();
    d->size[2] = molecule->NumResidues();

    connect(molecule, SIGNAL(primitiveAdded(Primitive *)),
        this, SLOT(addPrimitive(Primitive *)));
    connect(molecule, SIGNAL(primitiveUpdated(Primitive *)),
        this, SLOT(updatePrimitive(Primitive *)));
    connect(molecule, SIGNAL(primitiveRemoved(Primitive *)),
        this, SLOT(removePrimitive(Primitive *)));
  }

  void PrimitiveItemModel::addPrimitive(Primitive *primitive)
  {
    int parentRow = d->rowTypeMap.key(primitive->type());

    if(parentRow < d->size.size())
    {
      int last = d->size[parentRow]++;
      emit beginInsertRows(createIndex(parentRow, 0, 0), last, last);
      endInsertRows();
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
      emit beginRemoveRows(createIndex(parentRow, 0, 0), row, row);
      d->size[parentRow]--;
      endRemoveRows();
    }
  }

  int PrimitiveItemModel::primitiveIndex(Primitive *primitive)
  {
    Primitive::Type type = primitive->type();
    if(type == Primitive::AtomType) {
      Atom *atom = static_cast<Atom *>(primitive);
      return atom->GetIdx()-1;
    } else if (type == Primitive::BondType) {
      Bond *bond = static_cast<Bond *>(primitive);
      return bond->GetIdx();
    } else if (type == Primitive::ResidueType) {
      Residue *residue = static_cast<Residue *>(primitive);
      return residue->GetIdx();
    }
    return 0;
  }

  void PrimitiveItemModel::engineChanged()
  {
    PrimitiveList list = d->engine->primitives();
    foreach(int row, d->rowTypeMap.keys())
    {
      Primitive::Type type = d->rowTypeMap[row];
      int newsize = list.size(type);
      int oldsize = d->size.at(row);
      if(newsize < oldsize)
      {
        d->size[row] = newsize;

        beginRemoveRows(createIndex(row,0,0), newsize, oldsize-1);
        QList<Primitive *> subList = list.subList(type);
        if(subList.size())
        {
          // this is a minor hack to simplify things although it doesn't currently update the view
          emit layoutChanged();
        }
        endRemoveRows();
      }
      else if(newsize > oldsize)
      {
        d->size[row] = newsize;
        beginInsertRows(createIndex(row,0,0), oldsize, newsize-1);
        endInsertRows();
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

  QVariant PrimitiveItemModel::data ( const QModelIndex & index, int role ) const
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
        QString name;
        if(type == Primitive::AtomType) {
          Atom *atom = qobject_cast<Atom *>(primitive);
          if(atom)
          {
            name = tr("Atom ") + QString::number(atom->GetIdx());
          }
        } else if (type == Primitive::BondType) {
          Bond *bond = qobject_cast<Bond *>(primitive);
          if(bond)
          {
            name = tr("Bond ") + QString::number(bond->GetIdx());
          }
        }
        return name;

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

  QModelIndex PrimitiveItemModel::index ( int row, int column, const QModelIndex & parent ) const
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
      Primitive::Type type = d->rowTypeMap[parent.row()];

      Primitive *primitive;
      if(type == Primitive::AtomType) {
        primitive = static_cast<Atom *>(d->molecule->GetAtom(row+1));
      } else if (type == Primitive::BondType) {
        primitive = static_cast<Bond *>(d->molecule->GetBond(row));
      } else if (type == Primitive::ResidueType) {
        primitive = static_cast<Residue *>(d->molecule->GetResidue(row));
      }
      return createIndex(row, column, primitive);
    }

    return QModelIndex();
  }

} // end namespace Avogadro

#include "primitiveitemmodel.moc"
