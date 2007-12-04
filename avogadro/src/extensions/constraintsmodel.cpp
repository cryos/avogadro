/**********************************************************************
  forcefieldextension.cpp - molecular mechanics force field Plugin for Avogadro

  Copyright (C) 2006 by Donald Ephraim Curtis
  Copyright (C) 2006-2007 by Geoffrey R. Hutchison

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

#include "constraintsmodel.h"
#include <avogadro/primitive.h>
#include <avogadro/color.h>
#include <avogadro/glwidget.h>

#include <QtGui>
#include <QProgressDialog>
#include <QWriteLocker>
#include <QMutex>
#include <QMutexLocker>

using namespace std;
using namespace OpenBabel;

namespace Avogadro
{
  
  int ConstraintsModel::rowCount(const QModelIndex &parent) const
  {
    return m_constraints.Size();
  }

  int ConstraintsModel::columnCount(const QModelIndex &parent) const
  {
    return 1;
  }
  
  QVariant ConstraintsModel::data(const QModelIndex &index, int role) const
  {
    if (!index.isValid())
      return QVariant();

    if (index.row() >= m_constraints.Size())
      return QVariant();
 
    if (role == Qt::DisplayRole)
      return m_constraints.GetConstraintType(index.row());
    else
      return QVariant();
  }

} // end namespace Avogadro

#include "constraintsmodel.moc"
