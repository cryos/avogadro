/**********************************************************************
  OrbitalExtension - Molecular orbital explorer

  Copyright (C) 2010 by David C. Lonie

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.openmolecules.net/>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation version 2 of the License.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
 ***********************************************************************/

#include "orbitaltablemodel.h"
#include "orbitalwidget.h"

namespace Avogadro {

  OrbitalTableModel::OrbitalTableModel( QWidget *parent)
    : QAbstractTableModel(parent)
  {
    m_orbitals.clear();
  }

  OrbitalTableModel::~OrbitalTableModel()
  {
  }

  QVariant OrbitalTableModel::data(const QModelIndex & index, int role) const
  {
    if (role != Qt::DisplayRole || !index.isValid())
      return QVariant();

    const Orbital orb = m_orbitals.at(index.row());

    switch (Column(index.column())) {
    case C_Description:
      return orb.description;
    case C_Energy:
      return QString::number(orb.energy, 'g', 5);
    case C_Status:
      // Check for divide by zero
      if (orb.max == orb.min)
        return -1;
      return int( (orb.current - orb.min) /
                  (orb.max - orb.min) );
    default:
      case COUNT:
      return QVariant();

    }
  }

  QVariant OrbitalTableModel::headerData(int section, Qt::Orientation orientation, int role) const
  {
    if (role != Qt::DisplayRole)
      return QVariant();

    if (orientation == Qt::Horizontal) {
      switch (Column(section)) {
      case C_Description:
        return tr("Orbital");
      case C_Energy:
        return tr("Energy");
      case C_Status:
        return tr("Status");
      default:
      case COUNT:
      return QVariant();
        }
    }
    else
      return QString::number(section + 1);
  }

  QModelIndex OrbitalTableModel::HOMO() const
  {
    for (int i = 0; i < m_orbitals.size(); i++) {
      if (m_orbitals.at(i).description == tr("HOMO", "Highest Occupied MO"))
        return index(i, 0);
    }
    return QModelIndex();
  }

  QModelIndex OrbitalTableModel::LUMO() const
  {
    for (int i = 0; i < m_orbitals.size(); i++) {
      if (m_orbitals.at(i).description == tr("LUMO", "Lowest Unoccupied MO"))
        return index(i, 0);
    }
    return QModelIndex();
  }


  bool OrbitalTableModel::setOrbital(const Orbital &orbital)
  {
    int index = orbital.index;

    if (index + 1 > m_orbitals.size()) {
      // Construct empty orbital:
      Orbital orb;
      orb.energy = 0;
      orb.index = -1;
      orb.description = "";
      orb.queueEntry = 0;
      orb.min = 0;
      orb.max = 0;
      orb.current = 0;

      beginInsertRows(QModelIndex(), m_orbitals.size(), index);
      for (int i = 0; i <= index - m_orbitals.size() + 1; i++) {
        m_orbitals.append(orb);
      }
      endInsertRows();
    }

    m_orbitals.replace(index, orbital);

    return true;
  }

  bool OrbitalTableModel::clearOrbitals()
  {
    beginRemoveRows(QModelIndex(), 0, m_orbitals.size() - 1);
    m_orbitals.clear();
    endRemoveRows();
    return true;
  }


} // namespace Avogadro

