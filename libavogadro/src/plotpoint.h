/**********************************************************************
  PlotPoint -- Part of the Avogadro 2D plotting interface

  Copyright (C) 2003 Jason Harris <kstars@30doradus.org> (KDE)
  Copyright (C) 2008 David Lonie <loniedavid@gmail.com> (Avogadro)

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.openmolecules.net/>

  This file is based on KPlotWidget from the KDE library. For more
  information see <http://www.kde.org/>

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

#ifndef PLOTPOINT_H
#define PLOTPOINT_H

#include <avogadro/global.h>

#include <QtCore/QString>

class QPointF;

namespace Avogadro {

  /**
   * @class PlotPoint plotpoint.h <avogadro/plotpoint.h>
   * @short Encapsulates a point in the plot.
   *
   * A PlotPoint consists of X and Y coordinates (in Data units),
   * an optional label string, and an optional bar-width,
   * The bar-width is only used for plots of type PlotObject::Bars,
   * and it allows the width of each bar to be set manually.  If
   * bar-widths are omitted, then the widths will be set automatically,
   * based on the halfway-mark between adjacent points.
   */
  class A_EXPORT PlotPoint {
  public:
    /**
     * Default constructor.
     */
    explicit PlotPoint();

    /**
     * Constructor.  Sets the PlotPoint according to the given arguments
     * @param x the X-position for the point, in Data units
     * @param y the Y-position for the point, in Data units
     * @param label the label string for the point.  If the string
     * is defined, the point will be labeled in the plot.
     * @param width the bar width to use for this point (only used for
     * plots of type PlotObject::Bars)
     */
    PlotPoint( double x, double y, const QString &label = QString(), double width = 0.0 );

    /**
     * Constructor.  Sets the PlotPoint according to the given arguments
     * @param p the position for the point, in Data units
     * @param label the label string for the point.  If the string
     * is defined, the point will be labeled in the plot.
     * @param width the bar width to use for this point (only used for
     * plots of type PlotObject::Bars)
     */
    explicit PlotPoint( const QPointF &p, const QString &label = QString(), double width = 0.0 );

    /**
     * Destructor
     */
    ~PlotPoint();

    /**
     * @return the position of the point, in data units
     */
    QPointF position() const;

    /**
     * Set the position of the point, in data units
     * @param pos the new position for the point.
     */
    void setPosition( const QPointF &pos );

    /**
     * @return the X-position of the point, in data units
     */
    double x() const;

    /**
     * Set the X-position of the point, in Data units
     */
    void setX( double x );

    /**
     * @return the Y-position of the point, in data units
     */
    double y() const;

    /**
     * Set the Y-position of the point, in Data units
     */
    void setY( double y );

    /**
     * @return the label for the point
     */
    QString label() const;

    /**
     * Set the label for the point
     */
    void setLabel( const QString &label );

    /**
     * @return the bar-width for the point
     */
    double barWidth() const;

    /**
     * Set the bar-width for the point
     */
    void setBarWidth( double w );

  private:
    class Private;
    Private * const d;

    Q_DISABLE_COPY( PlotPoint )
  };

}
#endif
