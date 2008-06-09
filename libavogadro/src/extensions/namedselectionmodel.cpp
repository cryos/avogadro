/**********************************************************************
  namedselectionmodel.cpp - Model to hold named selections

  Copyright (C) 2008 by Tim Vandermeersch

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

#include "namedselectionmodel.h"
#include <avogadro/primitive.h>
#include <avogadro/color.h>
#include <avogadro/glwidget.h>

#include <openbabel/mol.h>

#include <QDebug>

using namespace std;
using namespace OpenBabel;

namespace Avogadro
{
  int NamedSelectionModel::rowCount(const QModelIndex &parent) const
  {
    Q_UNUSED(parent);
    
    return m_widget->namedSelections().size();
  }

  QVariant NamedSelectionModel::data(const QModelIndex &index, int role) const
  {
    if (!index.isValid())
      return QVariant();

    if (role != Qt::DisplayRole)
      return QVariant();
    
    if (index.row() >= m_widget->namedSelections().size())
      return QVariant();

    return m_widget->namedSelections().at(index.row());
  }
  
  QVariant NamedSelectionModel::headerData(int section, Qt::Orientation orientation, int role) const
  {
    if (role != Qt::DisplayRole)
      return QVariant();

    return tr("Selections");
  }
 
  Qt::ItemFlags NamedSelectionModel::flags(const QModelIndex &index) const
  {
    return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
  }

  bool NamedSelectionModel::setData(const QModelIndex &index, const QVariant &value, int role) 
  {
    if (!index.isValid())
      return false; 
      
    if (role != Qt::EditRole)
      return false;
    
    m_widget->renameNamedSelection(index.row(), value.toString());
    
    return true;
  }

} // end namespace Avogadro

#include "namedselectionmodel.moc"
