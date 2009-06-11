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

#ifndef NUMBERTABLEITEM_H
#define NUMBERTABLEITEM_H

#include <QTableWidgetItem>

#include <avogadro/global.h>

namespace Avogadro {

  /**
   * @class NumberTableItem
   * @brief Provides for number formatting and sorting in a QTableWidgetItem
   * @author Geoffrey Hutchison
   *
   * The QTableWidget class provides the capability for sorting data by columns,
   * but the sort is performed based on string comparisons. This is confusing for numeric
   * data. This class allows both formatting and numeric sort for float-point data in tables.
   */
  class A_EXPORT NumberTableItem : public QTableWidgetItem
  {
  public:
    explicit NumberTableItem(double number, int fieldWidth = 0, char format='g', int precision = -1);
    ~NumberTableItem();
    
    virtual QVariant data(int role) const;
    virtual void setData(int role, const QVariant &value);
    
    virtual void setFormat(int fieldWidth = 0, char format='g', int precision = -1);
    
    bool operator<(const NumberTableItem &other) const;
    
  private:
    double m_data;
    int m_fieldWidth;
    char m_format;
    int m_precision;
  };

} // end namespace Avogadro

#endif
