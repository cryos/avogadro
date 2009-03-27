/**********************************************************************
  ZMatrixModel - ZMatrix Table Model

  Copyright (C) 2009 by Marcus D. Hanwell

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

#include "zmatrixmodel.h"

#include <QDebug>

namespace Avogadro {

  ZMatrixModel::ZMatrixModel() : m_zMatrix(0)
  {
  }

  ZMatrixModel::~ZMatrixModel()
  {
  }

  void ZMatrixModel::setZMatrix(ZMatrix *zmatrix)
  {
    disconnect(m_zMatrix, 0, 0, 0);
    m_zMatrix = zmatrix;
    connect(m_zMatrix, SIGNAL(rowAdded(int)), this, SLOT(addRow(int)));
    qDebug() << "Set z matrix" << m_zMatrix;
  }

  ZMatrix * ZMatrixModel::zMatrix()
  {
    return m_zMatrix;
  }

  int ZMatrixModel::rowCount(const QModelIndex &) const
  {
    if (m_zMatrix)
      return m_zMatrix->rows();
    else
      return 0;
  }

  int ZMatrixModel::columnCount(const QModelIndex &) const
  {
    return 7;
  }

  QVariant ZMatrixModel::headerData(int section, Qt::Orientation orientation,
                                    int role) const
  {
    if (role != Qt::DisplayRole)
      return QVariant();
    if (orientation == Qt::Horizontal) {
      switch (section) {
        case 0:
          return QString(tr("Symbol"));
          break;
        case 1:
        case 3:
        case 5:
          return QVariant();
          break;
        case 2:
          return QString(tr("Bond Length"));
          break;
        case 4:
          return QString(tr("Bond Angle"));
          break;
        case 6:
          return QString(tr("Dihedral Angle"));
          break;
        default:
          return QString(tr("Unknown"));
      }
    }
    else
      return QString::number(section + 1);
  }

  Qt::ItemFlags ZMatrixModel::flags(const QModelIndex &index) const
  {
    if (!index.isValid())
      return Qt::ItemIsEnabled;
    if (index.row() == 0 && index.column() > 0)
      return Qt::NoItemFlags;
    else if (index.row() == 1 && index.column() > 2)
      return Qt::NoItemFlags;
    else if (index.row() == 2 && index.column() > 4)
      return Qt::NoItemFlags;
    return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
  }

  QVariant ZMatrixModel::data(const QModelIndex &index, int role) const
  {
    if (!index.isValid())
      return QVariant();

    if (!index.isValid())
      return QVariant();
    if (index.row() == 0 && index.column() > 0)
      return QVariant();
    else if (index.row() == 1 && index.column() > 2)
      return QVariant();
    else if (index.row() == 2 && index.column() > 4)
      return QVariant();

    if (index.row() >= 5 || index.column() >= 7)
      return QVariant();

    if (role == Qt::DisplayRole)
      return QString("No worky!");
    else
      return QVariant();
  }

  bool ZMatrixModel::setData(const QModelIndex &index, const QVariant &value,
                             int role)
  {
    if (index.isValid() && role == Qt::EditRole) {
      // do stuff
      return true;
    }
    return false;
  }

  bool ZMatrixModel::insertRows(int position, int rows,
                                const QModelIndex &index)
  {
    if (m_zMatrix) {
      beginInsertRows(QModelIndex(), position, position+rows-1);
      for (int row = 0; row < rows; ++row) {
        m_zMatrix->addRow(position+row);
      }
      endInsertRows();
      return true;
    }
    return false;
  }

  bool ZMatrixModel::removeRows(int position, int rows,
                                const QModelIndex &index)
  {
  }

  void ZMatrixModel::addRow(int row)
  {
    beginInsertRows(QModelIndex(), row, row+1);
    endRemoveRows();
  }

} // End namespace Avogadro
