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

#ifndef ZMATRIXMODEL_H
#define ZMATRIXMODEL_H

#include <QAbstractTableModel>

#include <avogadro/zmatrix.h>

namespace Avogadro {

  class ZMatrixModel : public QAbstractTableModel
  {
  public:
    ZMatrixModel();
    ~ZMatrixModel();

    void setZMatrix(ZMatrix *zmatrix);
    ZMatrix * zMatrix();

    // Implementing virtual methods for the model
    int rowCount(const QModelIndex & parent = QModelIndex()) const;
    int columnCount(const QModelIndex & parent = QModelIndex()) const;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole ) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    bool setData(const QModelIndex &index, const QVariant &value,
                 int role = Qt::EditRole);

    bool insertRows(int position, int rows,
                    const QModelIndex &index = QModelIndex());
    bool removeRows(int position, int rows,
                    const QModelIndex &index = QModelIndex());

  private:
    ZMatrix *m_zMatrix;

  private Q_SLOTS:
    void addRow(int row);
  };

} // End namespace Avogadro

#endif // ZMATRIXMODEL_H
