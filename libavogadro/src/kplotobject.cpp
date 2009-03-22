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

#include "kplotobject.h"

#include <QtAlgorithms>
#include <QPainter>

// avogadro change:
//#include <kdebug.h>
#include <qdebug.h>

#include "kplotpoint.h"
#include "kplotwidget.h"

class KPlotObject::Private
{
    public:
        Private( KPlotObject * qq )
            : q( qq )
        {
        }

        ~Private()
        {
            qDeleteAll( pList );
        }

        KPlotObject *q;

        QList<KPlotPoint*> pList;
        PlotTypes type;
        PointStyle pointStyle;
        double size;
        QPen pen, linePen, barPen, labelPen;
        QBrush brush, barBrush;
};

KPlotObject::KPlotObject( const QColor &c, PlotType t, double size, PointStyle ps )
    : d( new Private( this ) )
{
    //By default, all pens and brushes are set to the given color
    setBrush( c );
    setBarBrush( c );
    setPen( QPen( brush(), 1 ) );
    setLinePen( pen() );
    setBarPen( pen() );
    setLabelPen( pen() );

    d->type |= t;
    setSize( size );
    setPointStyle( ps );
}

KPlotObject::~KPlotObject()
{
    delete d;
}

KPlotObject::PlotTypes KPlotObject::plotTypes() const
{
    return d->type;
}

void KPlotObject::setShowPoints( bool b )
{
    if ( b )
    {
        d->type |= KPlotObject::Points;
    }
    else
    {
        d->type &= ~KPlotObject::Points;
    }
}

void KPlotObject::setShowLines( bool b )
{
    if ( b )
    {
        d->type |= KPlotObject::Lines;
    }
    else
    {
        d->type &= ~KPlotObject::Lines;
    }
}

void KPlotObject::setShowBars( bool b )
{
    if ( b )
    {
        d->type |= KPlotObject::Bars;
    }
    else
    {
        d->type &= ~KPlotObject::Bars;
    }
}

double KPlotObject::size() const
{
    return d->size;
}

void KPlotObject::setSize( double s )
{
    d->size = s;
}

KPlotObject::PointStyle KPlotObject::pointStyle() const
{
    return d->pointStyle;
}

void KPlotObject::setPointStyle( PointStyle p )
{
    d->pointStyle = p;
}

const QPen& KPlotObject::pen() const
{
    return d->pen;
}

void KPlotObject::setPen( const QPen &p )
{
    d->pen = p;
}

const QPen& KPlotObject::linePen() const
{
    return d->linePen;
}

void KPlotObject::setLinePen( const QPen &p )
{
    d->linePen = p;
}

const QPen& KPlotObject::barPen() const
{
    return d->barPen;
}

void KPlotObject::setBarPen( const QPen &p )
{
    d->barPen = p;
}

const QPen& KPlotObject::labelPen() const
{
    return d->labelPen;
}

void KPlotObject::setLabelPen( const QPen &p )
{
    d->labelPen = p;
}

const QBrush KPlotObject::brush() const
{
    return d->brush;
}

void KPlotObject::setBrush( const QBrush &b )
{
    d->brush = b;
}

const QBrush KPlotObject::barBrush() const
{
    return d->barBrush;
}

void KPlotObject::setBarBrush( const QBrush &b )
{
    d->barBrush = b;
}

QList< KPlotPoint* > KPlotObject::points() const
{
    return d->pList;
}

void KPlotObject::addPoint( const QPointF &p, const QString &label, double barWidth )
{
			addPoint( new KPlotPoint( p.x(), p.y(), label, barWidth ) );
}

void KPlotObject::addPoint( KPlotPoint *p )
{
    if ( !p )
        return;
    d->pList.append( p );
}

void KPlotObject::addPoint( double x, double y, const QString &label, double barWidth )
{
	addPoint( new KPlotPoint( x, y, label, barWidth ) );
}

void KPlotObject::removePoint( int index ) {
    if ( ( index < 0 ) || ( index >= d->pList.count() ) ) {
	// avogadro change
	// kWarning() << "KPlotObject::removePoint(): index " << index << " out of range!";
	qWarning() << "KPlotObject::removePoint(): index " << index << " out of range!";
        return;
    }

    d->pList.removeAt( index );
}

void KPlotObject::clearPoints()
{
    qDeleteAll( d->pList );
    d->pList.clear();
}

void KPlotObject::draw( QPainter *painter, KPlotWidget *pw ) {
    //Order of drawing determines z-distance: Bars in the back, then lines, 
    //then points, then labels.

    if ( d->type & Bars ) {
        painter->setPen( barPen() );
        painter->setBrush( barBrush() );

        for ( int i=0; i<d->pList.size(); ++i ) {
            double w = 0;
            if ( d->pList[i]->barWidth() == 0.0 ) {
                if ( i<d->pList.size()-1 ) 
                    w = d->pList[i+1]->x() - d->pList[i]->x();
                //For the last bin, we'll just keep the previous width

            } else {
                w = d->pList[i]->barWidth();
            }

            QPointF pp = d->pList[i]->position();
            QPointF p1( pp.x() - 0.5*w, 0.0 );
            QPointF p2( pp.x() + 0.5*w, pp.y() );
            QPointF sp1 = pw->mapToWidget( p1 );
            QPointF sp2 = pw->mapToWidget( p2 );

            QRectF barRect = QRectF( sp1.x(), sp1.y(), sp2.x()-sp1.x(), sp2.y()-sp1.y() ).normalized();
            painter->drawRect( barRect );
            pw->maskRect( barRect, 0.25 );
        }
    }
    
    //Draw lines:
    if ( d->type & Lines ) {
        painter->setPen( linePen() );

        QPointF Previous = QPointF();  //Initialize to null

        foreach ( KPlotPoint *pp, d->pList ) {
            //q is the position of the point in screen pixel coordinates
            QPointF q = pw->mapToWidget( pp->position() );

            if ( ! Previous.isNull() ) {
                painter->drawLine( Previous, q );
                pw->maskAlongLine( Previous, q );
            }
            
            Previous = q;
        }
    }

    //Draw points:
    if ( d->type & Points ) {

        foreach( KPlotPoint *pp, d->pList ) {
            //q is the position of the point in screen pixel coordinates
            QPointF q = pw->mapToWidget( pp->position() );
            if ( pw->pixRect().contains( q.toPoint(), false ) ) {
                double x1 = q.x() - size();
                double y1 = q.y() - size();
                QRectF qr = QRectF( x1, y1, 2*size(), 2*size() );
    
                //Mask out this rect in the plot for label avoidance
                pw->maskRect( qr, 2.0 );
    
                painter->setPen( pen() );
                painter->setBrush( brush() );
    
                switch ( pointStyle() ) {
                case Circle:
                    painter->drawEllipse( qr );
                    break;
    
                case Letter:
                    painter->drawText( qr, Qt::AlignCenter, pp->label().left(1) );
                    break;
    
                case Triangle:
                    {
                        QPolygonF tri;
                        tri << QPointF( q.x() - size(), q.y() + size() ) 
                                << QPointF( q.x(), q.y() - size() ) 
                                << QPointF( q.x() + size(), q.y() + size() );
                        painter->drawPolygon( tri );
                        break;
                    }
    
                case Square:
                    painter->drawRect( qr );
                    break;
    
                case Pentagon:
                    {
                        QPolygonF pent;
                        pent << QPointF( q.x(), q.y() - size() ) 
                                << QPointF( q.x() + size(), q.y() - 0.309*size() )
                                << QPointF( q.x() + 0.588*size(), q.y() + size() )
                                << QPointF( q.x() - 0.588*size(), q.y() + size() )
                                << QPointF( q.x() - size(), q.y() - 0.309*size() );
                        painter->drawPolygon( pent );
                        break;
                    }
    
                case Hexagon:
                    {
                        QPolygonF hex;
                        hex << QPointF( q.x(), q.y() + size() ) 
                                << QPointF( q.x() + size(), q.y() + 0.5*size() )
                                << QPointF( q.x() + size(), q.y() - 0.5*size() )
                                << QPointF( q.x(), q.y() - size() )
                                << QPointF( q.x() - size(), q.y() + 0.5*size() )
                                << QPointF( q.x() - size(), q.y() - 0.5*size() );
                        painter->drawPolygon( hex );
                        break;
                    }
    
                case Asterisk:
                    painter->drawLine( q, QPointF( q.x(), q.y() + size() ) );
                    painter->drawLine( q, QPointF( q.x() + size(), q.y() + 0.5*size() ) );
                    painter->drawLine( q, QPointF( q.x() + size(), q.y() - 0.5*size() ) );
                    painter->drawLine( q, QPointF( q.x(), q.y() - size() ) );
                    painter->drawLine( q, QPointF( q.x() - size(), q.y() + 0.5*size() ) );
                    painter->drawLine( q, QPointF( q.x() - size(), q.y() - 0.5*size() ) );
                    break;
    
                case Star:
                    {
                        QPolygonF star;
                        star << QPointF( q.x(), q.y() - size() ) 
                                << QPointF( q.x() + 0.2245*size(), q.y() - 0.309*size() )
                                << QPointF( q.x() + size(), q.y() - 0.309*size() )
                                << QPointF( q.x() + 0.363*size(), q.y() + 0.118*size() )
                                << QPointF( q.x() + 0.588*size(), q.y() + size() )
                                << QPointF( q.x(), q.y() + 0.382*size() )
                                << QPointF( q.x() - 0.588*size(), q.y() + size() )
                                << QPointF( q.x() - 0.363*size(), q.y() + 0.118*size() )
                                << QPointF( q.x() - size(), q.y() - 0.309*size() )
                                << QPointF( q.x() - 0.2245*size(), q.y() - 0.309*size() );
                        painter->drawPolygon( star );
                        break;
                    }
    
                default:
                    break;
                }
            }
        }
    }

    //Draw labels
    painter->setPen( labelPen() );

    foreach ( KPlotPoint *pp, d->pList ) {
        QPoint q = pw->mapToWidget( pp->position() ).toPoint();
        if ( pw->pixRect().contains(q, false) && ! pp->label().isEmpty() ) {
            pw->placeLabel( painter, pp );
        }
    }

}
