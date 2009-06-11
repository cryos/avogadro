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
   * data. This class allows both formatting and numeric sort for floating-point data in tables.
   */
  class A_EXPORT NumberTableItem : public QTableWidgetItem
  {
  public:
    /**
     * Constructor
     * @sa setFormat()
     * @param number The data for this NumberTableItem
     * @param fieldWidth Represents the minimum amount of space the formatted string will occupy
     * @param format The format for display. See http://doc.trolltech.com/4.5/qstring.html#argument-formats
     * @param precision The number of digits of precision to display after the decimal point
     */
    explicit NumberTableItem(double number, int fieldWidth = 0, char format='g', int precision = -1);

    /**
     * Destructor
     */
    ~NumberTableItem();
    
    /**
     * @return The data for this table item. This will return a formatted string for Qt::DisplayRole
     */
    virtual QVariant data(int role) const;

    /**
     * Set the data to value
     * @param role The role used for the data
     * @param value If value is a floating-point number, this will be used. Otherwise QTableWidgetItem is called.
     */
    virtual void setData(int role, const QVariant &value);
    
    /**
     * Set the format used for display. Uses localized QString::arg() for formatting
     * @sa QString::arg
     * @param fieldWidth Represents the minimum amount of space the formatted string will occupy
     * @param format The format for display. See http://doc.trolltech.com/4.5/qstring.html#argument-formats
     * @param precision The number of digits of precision to display after the decimal point
     */
    virtual void setFormat(int fieldWidth = 0, char format='g', int precision = -1);
    
    /**
     * Comparison operator for sorting
     * @param other Another table item for comparison
     * @return whether this item is less than @p other based on numeric comparison
     */
    bool operator<(const NumberTableItem &other) const;
    
  private:
    double m_data;
    int m_fieldWidth;
    char m_format;
    int m_precision;
  };

} // end namespace Avogadro

#endif
