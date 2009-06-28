/**********************************************************************
  PlotWidget -- 2D plotting interface

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

#include "plotwidget.h"
#include "plotwidget.moc"

#include <math.h>
#include <QDebug>

#include <QtGui/QActionEvent>
#include <QHash>
#include <QPainter>
#include <QPixmap>
#include <QToolTip>
#include <QtAlgorithms>
#include <QFont>
#include <QLabel>
#include <QPalette>

#include "plotaxis.h"
#include "plotpoint.h"
#include "plotobject.h"

#define XPADDING 20
#define YPADDING 20
#define BIGTICKSIZE 10
#define SMALLTICKSIZE 4
#define TICKOFFSET 0

namespace Avogadro {

  class PlotWidget::Private
  {
  public:
    Private( PlotWidget *qq )
      : q( qq ),
        cBackground( Qt::black ), cForeground( Qt::white ), cGrid( Qt::gray ),
        showGrid( false ), showObjectToolTip( true ), useAntialias( false ),
        font( QFont() ), followingMouse(false)
    {
      // create the axes and setting their default properties
      PlotAxis *leftAxis = new PlotAxis();
      leftAxis->setTickLabelsShown( true );
      axes.insert( LeftAxis, leftAxis );
      PlotAxis *bottomAxis = new PlotAxis();
      bottomAxis->setTickLabelsShown( true );
      axes.insert( BottomAxis, bottomAxis );
      PlotAxis *rightAxis = new PlotAxis();
      axes.insert( RightAxis, rightAxis );
      PlotAxis *topAxis = new PlotAxis();
      axes.insert( TopAxis, topAxis );
      mousefollow = new PlotObject(Qt::blue, PlotObject::Points);
      mousefollow->setPointStyle(PlotObject::Circle);
      privateObjectList.append(mousefollow);
      // Selection should follow mousefollow for a more natural feel when selecting points
      selection = new PlotObject(Qt::yellow, PlotObject::Points);
      selection->setPointStyle(PlotObject::Circle);
      privateObjectList.append(selection);
    }

    ~Private()
    {
      qDeleteAll( objectList );
      qDeleteAll( axes );
      qDeleteAll( privateObjectList );
    }

    PlotWidget *q;

    void calcDataRectLimits( double x1, double x2, double y1, double y2 );
    /**
     * @return a value indicating how well the given rectangle is
     * avoiding masked regions in the plot.  A higher returned value
     * indicates that the rectangle is intersecting a larger portion
     * of the masked region, or a portion of the masked region which
     * is weighted higher.
     * @param r The rectangle to be tested
     */
    float rectCost( const QRectF &r ) const;

    //Colors
    QColor cBackground, cForeground, cGrid;
    //draw options
    bool showGrid : 1;
    bool showObjectToolTip : 1;
    bool useAntialias : 1;
    //padding
    int leftPadding, rightPadding, topPadding, bottomPadding;
    // hashmap with the axes we have
    QHash<Axis, PlotAxis*> axes;
    // Lists of PlotObjects
    QList<PlotObject*> objectList, privateObjectList;
    // Private PlotObjects
    PlotObject *selection, *mousefollow;
    // Limits of the plot area in data units
    QRectF dataRect, secondDataRect, defaultDataRect;
    // Limits of the plot area in pixel units
    QRect pixRect;
    //Array holding the mask of "used" regions of the plot
    QImage plotMask;
    //Font properties
    QFont font;
    //Whether to highlight the point nearest the mouse
    bool followingMouse;
  };

  PlotWidget::PlotWidget( QWidget * parent )
    : QFrame( parent ), d( new Private( this ) )
  {
    setAttribute( Qt::WA_OpaquePaintEvent );
    setAttribute( Qt::WA_NoSystemBackground );

    d->font.setPointSize(10);

    d->secondDataRect = QRectF(); //default: no secondary data rect
    // sets the default limits
    d->calcDataRectLimits( 0.0, 1.0, 0.0, 1.0 );

    setDefaultPaddings();

    setMinimumSize( 150, 150 );
    resize( minimumSizeHint() );
  }

  PlotWidget::~PlotWidget()
  {
    delete d;
  }

  QSize PlotWidget::minimumSizeHint() const
  {
    return QSize( 150, 150 );
  }

  QSize PlotWidget::sizeHint() const
  {
    return size();
  }

  void PlotWidget::setLimits( double x1, double x2, double y1, double y2 )
  {
    d->calcDataRectLimits( x1, x2, y1, y2 );
    update();
  }

  void PlotWidget::scaleLimits(PlotObject * po) {
    double xmin=0, xmax=0, ymin=0, ymax=0;
    if (po) {
      if (po->points().isEmpty()) return;
      xmin = xmax = po->points().first()->x();
      ymin = ymax = po->points().first()->y();
      
      foreach ( PlotPoint *pp, po->points() ) {
        if (pp->x() < xmin) xmin = pp->x();
        if (pp->x() > xmax) xmax = pp->x();
        if (pp->y() < ymin) ymin = pp->y();
        if (pp->y() > ymax) ymax = pp->y();
      }
    } else {
      // Initialize values with the first point of the first non-empty plot object
      foreach ( PlotObject *po, d->objectList ) {
        if (!po->points().isEmpty()) {
          xmin = xmax = po->points().first()->x();
          ymin = ymax = po->points().first()->y();
          break;
        }
      }

      foreach ( PlotObject *po, d->objectList ) {
        foreach ( PlotPoint *pp, po->points() ) {
          if (pp->x() < xmin) xmin = pp->x();
          if (pp->x() > xmax) xmax = pp->x();
          if (pp->y() < ymin) ymin = pp->y();
          if (pp->y() > ymax) ymax = pp->y();
        }
      }
    }
    setDefaultLimits( xmin, xmax, ymin, ymax );
  }

  void PlotWidget::setDefaultLimits( double x1, double x2, double y1, double y2 )
  {
    if ( fabs(x2 - x1) < 1.0e-3 ) {
      qWarning() << "x1 and x2 cannot be equal. Setting x2 = x1 + 1.0";
      x2 = x1 + 1.0;
    }
    if ( fabs(y2 - y1) < 1.0e-3) {
      qWarning() << "y1 and y2 cannot be equal. Setting y2 = y1 + 1.0";
      y2 = y1 + 1.0;
    }
    d->defaultDataRect = QRectF( x1, y1, x2 - x1, y2 - y1 );
    setLimits( x1, x2, y1, y2 );
  }

  void PlotWidget::unsetDefaultLimits()
  {
    if (!defaultDataRect().isNull()) {
      d->defaultDataRect = QRectF();
    }
  }

  void PlotWidget::Private::calcDataRectLimits( double x1, double x2, double y1, double y2 )
  {
    // Removed limit checking, since IR spectra need the x-axis to run
    // from 4000 -> 400, not the other way around.
    double XA1, XA2, YA1, YA2;
    XA1=x1; XA2=x2;
    YA1=y1; YA2=y2;

    if ( fabs(XA2 - XA1) < 1.0e-3 ) {
      qWarning() << "x1 and x2 cannot be equal. Setting x2 = x1 + 1.0";
      XA2 = XA1 + 1.0;
    }
    if ( fabs(YA2 - YA1) < 1.0e-3) {
      qWarning() << "y1 and y2 cannot be equal. Setting y2 = y1 + 1.0";
      YA2 = YA1 + 1.0;
    }
    dataRect = QRectF( XA1, YA1, XA2 - XA1, YA2 - YA1 );

    q->axis( LeftAxis )->setTickMarks( dataRect.y(), dataRect.height() );
    q->axis( BottomAxis )->setTickMarks( dataRect.x(), dataRect.width() );

    if ( secondDataRect.isNull() )
    {
      q->axis( RightAxis )->setTickMarks( dataRect.y(), dataRect.height() );
      q->axis( TopAxis )->setTickMarks( dataRect.x(), dataRect.width() );
    }
  }

  void PlotWidget::setSecondaryLimits( double x1, double x2, double y1, double y2 ) {
    // Again, removed limit checking. Just be careful with it ;)
    double XA1, XA2, YA1, YA2;
    XA1=x1; XA2=x2;
    YA1=y1; YA2=y2;

    if ( fabs(XA2 - XA1) < 1.0e-3 ) {
      qDebug() << "x1 and x2 cannot be equal. Setting x2 = x1 + 1.0";
      XA2 = XA1 + 1.0;
    }
    if ( fabs(YA2 - YA1) < 1.0e-3) {
      qDebug() << "y1 and y2 cannot be equal. Setting y2 = y1 + 1.0";
      YA2 = YA1 + 1.0;
    }
    d->secondDataRect = QRectF( XA1, YA1, XA2-XA1, YA2-YA1 );

    axis(RightAxis)->setTickMarks( d->secondDataRect.y(), d->secondDataRect.height() );
    axis(TopAxis)->setTickMarks( d->secondDataRect.x(), d->secondDataRect.width() );

    update();
  }

  void PlotWidget::clearSecondaryLimits() {
    d->secondDataRect = QRectF();
    axis(RightAxis)->setTickMarks( d->dataRect.y(), d->dataRect.height() );
    axis(TopAxis)->setTickMarks( d->dataRect.x(), d->dataRect.width() );

    update();
  }

  QRectF PlotWidget::dataRect() const
  {
    return d->dataRect;
  }

  QRectF PlotWidget::defaultDataRect() const
  {
    return d->defaultDataRect;
  }

  QRectF PlotWidget::secondaryDataRect() const
  {
    return d->secondDataRect;
  }

  void PlotWidget::selectPoint(PlotPoint* point)
  {
    // Need to just send x and y; otherwise Bad Things happen when the point is cleared...
    d->selection->addPoint(point->x(), point->y());
    update();
  }

  void PlotWidget::selectPoints(const QList<PlotPoint*> & points)
  {
    for (int i = 0; i < points.size(); i++)
      // Need to just send x and y; otherwise Bad Things happen when the point is cleared...
      d->selection->addPoint(points.at(i)->x(), points.at(i)->y());
    update();
  }

  void PlotWidget::clearAndSelectPoint(PlotPoint* point)
  {
    clearSelection();
    // Need to just send x and y; otherwise Bad Things happen when the point is cleared...
    d->selection->addPoint(point->x(), point->y());
    update();
  }

  void PlotWidget::clearAndSelectPoints(const QList<PlotPoint*> & points)
  {
    clearSelection();
    for (int i = 0; i < points.size(); i++)
      // Need to just send x and y; otherwise Bad Things happen when the point is cleared...
      d->selection->addPoint(points.at(i)->x(), points.at(i)->y());
    update();
  }

  void PlotWidget::clearSelection()
  {
    d->selection->clearPoints();
    update();
  }

  void PlotWidget::setPointFollowMouse(bool b)
  {
    setMouseTracking(b);
    d->followingMouse = b;
    update();
  }

  void PlotWidget::addPlotObject( PlotObject *object )
  {
    // skip null pointers
    if ( !object )
      return;
    d->objectList.append( object );
    update();
  }

  void PlotWidget::addPlotObjects( const QList< PlotObject* >& objects )
  {
    bool addedsome = false;
    foreach ( PlotObject *o, objects )
      {
        if ( !o )
          continue;
	d->objectList.append( o );
	addedsome = true;
      }
    if ( addedsome )
      update();
  }

  QList< PlotObject* > PlotWidget::plotObjects() const
  {
    return d->objectList;
  }

  void PlotWidget::removeAllPlotObjects()
  {
    if ( d->objectList.isEmpty() )
      return;

    qDeleteAll( d->objectList );
    d->objectList.clear();
    update();
  }

  void PlotWidget::resetPlotMask() {
    d->plotMask = QImage( pixRect().size(), QImage::Format_ARGB32 );
    QColor fillColor = Qt::black;
    fillColor.setAlpha( 128 );
    d->plotMask.fill( fillColor.rgb() );
  }

  void PlotWidget::resetPlot() {
    qDeleteAll( d->objectList );
    d->objectList.clear();
    clearSecondaryLimits();
    d->calcDataRectLimits( 0.0, 1.0, 0.0, 1.0 );
    PlotAxis *a = axis( RightAxis );
    a->setLabel( QString() );
    a->setTickLabelsShown( false );
    a = axis( TopAxis );
    a->setLabel( QString() );
    a->setTickLabelsShown( false );
    axis(PlotWidget::LeftAxis)->setLabel( QString() );
    axis(PlotWidget::BottomAxis)->setLabel( QString() );
    resetPlotMask();
  }

  void PlotWidget::replacePlotObject( int i, PlotObject *o )
  {
    // skip null pointers and invalid indexes
    if ( !o || i < 0 || i >= d->objectList.count() )
      return;
    d->objectList.replace( i, o );
    update();
  }

  QColor PlotWidget::backgroundColor() const
  {
    return d->cBackground;
  }

  QColor PlotWidget::foregroundColor() const
  {
    return d->cForeground;
  }

  QColor PlotWidget::gridColor() const
  {
    return d->cGrid;
  }

  void PlotWidget::setBackgroundColor( const QColor &bg ) {
    d->cBackground = bg;
    update();
  }

  void PlotWidget::setForegroundColor( const QColor &fg )
  {
    d->cForeground = fg;
    update();
  }

  void PlotWidget::setGridColor( const QColor &gc )
  {
    d->cGrid = gc;
    update();
  }

  void PlotWidget::setFontSize( int pointSize )
  {
    d->font.setPointSize( pointSize );
  }

  void PlotWidget::setFont( QFont font )
  {
    if (d->font != font) {
      d->font = font;
      update();
    }
  }

  QFont PlotWidget::getFont()
  {
    return d->font;
  }

  bool PlotWidget::isGridShown() const
  {
    return d->showGrid;
  }

  bool PlotWidget::isObjectToolTipShown() const
  {
    return d->showObjectToolTip;
  }

  bool PlotWidget::antialiasing() const
  {
    return d->useAntialias;
  }

  void PlotWidget::setAntialiasing( bool b )
  {
    d->useAntialias = b;
    update();
  }

  void PlotWidget::setShowGrid( bool show ) {
    d->showGrid = show;
    update();
  }

  void PlotWidget::setObjectToolTipShown( bool show )
  {
    d->showObjectToolTip = show;
  }


  PlotAxis* PlotWidget::axis( Axis type )
  {
    QHash<Axis, PlotAxis*>::Iterator it = d->axes.find( type );
    return it != d->axes.end() ? it.value() : 0;
  }

  const PlotAxis* PlotWidget::axis( Axis type ) const
  {
    QHash<Axis, PlotAxis*>::ConstIterator it = d->axes.constFind( type );
    return it != d->axes.constEnd() ? it.value() : 0;
  }

  QRect PlotWidget::pixRect() const
  {
    return d->pixRect;
  }

  QList<PlotPoint*> PlotWidget::pointsUnderPoint( const QPoint& p ) const {
    QList<PlotPoint*> pts;
    foreach ( PlotObject *po, d->objectList ) {
      foreach ( PlotPoint *pp, po->points() ) {
        if ( ( p - mapToWidget( pp->position() ).toPoint() ).manhattanLength() <= 4 )
          pts << pp;
      }
    }

    return pts;
  }

  PlotPoint* PlotWidget::pointNearestPoint( const QPoint& p ) const {
    PlotPoint* pt = NULL;
    double cur, distance = rect().width(); // Widget width as default
    foreach ( PlotObject *po, d->objectList ) {
      foreach ( PlotPoint *pp, po->points() ) {
        cur = ( p - mapToWidget( pp->position() ).toPoint() ).manhattanLength();
        if ( cur < distance ) {
          pt = pp;
          distance = cur;
        }
      }
    }

    if (pt)
      return pt;
    else
      return NULL;
  }

  bool PlotWidget::event( QEvent* e ) {
    if ( e->type() == QEvent::ToolTip ) {
      if ( d->showObjectToolTip )
        {
          QHelpEvent *he = static_cast<QHelpEvent*>( e );
          QList<PlotPoint*> pts = pointsUnderPoint( he->pos() - QPoint( leftPadding(), topPadding() ) - contentsRect().topLeft() );
          if ( pts.count() > 0 ) {
            QToolTip::showText( he->globalPos(), pts.front()->label(), this );
          }
        }
      e->accept();
      return true;
    }
    else
      return QFrame::event( e );
  }

  void PlotWidget::mouseMoveEvent(QMouseEvent *event)
  {
    if (event->buttons() & Qt::RightButton) {
      QPointF pixelDelta = event->posF() - mouseClickOrigin; // How far the mouse has moved in QFrame coords.
      QPointF unitPerPixel (-dataRect().width() / pixRect().width(), dataRect().height() / pixRect().height()); // get conversion factor
      QPointF unitDelta (pixelDelta.x() * unitPerPixel.x(), pixelDelta.y() * unitPerPixel.y()); // How far the mouse has moved in axis coords
      // New limits
      float newX1 = dataRect().x() + unitDelta.x();
      float newX2 = dataRect().x() + unitDelta.x() + dataRect().width();
      float newY1 = dataRect().y() + unitDelta.y();
      float newY2 = dataRect().y() + unitDelta.y() + dataRect().height();

      setLimits(newX1, newX2, newY1, newY2);// Update axis

      mouseClickOrigin = event->posF();
    }

    if (event->buttons() & Qt::MidButton) {
      zoomPosF = event->pos();
      update();
    }

    // "mouseover" events
    if (event->button() == Qt::NoButton) {
      QPointF p_data = mapFrameToData(event->posF());
      emit mouseOverPoint(p_data.x(), p_data.y());
    }

    // "Mouse follow" events
    if (event->button() == Qt::NoButton && d->objectList.size() > 0 && d->followingMouse) {
      QPointF pF 	= mapToWidget(mapFrameToData(event->pos()));
      QPoint p_widget 	( static_cast<int>(pF.x()), static_cast<int>(pF.y()));
      PlotPoint *p 	= pointNearestPoint(p_widget);
      PlotPoint *old;

      if (p) { // Make sure p isn't null...
        if (d->mousefollow->points().size() != 0) {
          old 	= d->mousefollow->at(0);
          d->mousefollow->clearPoints();
        }
        d->mousefollow->addPoint(p->x(), p->y());
        update();
      }
    }
  }

  void PlotWidget::mousePressEvent(QMouseEvent *event)
  {
    if (event->buttons() & Qt::RightButton) {
      mouseClickOrigin = event->posF();
    }
    if (event->buttons() & Qt::MidButton) {
      mouseClickOrigin = event->posF();
    }
    if (event->buttons() & Qt::LeftButton) {
      QPointF pF ( mapToWidget(mapFrameToData(event->pos())));
      QPoint p_widget ( static_cast<int>(pF.x()), static_cast<int>(pF.y()));
      QPointF p_data = mapFrameToData(event->posF());
      PlotPoint *p_near = pointNearestPoint(p_widget); 
      emit pointClicked(p_data.x(), p_data.y());
      emit pointClicked(pointsUnderPoint(p_widget));
      if (p_near) emit pointClicked(p_near);
    }
  }

  void PlotWidget::mouseDoubleClickEvent(QMouseEvent *event)
  {
    if ((event->buttons() & Qt::LeftButton) && !defaultDataRect().isNull()) {
      double x1 = defaultDataRect().x();
      double x2 = x1 + defaultDataRect().width();
      double y1 = defaultDataRect().y();
      double y2 = y1 + defaultDataRect().height();
      setLimits(x1, x2, y1, y2);
    }
  }

  void PlotWidget::mouseReleaseEvent(QMouseEvent *event)
  {
    if (event->button() & Qt::MidButton) {
      // map coords
      QPointF p1 = mapFrameToData(event->posF());
      QPointF p2 = mapFrameToData(mouseClickOrigin);

      // get coords:
      float x1 = p1.x();
      float y1 = p1.y();
      float x2 = p2.x();
      float y2 = p2.y();

      // Discard invalid selections
      if (fabs(x1 - x2) < 1.0e-3 || fabs(y1 - y2) < 1.0e-3) {
        zoomPosF = QPointF();
        return;
      }

      // Sort for limits
      if (x1 > x2) {
        float x = x1; x1 = x2; x2 = x;
      }
      if (y1 > y2) {
        float y = y1; y1 = y2; y2 = y;
      }

      // swap if width/height of current limits are negative
      if (dataRect().width() < 0) {
        float x = x1; x1 = x2; x2 = x;
      }
      if (dataRect().height() < 0) {
        float y = y1; y1 = y2; y2 = y;
      }

      setLimits(x1, x2, y1, y2);
      zoomPosF = QPointF();
    }
  }

  void PlotWidget::wheelEvent(QWheelEvent * event)
  {
    // scroll deltas are in units of 1/8 degree
    float delta = event->delta();
    QPoint pos = event->pos();

    // get current limits
    double x1 = dataRect().x();
    double x2 = x1 + dataRect().width();
    double y1 = dataRect().y();
    double y2 = y1 + dataRect().height();

    // find conversion factor
    QPointF unitPerPixel (dataRect().width()/pixRect().width(), dataRect().height()/pixRect().height());

    // find cursor position in plot units
    QPointF center (x1 + (pos.x() * unitPerPixel.x()), y2 - (pos.y() * unitPerPixel.y()));

    // change per 90 degree rotation (100% zoom on center)
    double Dx1 = (center.x() - x1)/2;
    double Dx2 =-(x2 - center.x())/2;
    double Dy1 = (center.y() - y1)/2;
    double Dy2 =-(y2 - center.y())/2;

    // scaling factor
    double scale = delta * (1.0/8.0) / 90;

    // actual changes in limits
    Dx1 *= scale;
    Dx2 *= scale;
    Dy1 *= scale;
    Dy2 *= scale;

    setLimits(x1 + Dx1, x2 + Dx2, y1 + Dy1, y2 + Dy2);
    event->accept();
  }

  void PlotWidget::resizeEvent( QResizeEvent* e ) {
    QFrame::resizeEvent( e );
    setPixRect();
    resetPlotMask();
  }

  void PlotWidget::setPixRect() {
    int newWidth = contentsRect().width() - leftPadding() - rightPadding();
    int newHeight = contentsRect().height() - topPadding() - bottomPadding();
    // PixRect starts at (0,0) because we will translate by leftPadding(), topPadding()
    d->pixRect = QRect( 0, 0, newWidth, newHeight );
  }

  QPointF PlotWidget::mapToWidget( const QPointF& p ) const
  {
    float px = d->pixRect.left() + d->pixRect.width() * ( p.x() - d->dataRect.x() ) / d->dataRect.width();
    float py = d->pixRect.top() + d->pixRect.height() * ( d->dataRect.y() + d->dataRect.height() - p.y() ) / d->dataRect.height();
    return QPointF( px, py );
  }

  QPointF PlotWidget::mapToData( const QPointF& p ) const
  {
    float px = ( p.x() - d->pixRect.left() ) / d->pixRect.width() * d->dataRect.width() + d->dataRect.x();
    float py = d->dataRect.y() + d->dataRect.height() - ( d->dataRect.height() / d->pixRect.height() ) * ( p.y() - d->pixRect.top() );
    return QPointF( px, py );
  }

  QPointF PlotWidget::mapFrameToData( const QPointF& p ) const
  {
    float px = ( p.x() - leftPadding() ) / d->pixRect.width() * d->dataRect.width() + d->dataRect.x();
    float py = d->dataRect.y() + d->dataRect.height() - ( d->dataRect.height() / d->pixRect.height() ) * ( p.y() - topPadding());
    return QPointF( px, py );
  }

  void PlotWidget::maskRect( const QRectF& rf, float fvalue ) {
    QRect r = rf.toRect().intersected( d->pixRect );
    int value = int( fvalue );
    QColor newColor;
    for ( int ix=r.left(); ix<r.right(); ++ix ) {
      for ( int iy=r.top(); iy<r.bottom(); ++iy ) {
        newColor = QColor( d->plotMask.pixel(ix,iy) );
        newColor.setAlpha( 200 );
        newColor.setRed( qMin( newColor.red() + value, 255 ) );
        d->plotMask.setPixel( ix, iy, newColor.rgba() );
      }
    }

  }

  void PlotWidget::maskAlongLine( const QPointF &p1, const QPointF &p2, float fvalue ) {
    if ( ! d->pixRect.contains( p1.toPoint() ) && ! d->pixRect.contains( p2.toPoint() ) ) {
      return;
    }

    int value = int( fvalue );

    //Determine slope and zeropoint of line
    double m = (p2.y() - p1.y())/(p2.x() - p1.x());
    double y0 = p1.y() - m*p1.x();
    QColor newColor;

    //Mask each pixel along the line joining p1 and p2
    if ( m > 1.0 || m < -1.0 ) { //step in y-direction
      int y1 = int( p1.y() );
      int y2 = int( p2.y() );
      if ( y1 > y2 ) {
        y1 = int( p2.y() );
        y2 = int( p1.y() );
      }

      for ( int y=y1; y<=y2; ++y ) {
        int x = int( (y - y0)/m );
        if ( d->pixRect.contains( x, y ) ) {
          newColor = QColor( d->plotMask.pixel(x,y) );
          newColor.setAlpha( 100 );
          newColor.setRed( qMin( newColor.red() + value, 255 ) );
          d->plotMask.setPixel( x, y, newColor.rgba() );
        }
      }

    } else { //step in x-direction
      int x1 = int( p1.x() );
      int x2 = int( p2.x() );
      if ( x1 > x2 ) {
        x1 = int( p2.x() );
        x2 = int( p1.x() );
      }

      for ( int x=x1; x<=x2; ++x ) {
        int y = int( y0 + m*x );
        if ( d->pixRect.contains( x, y ) ) {
          newColor = QColor( d->plotMask.pixel(x,y) );
          newColor.setAlpha( 100 );
          newColor.setRed( qMin( newColor.red() + value, 255 ) );
          d->plotMask.setPixel( x, y, newColor.rgba() );
        }
      }
    }
  }

  //Determine optimal placement for a text label for point pp.  We want
  //the label to be near point pp, but we don't want it to overlap with
  //other labels or plot elements.  We will use a "downhill simplex"
  //algorithm to find a label position that minimizes the pixel values
  //in the plotMask image over the label's rect().  The sum of pixel
  //values in the label's rect is the "cost" of placing the label there.
  //
  //Because a downhill simplex follows the local gradient to find low
  //values, it can get stuck in local minima.  To mitigate this, we will
  //iteratively attempt each of the initial path offset directions (up,
  //down, right, left) in the order of increasing cost at each location.
  void PlotWidget::placeLabel( QPainter *painter, PlotPoint *pp ) {
    int textFlags = Qt::TextSingleLine | Qt::AlignCenter;

    QPointF pos = mapToWidget( pp->position() );
    if ( ! d->pixRect.contains( pos.toPoint() ) ) return;

    QFontMetricsF fm( painter->font(), painter->device() );
    QRectF bestRect = fm.boundingRect( QRectF( pos.x(), pos.y(), 1, 1 ), textFlags, pp->label() );
    float xStep = 0.5*bestRect.width();
    float yStep = 0.5*bestRect.height();
    float maxCost = 0.05 * bestRect.width() * bestRect.height();
    float bestCost = d->rectCost( bestRect );

    //We will travel along a path defined by the maximum decrease in
    //the cost at each step.  If this path takes us to a local minimum
    //whose cost exceeds maxCost, then we will restart at the
    //beginning and select the next-best path.  The indices of
    //already-tried paths are stored in the TriedPathIndex list.
    //
    //If we try all four first-step paths and still don't get below
    //maxCost, then we'll adopt the local minimum position with the
    //best cost (designated as bestBadCost).
    int iter = 0;
    QList<int> TriedPathIndex;
    float bestBadCost = 10000;
    QRectF bestBadRect;

    //needed to halt iteration from inside the switch
    bool flagStop = false;

    while ( bestCost > maxCost ) {
      //Displace the label up, down, left, right; determine which
      //step provides the lowest cost
      QRectF upRect = bestRect;
      upRect.moveTop( upRect.top() + yStep );
      float upCost = d->rectCost( upRect );
      QRectF downRect = bestRect;
      downRect.moveTop( downRect.top() - yStep );
      float downCost = d->rectCost( downRect );
      QRectF leftRect = bestRect;
      leftRect.moveLeft( leftRect.left() - xStep );
      float leftCost = d->rectCost( leftRect );
      QRectF rightRect = bestRect;
      rightRect.moveLeft( rightRect.left() + xStep );
      float rightCost = d->rectCost( rightRect );

      //which direction leads to the lowest cost?
      QList<float> costList;
      costList << upCost << downCost << leftCost << rightCost;
      int imin = -1;
      for ( int i=0; i<costList.size(); ++i ) {
        if ( iter == 0 && TriedPathIndex.contains( i ) ) {
          continue; //Skip this first-step path, we already tried it!
        }

        //If this first-step path doesn't improve the cost,
        //skip this direction from now on
        if ( iter == 0 && costList[i] >= bestCost ) {
          TriedPathIndex.append( i );
          continue;
        }

        if ( costList[i] < bestCost && (imin < 0 || costList[i] < costList[imin]) ) {

          imin = i;
        }
      }

      //Make a note that we've tried the current first-step path
      if ( iter == 0 && imin >= 0 ) {
        TriedPathIndex.append( imin );
      }

      //Adopt the step that produced the best cost
      switch ( imin ) {
      case 0: //up
        bestRect.moveTop( upRect.top() );
        bestCost = upCost;
        break;
      case 1: //down
        bestRect.moveTop( downRect.top() );
        bestCost = downCost;
        break;
      case 2: //left
        bestRect.moveLeft( leftRect.left() );
        bestCost = leftCost;
        break;
      case 3: //right
        bestRect.moveLeft( rightRect.left() );
        bestCost = rightCost;
        break;
      case -1: //no lower cost found!
        //We hit a local minimum.  Keep the best of these as bestBadRect
        if ( bestCost < bestBadCost ) {
          bestBadCost = bestCost;
          bestBadRect = bestRect;
        }

        //If all of the first-step paths have now been searched, we'll
        //have to adopt the bestBadRect
        if ( TriedPathIndex.size() == 4 ) {
          bestRect = bestBadRect;
          flagStop = true; //halt iteration
          break;
        }

        //If we haven't yet tried all of the first-step paths, start over
        if ( TriedPathIndex.size() < 4 ) {
          iter = -1; //anticipating the ++iter below
          bestRect = fm.boundingRect( QRectF( pos.x(), pos.y(), 1, 1 ), textFlags, pp->label() );
          bestCost = d->rectCost( bestRect );
        }
        break;
      }

      //Halt iteration, because we've tried all directions and
      //haven't gotten below maxCost (we'll adopt the best
      //local minimum found)
      if ( flagStop ) {
        break;
      }

      ++iter;
    }

    //Place label
    painter->drawText( bestRect, textFlags, pp->label() );

    //Is a line needed to connect the label to the point?
    float deltax = pos.x() - bestRect.center().x();
    float deltay = pos.y() - bestRect.center().y();
    float rbest = sqrt( deltax*deltax + deltay*deltay );
    if ( rbest > 20.0 ) {
      //Draw a rectangle around the label
      painter->setBrush( QBrush() );
      //QPen pen = painter->pen();
      //pen.setStyle( Qt::DotLine );
      //painter->setPen( pen );
      painter->drawRoundRect( bestRect );

      //Now connect the label to the point with a line.
      //The line is drawn from the center of the near edge of the rectangle
      float xline = bestRect.center().x();
      if ( bestRect.left() > pos.x() )
        xline = bestRect.left();
      if ( bestRect.right() < pos.x() )
        xline = bestRect.right();

      float yline = bestRect.center().y();
      if ( bestRect.top() > pos.y() )
        yline = bestRect.top();
      if ( bestRect.bottom() < pos.y() )
        yline = bestRect.bottom();

      painter->drawLine( QPointF( xline, yline ), pos );
    }

    //Mask the label's rectangle so other labels won't overlap it.
    maskRect( bestRect );
  }

  float PlotWidget::Private::rectCost( const QRectF &r ) const
  {
    if ( ! plotMask.rect().contains( r.toRect() ) ) {
      return 10000.;
    }

    //Compute sum of mask values in the rect r
    QImage subMask = plotMask.copy( r.toRect() );
    int cost = 0;
    for ( int ix=0; ix<subMask.width(); ++ix ) {
      for ( int iy=0; iy<subMask.height(); ++iy ) {
        cost += QColor( subMask.pixel( ix, iy ) ).red();
      }
    }

    return float(cost);
  }

  void PlotWidget::paintEvent( QPaintEvent *e ) {
    // let QFrame draw its default stuff (like the frame)
    QFrame::paintEvent( e );
    QPainter p;

    p.begin( this );
    p.setFont(d->font);
    p.setRenderHint( QPainter::Antialiasing, d->useAntialias );
    p.fillRect( rect(), backgroundColor() );
    p.translate( leftPadding() + 0.5, topPadding() + 0.5 );

    setPixRect();
    p.setClipRect( d->pixRect );
    p.setClipping( true );

    resetPlotMask();

    foreach( PlotObject *po, d->objectList )
      po->draw( &p, this );

    // Draw private objects
    foreach( PlotObject *po, d->privateObjectList )
      po->draw( &p, this );

    //DEBUG: Draw the plot mask
    //    p.drawImage( 0, 0, d->plotMask );

    p.setClipping( false );
    drawAxes( &p );

    // Draw zoom rectangle
    if (!zoomPosF.isNull()) {
      // Prepare pen
      QPen oldPen = p.pen();
      QPen pen (Qt::red);
      pen.setStyle(Qt::DotLine);
      pen.setWidth(1);
      p.setPen(pen);

      // get points
      int x1 = int(zoomPosF.x());
      int x2 = int(mouseClickOrigin.x());
      int y1 = int(zoomPosF.y());
      int y2 = int(mouseClickOrigin.y());

      // draw rectangle
      p.resetMatrix();
      p.drawLine(x1, y1, x1, y2);
      p.drawLine(x1, y2, x2, y2);
      p.drawLine(x2, y2, x2, y1);
      p.drawLine(x2, y1, x1, y1);
      p.setPen(oldPen);
    }

    p.end();
  }

  bool PlotWidget::saveImage(const QString &filename, double width, double height, double dpi, bool optimizeFontSize )
  {
    // dots per meter
    double dpm = 39.3700787 * dpi;

    // pixel values from meter
    int w = int(width * dpm);
    int h = int(height * dpm);
    // Find the largest dimension and use it to calculate padding, etc.
    int pad = h>w ? h : w;
    int imTopPadding		= int(pad * .01);
    int imBottomPadding	= int(pad * .05);
    int imLeftPadding		= int(pad * .05);
    int imRightPadding	= int(pad * .01);
    int tickOffset		  = 0;
    int bigTickSize		  = int(pad * .01);
    int smallTickSize		= int(pad * .005);

    QImage im (w, h, QImage::Format_ARGB32);
    im.setDotsPerMeterX(int(dpm));
    im.setDotsPerMeterY(int(dpm));
    im.fill(0);
    QPainter p;

    p.begin( &im );
    p.setFont(d->font);
    if (optimizeFontSize) {
      int limHeight = imLeftPadding/2;
      int fontHeight = limHeight;
      QFont tmpFont = p.font();
      do {
        fontHeight -= 1;
        tmpFont.setPixelSize(fontHeight);
      } while (QFontMetrics(tmpFont).height() >= limHeight);
      p.setFont(tmpFont);
    }

    p.setRenderHint( QPainter::Antialiasing, d->useAntialias );
    p.fillRect( im.rect(), backgroundColor() );
    p.translate( imLeftPadding + 0.5, imTopPadding + 0.5 );

    // modify setPixRect():
    int newWidth = im.rect().width() - imLeftPadding - imRightPadding;
    int newHeight = im.rect().height() - imTopPadding - imBottomPadding;
    // PixRect starts at (0,0) because we will translate by leftPadding, topPadding
    QRect imPixRect = QRect( 0, 0, newWidth, newHeight );

    p.setClipRect( imPixRect );
    p.setClipping( true );

    foreach( PlotObject *po, d->objectList ) {
      po->drawImage( &p, &imPixRect, &d->dataRect );
    }

    foreach( PlotObject *po, d->privateObjectList ) {
      po->drawImage( &p, &imPixRect, &d->dataRect );
    }

    p.setClipping( false );

    // modified drawAxes( &p ):
    if ( d->showGrid ) {
      p.setPen( gridColor() );

      //Grid lines are placed at locations of primary axes' major tickmarks
      //vertical grid lines
      foreach ( double xx, axis(BottomAxis)->majorTickMarks() ) {
        double px = imPixRect.width() * (xx - d->dataRect.x()) / d->dataRect.width();
        p.drawLine( QPointF( px, 0.0 ), QPointF( px, double(imPixRect.height()) ) );
      }
      //horizontal grid lines
      foreach( double yy, axis(LeftAxis)->majorTickMarks() ) {
        double py = imPixRect.height() * (yy - d->dataRect.y()) / d->dataRect.height();
        p.drawLine( QPointF( 0.0, py ), QPointF( double(imPixRect.width()), py ) );
      }
    }

    p.setPen( foregroundColor() );
    p.setBrush( Qt::NoBrush );

    /*** BottomAxis ***/
    PlotAxis *a = axis(BottomAxis);
    if (a->isVisible()) {
      //Draw axis line
      p.drawLine( 0, imPixRect.height(), imPixRect.width(), imPixRect.height() );

      // Draw major tickmarks
      foreach( double xx, a->majorTickMarks() ) {
        double px = imPixRect.width() * (xx - d->dataRect.x()) / d->dataRect.width();
        if ( px > 0 && px < imPixRect.width() ) {
          p.drawLine( QPointF( px, double(imPixRect.height() - tickOffset)),
                      QPointF( px, double(imPixRect.height() - bigTickSize - tickOffset)) );
        }
      }

      // Draw minor tickmarks
      foreach ( double xx, a->minorTickMarks() ) {
        double px = imPixRect.width() * (xx - d->dataRect.x()) / d->dataRect.width();
        if ( px > 0 && px < imPixRect.width() ) {
          p.drawLine( QPointF( px, double(imPixRect.height() - tickOffset)),
		      QPointF( px, double(imPixRect.height() - smallTickSize -tickOffset)) );
        }
      }
    }  //End of BottomAxis

    /*** LeftAxis ***/
    a = axis(LeftAxis);
    if (a->isVisible()) {
      //Draw axis line
      p.drawLine( 0, 0, 0, imPixRect.height() );

      // Draw major tickmarks
      foreach( double yy, a->majorTickMarks() ) {
        double py = imPixRect.height() * ( 1.0 - (yy - d->dataRect.y()) / d->dataRect.height() );
        if ( py > 0 && py < imPixRect.height() ) {
          p.drawLine( QPointF( tickOffset, py ), QPointF( double(tickOffset + bigTickSize), py ) );
        }
      }

      // Draw minor tickmarks
      foreach ( double yy, a->minorTickMarks() ) {
        double py = imPixRect.height() * ( 1.0 - (yy - d->dataRect.y()) / d->dataRect.height() );
        if ( py > 0 && py < imPixRect.height() ) {
          p.drawLine( QPointF( tickOffset, py ), QPointF( double(tickOffset + smallTickSize), py ) );
        }
      }
    }  //End of LeftAxis

    //Prepare for top and right axes; we may need the secondary data rect
    double x0 = d->dataRect.x();
    double y0 = d->dataRect.y();
    double dw = d->dataRect.width();
    double dh = d->dataRect.height();
    if ( secondaryDataRect().isValid() ) {
      x0 = secondaryDataRect().x();
      y0 = secondaryDataRect().y();
      dw = secondaryDataRect().width();
      dh = secondaryDataRect().height();
    }

    /*** TopAxis ***/
    a = axis(TopAxis);
    if (a->isVisible()) {
      //Draw axis line
      p.drawLine( 0, 0, imPixRect.width(), 0 );

      // Draw major tickmarks
      foreach( double xx, a->majorTickMarks() ) {
        double px = imPixRect.width() * (xx - x0) / dw;
        if ( px > 0 && px < imPixRect.width() ) {
          p.drawLine( QPointF( px, tickOffset ), QPointF( px, double(bigTickSize + tickOffset)) );

          //Draw ticklabel
          if ( a->areTickLabelsShown() ) {
            QRect r( int(px) - bigTickSize, (int)-1.5*bigTickSize, 2*bigTickSize, bigTickSize );
            p.drawText( r, Qt::AlignCenter | Qt::TextDontClip, a->tickLabel( xx ) );
          }
        }
      }

      // Draw minor tickmarks
      foreach ( double xx, a->minorTickMarks() ) {
        double px = imPixRect.width() * (xx - x0) / dw;
        if ( px > 0 && px < imPixRect.width() ) {
          p.drawLine( QPointF( px, tickOffset ), QPointF( px, double(smallTickSize + tickOffset)) );
        }
      }

      // Draw TopAxis Label
      /*      if ( ! a->label().isEmpty() ) {
        QRect r( 0, 0 - 3*YPADDING, imPixRect.width(), YPADDING );
        p.drawText( r, Qt::AlignCenter, a->label() );
        }*/
    }  //End of TopAxis

    /*** RightAxis ***/
    a = axis(RightAxis);
    if (a->isVisible()) {
      //Draw axis line
      p.drawLine( imPixRect.width(), 0, imPixRect.width(), imPixRect.height() );

      // Draw major tickmarks
      foreach( double yy, a->majorTickMarks() ) {
        double py = imPixRect.height() * ( 1.0 - (yy - y0) / dh );
        if ( py > 0 && py < imPixRect.height() ) {
          p.drawLine( QPointF( double(imPixRect.width() - tickOffset), py ),
                      QPointF( double(imPixRect.width() - tickOffset - bigTickSize), py ) );

          //Draw ticklabel
          if ( a->areTickLabelsShown() ) {
            QRect r( imPixRect.width() + smallTickSize, int(py)-smallTickSize, 2*bigTickSize, 2*smallTickSize );
            p.drawText( r, Qt::AlignLeft | Qt::AlignVCenter | Qt::TextDontClip, a->tickLabel( yy ) );
          }
        }
      }

      // Draw minor tickmarks
      foreach ( double yy, a->minorTickMarks() ) {
        double py = imPixRect.height() * ( 1.0 - (yy - y0) / dh );
        if ( py > 0 && py < imPixRect.height() ) {
          p.drawLine( QPointF( double(imPixRect.width() - 0.0), py ),
                      QPointF( double(imPixRect.width() - 0.0 - smallTickSize), py ) );
        }
      }

      //Draw RightAxis Label.  We need to draw the text sideways.
      /*      if ( ! a->label().isEmpty() ) {
        //store current painter translation/rotation state
        p.save();

        //translate coord sys to left corner of axis label rectangle, then rotate 90 degrees.

        p.translate( imPixRect.width() + 2*XPADDING, imPixRect.height() );
        p.rotate( -90.0 );

        QRect r( 0, 0, imPixRect.height(), XPADDING );
        p.drawText( r, Qt::AlignCenter, a->label() ); //draw the label, now that we are sideways

        p.restore();  //restore translation/rotation state
        }*/
    }  //End of RightAxis

    // Since the following use QLabels to render their text, it is necessary
    // to paint them after drawing to keep the painter from becoming invalid.
    // Also, be sure NOT to name the plotwidget instance as the labels parent.
    // That makes the widget redraw itself continuously.

    // Draw BottomAxis Label
    a = axis(BottomAxis);
    if (a->isVisible() && !a->label().isEmpty() ) {
      QRect r( 0, 0, imPixRect.width(), imBottomPadding/2 );

      QLabel textLabel (a->label(), NULL);
      textLabel.setGeometry(r);
      textLabel.setFont(p.font());
      textLabel.setAlignment(Qt::AlignCenter);

      QPalette palette = textLabel.palette();
      palette.setColor(QPalette::Foreground, foregroundColor());
      palette.setColor(QPalette::Background, QColor(0,0,0,0)); // Transparent background
      textLabel.setPalette(palette);

      QPoint offset (0, imPixRect.height() + imBottomPadding/2);
      textLabel.render(&p, offset);
    }
    // Tick Labels
    if (a->isVisible()) {
      foreach( double xx, a->majorTickMarks() ) {
        double px = imPixRect.width() * (xx - d->dataRect.x()) / d->dataRect.width();
        p.save();
        if ( px > 0 && px < imPixRect.width() ) {
          if ( a->areTickLabelsShown() ) {
            p.setClipping(false);
            QRect r( 0, 0, imPixRect.width(), imBottomPadding/2 );

            QLabel textLabel (a->tickLabel( xx ));
            textLabel.setGeometry(r);
            textLabel.setFont(p.font());
            textLabel.setAlignment(Qt::AlignCenter);

            QPalette palette = textLabel.palette();
            palette.setColor(QPalette::Foreground, foregroundColor());
            palette.setColor(QPalette::Background, QColor(0,0,0,0)); // Transparent background
            textLabel.setPalette(palette);

            QPoint offset  (int(px) - imPixRect.width()/2, imPixRect.height());
            textLabel.render(&p, offset);
          }
        }
        p.restore();
      }
    } // BottomAxis Label

    //Draw LeftAxis Label.  We need to draw the text sideways.
    a = axis(LeftAxis);
    if (a->isVisible() && !a->label().isEmpty() ) {
      p.save();
      //translate coord sys to left corner of axis label rectangle, then rotate 90 degrees.
      p.translate( -imLeftPadding, imPixRect.height() );
      p.rotate( -90.0 );

      QRect r( 0, 0, imPixRect.height(), imLeftPadding/2 );

      QLabel textLabel (a->label(), NULL);
      textLabel.setGeometry(r);
      textLabel.setFont(p.font());
      textLabel.setAlignment(Qt::AlignCenter);

      QPalette palette = textLabel.palette();
      palette.setColor(QPalette::Foreground, foregroundColor());
      palette.setColor(QPalette::Background, QColor(0,0,0,0)); // Transparent background
      textLabel.setPalette(palette);

      QPoint offset (0, 0);
      textLabel.render(&p, offset);
      p.restore();
    }
    if (a->isVisible()) {
      p.save();
      p.rotate( -90.0 );
      foreach( double yy, a->majorTickMarks() ) {
        double py = imPixRect.height() * ( 1.0 - (yy - d->dataRect.y()) / d->dataRect.height() );
        if ( py > 0 && py < imPixRect.height() ) {
          if ( a->areTickLabelsShown() ) {
            p.setClipping(false);
            QRect r( 0, 0, imPixRect.height(), imLeftPadding/2);

            QLabel textLabel (a->tickLabel( yy ));
            textLabel.setGeometry(r);
            textLabel.setFont(p.font());
            textLabel.setAlignment(Qt::AlignCenter);

            QPalette palette = textLabel.palette();
            palette.setColor(QPalette::Foreground, foregroundColor());
            palette.setColor(QPalette::Background, QColor(0,0,0,0)); // Transparent background
            textLabel.setPalette(palette);

            QPoint offset  (-( int(py) + imPixRect.height()/2 ), -imLeftPadding/2);
            textLabel.render(&p, offset);
          }
        }
      }
      p.restore();
    }// LeftAxis
    p.end();
    return im.save(filename);
  }


  void PlotWidget::drawAxes( QPainter *p ) {
    if ( d->showGrid ) {
      p->setPen( gridColor() );

      //Grid lines are placed at locations of primary axes' major tickmarks
      //vertical grid lines
      foreach ( double xx, axis(BottomAxis)->majorTickMarks() ) {
        double px = d->pixRect.width() * (xx - d->dataRect.x()) / d->dataRect.width();
        p->drawLine( QPointF( px, 0.0 ), QPointF( px, double(d->pixRect.height()) ) );
      }
      //horizontal grid lines
      foreach( double yy, axis(LeftAxis)->majorTickMarks() ) {
        double py = d->pixRect.height() * (yy - d->dataRect.y()) / d->dataRect.height();
        p->drawLine( QPointF( 0.0, py ), QPointF( double(d->pixRect.width()), py ) );
      }
    }

    p->setPen( foregroundColor() );
    p->setBrush( Qt::NoBrush );

    /*** BottomAxis ***/
    PlotAxis *a = axis(BottomAxis);
    if (a->isVisible()) {
      //Draw axis line
      p->drawLine( 0, d->pixRect.height(), d->pixRect.width(), d->pixRect.height() );

      // Draw major tickmarks
      foreach( double xx, a->majorTickMarks() ) {
        double px = d->pixRect.width() * (xx - d->dataRect.x()) / d->dataRect.width();
        if ( px > 0 && px < d->pixRect.width() ) {
          p->drawLine( QPointF( px, double(d->pixRect.height() - TICKOFFSET)),
                       QPointF( px, double(d->pixRect.height() - BIGTICKSIZE - TICKOFFSET)) );
        }
      }

      // Draw minor tickmarks
      foreach ( double xx, a->minorTickMarks() ) {
        double px = d->pixRect.width() * (xx - d->dataRect.x()) / d->dataRect.width();
        if ( px > 0 && px < d->pixRect.width() ) {
          p->drawLine( QPointF( px, double(d->pixRect.height() - TICKOFFSET)),
                       QPointF( px, double(d->pixRect.height() - SMALLTICKSIZE -TICKOFFSET)) );
        }
      }
    }  //End of BottomAxis

    /*** LeftAxis ***/
    a = axis(LeftAxis);
    if (a->isVisible()) {
      //Draw axis line
      p->drawLine( 0, 0, 0, d->pixRect.height() );

      // Draw major tickmarks
      foreach( double yy, a->majorTickMarks() ) {
        double py = d->pixRect.height() * ( 1.0 - (yy - d->dataRect.y()) / d->dataRect.height() );
        if ( py > 0 && py < d->pixRect.height() ) {
          p->drawLine( QPointF( TICKOFFSET, py ), QPointF( double(TICKOFFSET + BIGTICKSIZE), py ) );
        }
      }

      // Draw minor tickmarks
      foreach ( double yy, a->minorTickMarks() ) {
        double py = d->pixRect.height() * ( 1.0 - (yy - d->dataRect.y()) / d->dataRect.height() );
        if ( py > 0 && py < d->pixRect.height() ) {
          p->drawLine( QPointF( TICKOFFSET, py ), QPointF( double(TICKOFFSET + SMALLTICKSIZE), py ) );
        }
      }
    }  //End of LeftAxis

    //Prepare for top and right axes; we may need the secondary data rect
    double x0 = d->dataRect.x();
    double y0 = d->dataRect.y();
    double dw = d->dataRect.width();
    double dh = d->dataRect.height();
    if ( secondaryDataRect().isValid() ) {
      x0 = secondaryDataRect().x();
      y0 = secondaryDataRect().y();
      dw = secondaryDataRect().width();
      dh = secondaryDataRect().height();
    }

    /*** TopAxis ***/
    a = axis(TopAxis);
    if (a->isVisible()) {
      //Draw axis line
      p->drawLine( 0, 0, d->pixRect.width(), 0 );

      // Draw major tickmarks
      foreach( double xx, a->majorTickMarks() ) {
        double px = d->pixRect.width() * (xx - x0) / dw;
        if ( px > 0 && px < d->pixRect.width() ) {
          p->drawLine( QPointF( px, TICKOFFSET ), QPointF( px, double(BIGTICKSIZE + TICKOFFSET)) );

          //Draw ticklabel
          if ( a->areTickLabelsShown() ) {
            QRect r( int(px) - BIGTICKSIZE, (int)-1.5*BIGTICKSIZE, 2*BIGTICKSIZE, BIGTICKSIZE );
            p->drawText( r, Qt::AlignCenter | Qt::TextDontClip, a->tickLabel( xx ) );
          }
        }
      }

      // Draw minor tickmarks
      foreach ( double xx, a->minorTickMarks() ) {
        double px = d->pixRect.width() * (xx - x0) / dw;
        if ( px > 0 && px < d->pixRect.width() ) {
          p->drawLine( QPointF( px, TICKOFFSET ), QPointF( px, double(SMALLTICKSIZE + TICKOFFSET)) );
        }
      }

      // Draw TopAxis Label
      if ( ! a->label().isEmpty() ) {
        QRect r( 0, 0 - 3*YPADDING, d->pixRect.width(), YPADDING );
        p->drawText( r, Qt::AlignCenter, a->label() );
      }
    }  //End of TopAxis

    /*** RightAxis ***/
    a = axis(RightAxis);
    if (a->isVisible()) {
      //Draw axis line
      p->drawLine( d->pixRect.width(), 0, d->pixRect.width(), d->pixRect.height() );

      // Draw major tickmarks
      foreach( double yy, a->majorTickMarks() ) {
        double py = d->pixRect.height() * ( 1.0 - (yy - y0) / dh );
        if ( py > 0 && py < d->pixRect.height() ) {
          p->drawLine( QPointF( double(d->pixRect.width() - TICKOFFSET), py ),
                       QPointF( double(d->pixRect.width() - TICKOFFSET - BIGTICKSIZE), py ) );

          //Draw ticklabel
          if ( a->areTickLabelsShown() ) {
            QRect r( d->pixRect.width() + SMALLTICKSIZE, int(py)-SMALLTICKSIZE, 2*BIGTICKSIZE, 2*SMALLTICKSIZE );
            p->drawText( r, Qt::AlignLeft | Qt::AlignVCenter | Qt::TextDontClip, a->tickLabel( yy ) );
          }
        }
      }

      // Draw minor tickmarks
      foreach ( double yy, a->minorTickMarks() ) {
        double py = d->pixRect.height() * ( 1.0 - (yy - y0) / dh );
        if ( py > 0 && py < d->pixRect.height() ) {
          p->drawLine( QPointF( double(d->pixRect.width() - 0.0), py ),
                       QPointF( double(d->pixRect.width() - 0.0 - SMALLTICKSIZE), py ) );
        }
      }

      //Draw RightAxis Label.  We need to draw the text sideways.
      if ( ! a->label().isEmpty() ) {
        //store current painter translation/rotation state
        p->save();

        //translate coord sys to left corner of axis label rectangle, then rotate 90 degrees.
        p->translate( d->pixRect.width() + 2*XPADDING, d->pixRect.height() );
        p->rotate( -90.0 );

        QRect r( 0, 0, d->pixRect.height(), XPADDING );
        p->drawText( r, Qt::AlignCenter, a->label() ); //draw the label, now that we are sideways

        p->restore();  //restore translation/rotation state
      }
    }  //End of RightAxis


    // Since the following use QLabels to render their text, it is necessary
    // to paint them after drawing to keep the painter from becoming invalid.

    // Draw BottomAxis Label
    a = axis(BottomAxis);
    if (a->isVisible() && !a->label().isEmpty() ) {
      QRect r( 0, 0, d->pixRect.width(), bottomPadding()/2 );

      QLabel textLabel (a->label(), NULL);
      textLabel.setGeometry(r);
      textLabel.setFont(d->font);
      textLabel.setAlignment(Qt::AlignCenter);

      QPalette palette = textLabel.palette();
      palette.setColor(QPalette::Foreground, foregroundColor());
      palette.setColor(QPalette::Background, QColor(0,0,0,0)); // Transparent background
      textLabel.setPalette(palette);

      QPoint offset (0, d->pixRect.height() + bottomPadding()/2);
      textLabel.render(p, offset);
    }
    // Tick Labels
    if (a->isVisible()) {
      foreach( double xx, a->majorTickMarks() ) {
        double px = d->pixRect.width() * (xx - d->dataRect.x()) / d->dataRect.width();
        p->save();
        if ( px > 0 && px < d->pixRect.width() ) {
          if ( a->areTickLabelsShown() ) {
            p->setClipping(false);
            QRect r( 0, 0, d->pixRect.width(), bottomPadding()/2 );

            QLabel textLabel (a->tickLabel( xx ));
            textLabel.setGeometry(r);
            textLabel.setFont(d->font);
            textLabel.setAlignment(Qt::AlignCenter);

            QPalette palette = textLabel.palette();
            palette.setColor(QPalette::Foreground, foregroundColor());
            palette.setColor(QPalette::Background, QColor(0,0,0,0)); // Transparent background
            textLabel.setPalette(palette);

            QPoint offset  (int(px) - d->pixRect.width()/2, d->pixRect.height());
            textLabel.render(p, offset);
          }
        }
        p->restore();
      }
    } // BottomAxis Label

    //Draw LeftAxis Label.  We need to draw the text sideways.
    a = axis(LeftAxis);
    if (a->isVisible() && !a->label().isEmpty() ) {
      p->save();
      //translate coord sys to left corner of axis label rectangle, then rotate 90 degrees.
      p->translate( -leftPadding(), d->pixRect.height() );
      p->rotate( -90.0 );

      QRect r( 0, 0, d->pixRect.height(), leftPadding()/2 );

      QLabel textLabel (a->label(), NULL);
      textLabel.setGeometry(r);
      textLabel.setFont(d->font);
      textLabel.setAlignment(Qt::AlignCenter);

      QPalette palette = textLabel.palette();
      palette.setColor(QPalette::Foreground, foregroundColor());
      palette.setColor(QPalette::Background, QColor(0,0,0,0)); // Transparent background
      textLabel.setPalette(palette);

      QPoint offset (0, 0);
      textLabel.render(p, offset);
      p->restore();
    }
    if (a->isVisible()) {
      p->save();
      p->rotate( -90.0 );
      foreach( double yy, a->majorTickMarks() ) {
        double py = d->pixRect.height() * ( 1.0 - (yy - d->dataRect.y()) / d->dataRect.height() );
        if ( py > 0 && py < d->pixRect.height() ) {
          if ( a->areTickLabelsShown() ) {
            p->setClipping(false);
            QRect r( 0, 0, d->pixRect.height(), leftPadding()/2);

            QLabel textLabel (a->tickLabel( yy ), NULL);
            textLabel.setGeometry(r);
            textLabel.setFont(d->font);
            textLabel.setAlignment(Qt::AlignCenter);

            QPalette palette = textLabel.palette();
            palette.setColor(QPalette::Foreground, foregroundColor());
            palette.setColor(QPalette::Background, QColor(0,0,0,0)); // Transparent background
            textLabel.setPalette(palette);

            QPoint offset  (-( int(py) + d->pixRect.height()/2 ), -leftPadding()/2);
            textLabel.render(p, offset);
          }
        }
      }
      p->restore();
    }// LeftAxis
  }

  int PlotWidget::leftPadding() const
  {
    if ( d->leftPadding >= 0 )
      return d->leftPadding;
    const PlotAxis *a = axis( LeftAxis );
    if ( a && a->isVisible() && a->areTickLabelsShown() )
    {
      return !a->label().isEmpty() ? 3 * XPADDING : 2 * XPADDING;
    }
    return XPADDING;
  }

  int PlotWidget::rightPadding() const
  {
    if ( d->rightPadding >= 0 )
      return d->rightPadding;
    const PlotAxis *a = axis( RightAxis );
    if ( a && a->isVisible() && a->areTickLabelsShown() )
    {
      return !a->label().isEmpty() ? 3 * XPADDING : 2 * XPADDING;
    }
    return XPADDING;
  }

  int PlotWidget::topPadding() const
  {
    if ( d->topPadding >= 0 )
      return d->topPadding;
    const PlotAxis *a = axis( TopAxis );
    if ( a && a->isVisible() && a->areTickLabelsShown() )
    {
      return !a->label().isEmpty() ? 3 * YPADDING : 2 * YPADDING;
    }
    return YPADDING;
  }

  int PlotWidget::bottomPadding() const
  {
    if ( d->bottomPadding >= 0 )
      return d->bottomPadding;
    const PlotAxis *a = axis( BottomAxis );
    if ( a && a->isVisible() && a->areTickLabelsShown() )
    {
      return !a->label().isEmpty() ? 3 * YPADDING : 2 * YPADDING;
    }
    return YPADDING;
  }

  void PlotWidget::setLeftPadding( int padding )
  {
    d->leftPadding = padding;
  }

  void PlotWidget::setRightPadding( int padding )
  {
    d->rightPadding = padding;
  }

  void PlotWidget::setTopPadding( int padding )
  {
    d->topPadding = padding;
  }

  void PlotWidget::setBottomPadding( int padding )
  {
    d->bottomPadding = padding;
  }

  void PlotWidget::setDefaultPaddings()
  {
    d->leftPadding = -1;
    d->rightPadding = -1;
    d->topPadding = -1;
    d->bottomPadding = -1;
  }

}
