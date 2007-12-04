/**********************************************************************
  EngineItemModel - List Model for Engines

  Copyright (C) 2007 Donald Ephraim Curtis

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

#ifndef __ENGINEITEMMODEL_H
#define __ENGINEITEMMODEL_H

#include <QModelIndex>

namespace Avogadro {
  class GLWidget;

  class Engine;
  class EngineItemModelPrivate;
  class EngineItemModel : public QAbstractItemModel
  {
    Q_OBJECT

    public:
      enum Role {
        EngineRole = Qt::UserRole + 1,
      };

    public:
      explicit EngineItemModel( GLWidget *widget, QObject *parent = 0 );

      QModelIndex parent( const QModelIndex & index ) const;
      int rowCount( const QModelIndex & parent = QModelIndex() ) const;
      int columnCount( const QModelIndex & parent = QModelIndex() ) const;
      QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const;
      bool setData ( const QModelIndex & index, const QVariant & value, int role = Qt::EditRole );
      Qt::ItemFlags flags ( const QModelIndex & index ) const;

      QModelIndex index ( int row, int column, const QModelIndex & parent = QModelIndex() ) const;

    private:
      EngineItemModelPrivate * const d;

    private Q_SLOTS:
      void addEngine(Engine *engine);
      void removeEngine(Engine *engine);

  };

} // end namespace Avogadro

#endif
