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

#include "plotpoint.h"

#include <QtAlgorithms>
#include <QPainter>

namespace Avogadro {

  class PlotPoint::Private
  {
  public:
    Private( PlotPoint * qq, const QPointF &p, const QString &l, double bw )
      : q( qq ), point( p ), label( l ), barWidth( bw )
    {
    }

    PlotPoint *q;

    QPointF point;
    QString label;
    double barWidth;
  };

  PlotPoint::PlotPoint()
    : d( new Private( this, QPointF(), QString(), 0.0 ) )
  {
  }

  PlotPoint::PlotPoint( double x, double y, const QString &label, double barWidth )
    : d( new Private( this, QPointF( x, y ), label, barWidth ) )
  {
  }

  PlotPoint::PlotPoint( const QPointF &p, const QString &label, double barWidth )
    : d( new Private( this, p, label, barWidth ) )
  {
  }

  PlotPoint::~PlotPoint()
  {
    delete d;
  }

  QPointF PlotPoint::position() const
  {
    return d->point;
  }

  void PlotPoint::setPosition( const QPointF &pos )
  {
    d->point = pos;
  }

  double PlotPoint::x() const
  {
    return d->point.x();
  }

  void PlotPoint::setX( double x )
  {
    d->point.setX( x );
  }

  double PlotPoint::y() const
  {
    return d->point.y();
  }

  void PlotPoint::setY( double y )
  {
    d->point.setY( y );
  }

  QString PlotPoint::label() const
  {
    return d->label;
  }

  void PlotPoint::setLabel( const QString &label )
  {
    d->label = label;
  }

  double PlotPoint::barWidth() const
  {
    return d->barWidth;
  }

  void PlotPoint::setBarWidth( double w )
  {
    d->barWidth = w;
  }

}
