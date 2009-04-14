/**********************************************************************
  PlotObject -- Part of the Avogadro 2D plotting interface

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

#include "plotobject.h"

#include <QtAlgorithms>
#include <QPainter>

#include <qdebug.h>

#include "plotpoint.h"
#include "plotwidget.h"

namespace Avogadro {

  class PlotObject::Private
  {
  public:
    Private( PlotObject * qq )
      : q( qq )
    {
    }

    ~Private()
    {
      qDeleteAll( pList );
    }

    PlotObject *q;

    QList<PlotPoint*> pList;
    PlotTypes type;
    PointStyle pointStyle;
    double size;
    QPen pen, linePen, barPen, labelPen;
    QBrush brush, barBrush;
  };

  PlotObject::PlotObject( const QColor &c, PlotType t, double size, PointStyle ps )
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

  PlotObject::~PlotObject()
  {
    delete d;
  }

  PlotObject::PlotTypes PlotObject::plotTypes() const
  {
    return d->type;
  }

  void PlotObject::setShowPoints( bool b )
  {
    if ( b )
      {
        d->type |= PlotObject::Points;
      }
    else
      {
        d->type &= ~PlotObject::Points;
      }
  }

  void PlotObject::setShowLines( bool b )
  {
    if ( b )
      {
        d->type |= PlotObject::Lines;
      }
    else
      {
        d->type &= ~PlotObject::Lines;
      }
  }

  void PlotObject::setShowBars( bool b )
  {
    if ( b )
      {
        d->type |= PlotObject::Bars;
      }
    else
      {
        d->type &= ~PlotObject::Bars;
      }
  }

  double PlotObject::size() const
  {
    return d->size;
  }

  void PlotObject::setSize( double s )
  {
    d->size = s;
  }

  PlotObject::PointStyle PlotObject::pointStyle() const
  {
    return d->pointStyle;
  }

  void PlotObject::setPointStyle( PointStyle p )
  {
    d->pointStyle = p;
  }

  const QPen& PlotObject::pen() const
  {
    return d->pen;
  }

  void PlotObject::setPen( const QPen &p )
  {
    d->pen = p;
  }

  const QPen& PlotObject::linePen() const
  {
    return d->linePen;
  }

  void PlotObject::setLinePen( const QPen &p )
  {
    d->linePen = p;
  }

  const QPen& PlotObject::barPen() const
  {
    return d->barPen;
  }

  void PlotObject::setBarPen( const QPen &p )
  {
    d->barPen = p;
  }

  const QPen& PlotObject::labelPen() const
  {
    return d->labelPen;
  }

  void PlotObject::setLabelPen( const QPen &p )
  {
    d->labelPen = p;
  }

  const QBrush PlotObject::brush() const
  {
    return d->brush;
  }

  void PlotObject::setBrush( const QBrush &b )
  {
    d->brush = b;
  }

  const QBrush PlotObject::barBrush() const
  {
    return d->barBrush;
  }

  void PlotObject::setBarBrush( const QBrush &b )
  {
    d->barBrush = b;
  }

  QList< PlotPoint* > PlotObject::points() const
  {
    return d->pList;
  }

  void PlotObject::addPoint( const QPointF &p, const QString &label, double barWidth )
  {
    addPoint( new PlotPoint( p.x(), p.y(), label, barWidth ) );
  }

  void PlotObject::addPoint( PlotPoint *p )
  {
    if ( !p )
      return;
    d->pList.append( p );
  }

  void PlotObject::addPoint( double x, double y, const QString &label, double barWidth )
  {
    addPoint( new PlotPoint( x, y, label, barWidth ) );
  }

  void PlotObject::removePoint( int index ) {
    if ( ( index < 0 ) || ( index >= d->pList.count() ) ) {
      qWarning() << "PlotObject::removePoint(): index " << index << " out of range!";
      return;
    }

    d->pList.removeAt( index );
  }

  void PlotObject::clearPoints()
  {
    qDeleteAll( d->pList );
    d->pList.clear();
  }

  void PlotObject::draw( QPainter *painter, PlotWidget *pw ) {
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

      foreach ( PlotPoint *pp, d->pList ) {
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

      foreach( PlotPoint *pp, d->pList ) {
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

    foreach ( PlotPoint *pp, d->pList ) {
      QPoint q = pw->mapToWidget( pp->position() ).toPoint();
      if ( pw->pixRect().contains(q, false) && ! pp->label().isEmpty() ) {
	pw->placeLabel( painter, pp );
      }
    }

  }

  void PlotObject::drawImage( QPainter *painter, QRect *pixRect, QRectF *dataRect) {
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
        
        QPointF sp1 (pixRect->left() + pixRect->width() * ( p1.x() - dataRect->x() ) / dataRect->width(),
                     pixRect->top() + pixRect->height() * ( dataRect->y() + dataRect->height() - p1.y() ) / dataRect->height());
        QPointF sp2 (pixRect->left() + pixRect->width() * ( p2.x() - dataRect->x() ) / dataRect->width(),
                     pixRect->top() + pixRect->height() * ( dataRect->y() + dataRect->height() - p2.y() ) / dataRect->height());

	QRectF barRect = QRectF( sp1.x(), sp1.y(), sp2.x()-sp1.x(), sp2.y()-sp1.y() ).normalized();
	painter->drawRect( barRect );
      }
    }
    
    //Draw lines:
    if ( d->type & Lines ) {
      painter->setPen( linePen() );

      QPointF Previous = QPointF();  //Initialize to null

      foreach ( PlotPoint *pp, d->pList ) {
	//q is the position of the point in screen pixel coordinates
        QPointF ppp = pp->position();
        QPointF q (pixRect->left() + pixRect->width() * ( ppp.x() - dataRect->x() ) / dataRect->width(),
                   pixRect->top() + pixRect->height() * ( dataRect->y() + dataRect->height() - ppp.y() ) / dataRect->height());

	if ( ! Previous.isNull() ) {
	  painter->drawLine( Previous, q );
        }
            
	Previous = q;
      }
    }

    //Draw points:
    if ( d->type & Points ) {

      foreach( PlotPoint *pp, d->pList ) {
	//q is the position of the point in screen pixel coordinates
        QPointF q (pixRect->left() + pixRect->width() * ( pp->position().x() - dataRect->x() ) / dataRect->width(),
                   pixRect->top() + pixRect->height() * ( dataRect->y() + dataRect->height() - pp->position().y() ) / dataRect->height());
	if ( pixRect->contains( q.toPoint(), false ) ) {
	  double x1 = q.x() - size();
	  double y1 = q.y() - size();
	  QRectF qr = QRectF( x1, y1, 2*size(), 2*size() );
    
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
  }
}
