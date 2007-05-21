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

#include "engineitemmodel.h"

#include <avogadro/glwidget.h>
#include <QString>

namespace Avogadro {
  class EngineItemModelPrivate
  {
    public:
      GLWidget *widget;
  };

  EngineItemModel::EngineItemModel( GLWidget *widget, QObject *parent ) : d(new EngineItemModelPrivate)
  {
    d->widget = widget;
  }

  QModelIndex EngineItemModel::parent( const QModelIndex & index ) const
  {
    return QModelIndex();
  }

  int EngineItemModel::columnCount( const QModelIndex & parent ) const
  {
    return 1;
  }

  int EngineItemModel::rowCount( const QModelIndex & parent ) const
  {
    if(!parent.isValid())
    {
      return d->widget->engines().size();
    }
    else
    {
      return 0;
    }
  }

  QVariant EngineItemModel::data ( const QModelIndex & index, int role ) const
  {
    if(!index.isValid() || index.column() != 0)
    {
      return QVariant();
    }

    Engine *engine = qobject_cast<Engine *>(static_cast<QObject *>(index.internalPointer()));
    if(engine)
    {
      if(role == Qt::DisplayRole) {
          return engine->name();
      } else if ( role == Qt::CheckStateRole) {
          if(engine->isEnabled()) {
            return Qt::Checked;
          } else {
            return Qt::Unchecked;
          }
      } else if ( role == EngineItemModel::EngineRole ) {
        return qVariantFromValue(engine);
      }

    }

    return QVariant();
  }

  bool EngineItemModel::setData ( const QModelIndex & index, const QVariant & value, int role )
  {
    if(!index.isValid() || !index.internalPointer()) {
      return false;
    }

    Engine *engine = qobject_cast<Engine *>(static_cast<QObject *>(index.internalPointer()));
    if(role == Qt::CheckStateRole) {
      if(value == Qt::Checked) {
        engine->setEnabled(true);
      } else {
        engine->setEnabled(false);
      }
      emit dataChanged(index, index);
      return true;
    } else if ( role == Qt::DisplayRole ) {
      engine->setName(value.toString());
      emit dataChanged(index, index);
      return true;
    }

    return false;
  }

  Qt::ItemFlags EngineItemModel::flags ( const QModelIndex & index ) const
  {
    return (Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
  }

  QModelIndex EngineItemModel::index ( int row, int column, const QModelIndex & parent ) const
  {
    //FIXME: (bjacob) I added the "&& row >=0" condition below because I had to
    //fix a failed assert. It'd be cleaner to fix the cause of the problem, which is that
    //this function is being called with row=-1.
    if(!parent.isValid() && row >=0)
    {
      Engine *engine = d->widget->engines().at(row);
      return createIndex(row,column,engine);
    }

    return QModelIndex();
  }

} // end namespace Avogadro

#include "engineitemmodel.moc"
