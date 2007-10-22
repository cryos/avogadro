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

#ifndef __PRIMITIVEITEMMODEL_H
#define __PRIMITIVEITEMMODEL_H

#include <avogadro/global.h>
#include <avogadro/primitivelist.h>
#include <avogadro/engine.h>

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QObject>
#include <QVariant>

class QTreeView;
namespace Avogadro {

  class PrimitiveItemModelPrivate;
  class A_EXPORT PrimitiveItemModel : public QAbstractItemModel
  {
    Q_OBJECT

    public:
      enum Role {
        PrimitiveRole = Qt::UserRole + 1,
        PrimitiveTypeRole,
      };

    public:
      explicit PrimitiveItemModel( Engine *engine, QObject *parent = 0 );
      explicit PrimitiveItemModel( Molecule *molecule, QObject *parent = 0 );

      QModelIndex parent( const QModelIndex & index ) const;
      int rowCount( const QModelIndex & parent = QModelIndex() ) const;
      int columnCount( const QModelIndex & parent = QModelIndex() ) const;
      QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const;
      Qt::ItemFlags flags ( const QModelIndex & index ) const;

      QModelIndex index ( int row, int column, const QModelIndex & parent = QModelIndex() ) const;

    private Q_SLOTS:
      void engineChanged();
      void addPrimitive(Primitive *primitive);
      void updatePrimitive(Primitive *primitive);
      void removePrimitive(Primitive *primitive);

    private:
      PrimitiveItemModelPrivate * const d;

      int primitiveIndex(Primitive *primitive);

  };

}  // end namespace Avogadro

#endif // __PRIMITIVEITEMMODEL_H
