/**********************************************************************
  ColorButton - Button widget to display the current selected color
                and bring up the color picker when clicked

  Copyright (C) 2008 Geoffrey Hutchison

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

#ifndef __COLORBUTTON_H
#define __COLORBUTTON_H

#include <QAbstractButton>
#include <QColor>

  /**
   * @class ColorButton colorbutton.h <avogadro/colorbutton.h>
   * @author Geoffrey Hutchison
   * @brief A button to show the current color and bring up the QColorDialog.
   *
   * This class implements a QAbstractButton to display a colored rectangle.
   * When clicked by the user, it brings up a color picker to select a new
   * color.
   *
   * The widget has a default minimium size of 35x20 pixels.
   */

class ColorButton : public QAbstractButton
{
 Q_OBJECT

public:
    ColorButton(QWidget *parent = 0);
    explicit ColorButton(const QColor& initial, QWidget *parent = 0);

    /** 
     *  Redraw the widget (i.e., refresh the colored rectange)
     */
    void paintEvent(QPaintEvent *);

    /**
     * @param color the new color to be used
     */
    void setColor(const QColor& color);
    
    /**
     * @return the current color
     */ 
    QColor color() const;

signals:
    /**
     *  emit any time the color is changed, either by a user or by setColor()
     */
    void colorChanged(QColor);

public slots:
    /**
     * Call for a change in the current color
     */
   void changeColor();

protected:
   QColor m_color; //!< The current color
};

#endif
