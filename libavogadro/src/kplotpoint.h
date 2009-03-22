/*  -*- C++ -*-
    This file is part of the KDE libraries
    Copyright (C) 2003 Jason Harris <kstars@30doradus.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef KPLOTPOINT_H
#define KPLOTPOINT_H

// Removed from avogadro
// #include <kdeui_export.h>

#include <QtCore/QString>

class QPointF;

/**
 * @class KPlotPoint
 * @short Encapsulates a point in the plot.
 * A KPlotPoint consists of X and Y coordinates (in Data units),
 * an optional label string, and an optional bar-width,
 * The bar-width is only used for plots of type KPlotObject::Bars,
 * and it allows the width of each bar to be set manually.  If 
 * bar-widths are omitted, then the widths will be set automatically, 
 * based on the halfway-mark between adjacent points.
 */
// avogadro change
//class KDEUI_EXPORT KPlotPoint {
class KPlotPoint {
public:
    /**
     * Default constructor.
     */
    explicit KPlotPoint();

    /**
     * Constructor.  Sets the KPlotPoint according to the given arguments
     * @param x the X-position for the point, in Data units
     * @param y the Y-position for the point, in Data units
     * @param label the label string for the point.  If the string 
     * is defined, the point will be labeled in the plot.
     * @param width the bar width to use for this point (only used for
     * plots of type KPlotObject::Bars)
     */
    KPlotPoint( double x, double y, const QString &label = QString(), double width = 0.0 );

    /**
     * Constructor.  Sets the KPlotPoint according to the given arguments
     * @param p the position for the point, in Data units
     * @param label the label string for the point.  If the string 
     * is defined, the point will be labeled in the plot.
     * @param width the bar width to use for this point (only used for
     * plots of type KPlotObject::Bars)
     */
    explicit KPlotPoint( const QPointF &p, const QString &label = QString(), double width = 0.0 );

    /**
     * Destructor
     */
    ~KPlotPoint();

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

    Q_DISABLE_COPY( KPlotPoint )
};

#endif
