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

#ifndef KPLOTOBJECT_H
#define KPLOTOBJECT_H

// Removed from avogadro
// #include <kdeui_export.h>

#include <QtCore/QString>
#include <QtGui/QColor>

class QBrush;
class QPainter;
class QPen;
class QPointF;
class KPlotWidget;
class KPlotPoint;

/**
 * @class KPlotObject
 * @short Encapsulates a data set to be plotted in a KPlotWidget.
 *
 * Think of a KPlotObject as a set of data displayed as a group in the plot.
 * Each KPlotObject consists of a list of KPlotPoints, a "type" controlling 
 * how the data points are displayed (some combination of Points, Lines, or
 * Bars), a color, and a size. There is also a parameter which controls the
 * shape of the points used to display the KPlotObject.
 *
 * @note KPlotObject will take care of the points added to it, so when clearing
 * the points list (eg with clearPoints()) any previous reference to a KPlotPoint
 * already added to a KPlotObject will be invalid.
 *
 * @author Jason Harris
 * @version 1.1
 */
// avogadro change
//class KDEUI_EXPORT KPlotObject{
class KPlotObject{
public:
    /**
     * The type classification of the KPlotObject.
     *
     * These are bitmask values that can be OR'd together, so that a set
     * of points can be represented in the plot in multiple ways.
     *
     * @note points should be added in order of increasing x-coordinate
     * when using Bars.
     */
    enum PlotType
    {
        UnknownType = 0,
        Points = 1,       ///< each KPlotPoint is represented with a drawn point
        Lines = 2,        ///< each KPlotPoint is connected with a line
        Bars = 4          ///< each KPlotPoint is shown as a vertical bar
    };
    Q_DECLARE_FLAGS( PlotTypes, PlotType )

    /**
     * The available shape styles for plotted points.
     */
    enum PointStyle
    {
        NoPoints = 0,
        Circle = 1,
        Letter = 2,
        Triangle = 3,
        Square = 4,
        Pentagon = 5,
        Hexagon = 6,
        Asterisk = 7,
        Star = 8,
        UnknwonPoint
    };

    /**
     * Constructor.
     * @param color The color for plotting this object. By default this sets
     * the color for Points, Lines and Bars, but there are functions to
     * override any of these.
     * @param otype the PlotType for this object (Points, Lines or Bars)
     * @param size the size to use for plotted points, in pixels
     * @param ps The PointStyle describing the shape for plotted points
     */
    explicit KPlotObject( const QColor &color = Qt::white, PlotType otype = Points, double size = 2, PointStyle ps = Circle );

    /**
     * Destructor.
     */
    ~KPlotObject();

    /**
     * @return the plot flags of the object
     */
    PlotTypes plotTypes() const;

    /**
     * Set whether points will be drawn for this object
     * @param b if true, points will be drawn
     */
    void setShowPoints( bool b );

    /**
     * Set whether lines will be drawn for this object
     * @param b if true, lines will be drawn
     */
    void setShowLines( bool b );

    /**
     * Set whether bars will be drawn for this object
     * @param b if true, bars will be drawn
     */
    void setShowBars( bool b );

    /**
     * @return the size of the plotted points in this object, in pixels
     */
    double size() const;

    /**
     * Set the size for plotted points in this object, in pixels
     * @param s the new size
     */
    void setSize( double s );

    /**
     * @return the style used for drawing the points in this object
     */
    PointStyle pointStyle() const;

    /**
     * Set a new style for drawing the points in this object
     * @param p the new style
     */
    void setPointStyle( PointStyle p );

    /**
     * @return the default pen for this Object.
     * If no other pens are set, this pen will be used for 
     * points, lines, bars and labels (this pen is always used for points).
     */
    const QPen& pen() const;

    /**
     * Set the default pen for this object
     * @p The pen to use
     */
    void setPen( const QPen &p );
    
    /**
     * @return the pen to use for drawing lines for this Object.
     */
    const QPen& linePen() const;

    /**
     * Set the pen to use for drawing lines for this object
     * @p The pen to use
     */
    void setLinePen( const QPen &p );
    
    /**
     * @return the pen to use for drawing bars for this Object.
     */
    const QPen& barPen() const;

    /**
     * Set the pen to use for drawing bars for this object
     * @p The pen to use
     */
    void setBarPen( const QPen &p );
    
    /**
     * @return the pen to use for drawing labels for this Object.
     */
    const QPen& labelPen() const;

    /**
     * Set the pen to use for labels for this object
     * @p The pen to use
     */
    void setLabelPen( const QPen &p );
    
    /**
     * @return the default Brush to use for this Object.
     */
    const QBrush brush() const;

    /**
     * Set the default brush to use for this object
     * @b The brush to use
     */
    void setBrush( const QBrush &b );

    /**
     * @return the brush to use for filling bars for this Object.
     */
    const QBrush barBrush() const;

    /**
     * Set the brush to use for drawing bars for this object
     * @b The brush to use
     */
    void setBarBrush( const QBrush &b );

    /**
     * @return the list of KPlotPoints that make up this object
     */
    QList< KPlotPoint* > points() const;

    /**
     * Add a point to the object's list of points, using input data to construct a KPlotPoint.
     * @param p the QPointF to add.
     * @param label the optional text label for this point
     * @param barWidth the width of the bar, if this object is to be drawn with bars
     * @note if @param barWidth is left at its default value of 0.0, then the width will be 
		 * automatically set to the distance between this point and the one to its right.
     */
    void addPoint( const QPointF &p, const QString &label = QString(), double barWidth = 0.0 );

    /**
     * Add a given KPlotPoint to the object's list of points.
     * @overload
     * @param p pointer to the KPlotPoint to add.
     */
    void addPoint( KPlotPoint *p );

    /**
     * Add a point to the object's list of points, using input data to construct a KPlotPoint.
     * @overload
     * @param x the X-coordinate of the point to add.
     * @param y the Y-coordinate of the point to add.
     * @param label the optional text label
     * @param barWidth the width of the bar, if this object is to be drawn with bars
     * @note if @param barWidth is left at its default value of 0.0, then the width will be 
		 * automatically set to the distance between this point and the one to its right.
     */
    void addPoint( double x, double y, const QString &label = QString(), double barWidth = 0.0 );

    /**
     * Remove the QPointF at position index from the list of points
     * @param index the index of the point to be removed.
     */
    void removePoint( int index );

    /**
     * Remove and destroy the points of this object
     */
    void clearPoints();

    /**
     * Draw this KPlotObject on the given QPainter
     * @param p The QPainter to draw on
     * @param pw the KPlotWidget to draw on (this is needed 
     * for the KPlotWidget::mapToWidget() function)
     */
    void draw( QPainter *p, KPlotWidget *pw );

private:
    class Private;
    Private * const d;

    Q_DISABLE_COPY( KPlotObject )
};
Q_DECLARE_OPERATORS_FOR_FLAGS( KPlotObject::PlotTypes )

#endif
