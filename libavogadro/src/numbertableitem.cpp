/**********************************************************************
  NumberTableItem - Provides a QTableWidgetItem capable of number sorting

  Copyright (C) 2009 Geoffrey Hutchison

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

#include "numbertableitem.h"

#include <QObject>

namespace Avogadro {

  NumberTableItem::NumberTableItem(double number, int fieldWidth, char format, int precision):
    m_data(number), m_fieldWidth(fieldWidth), m_format(format), m_precision(precision)
  {
    // We set the parent class too, since it will handle "extra" roles
    // (i.e., beyond EditRole or DisplayRole
    QTableWidgetItem::setData(Qt::DisplayRole, number);
  }

  NumberTableItem::~NumberTableItem()
  {
  }
    
  QVariant NumberTableItem::data(int role) const
  {
    if (role == Qt::EditRole)
      return QVariant(m_data);
    else if (role == Qt::DisplayRole)
      return QString("%L1").arg(m_data, m_fieldWidth, m_format, m_precision);
    else
      return QTableWidgetItem::data(role);
  }

  void NumberTableItem::setData(int role, const QVariant &value)
  {
    if (value.type() == QVariant::Double)
      m_data = value.toDouble();

    // always pass it along to QTableWidget for auxiliary roles
    QTableWidgetItem::setData(role, value);
  }
    
  void NumberTableItem::setFormat(int fieldWidth, char format, int precision)
  {
    m_fieldWidth = fieldWidth;
    m_format = format;
    m_precision = precision;
  }
    
  bool NumberTableItem::operator<(const NumberTableItem &other) const
  {
    return m_data < other.m_data;
  }

} // end namespace Avogadro

#include "numbertableitem.moc"
