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

#include "kplotpoint.h"

#include <QtAlgorithms>
#include <QPainter>

// Removed from avogadro
// #include <kdebug.h>

class KPlotPoint::Private
{
    public:
        Private( KPlotPoint * qq, const QPointF &p, const QString &l, double bw )
            : q( qq ), point( p ), label( l ), barWidth( bw )
        {
        }

        KPlotPoint *q;

        QPointF point;
        QString label;
        double barWidth;
};

KPlotPoint::KPlotPoint()
    : d( new Private( this, QPointF(), QString(), 0.0 ) )
{
}

KPlotPoint::KPlotPoint( double x, double y, const QString &label, double barWidth )
    : d( new Private( this, QPointF( x, y ), label, barWidth ) )
{
}

KPlotPoint::KPlotPoint( const QPointF &p, const QString &label, double barWidth )
    : d( new Private( this, p, label, barWidth ) )
{
}

KPlotPoint::~KPlotPoint()
{
    delete d;
}

QPointF KPlotPoint::position() const
{
    return d->point;
}

void KPlotPoint::setPosition( const QPointF &pos )
{
    d->point = pos;
}

double KPlotPoint::x() const
{
    return d->point.x();
}

void KPlotPoint::setX( double x )
{
    d->point.setX( x );
}

double KPlotPoint::y() const
{
    return d->point.y();
}

void KPlotPoint::setY( double y )
{
    d->point.setY( y );
}

QString KPlotPoint::label() const
{
    return d->label;
}

void KPlotPoint::setLabel( const QString &label )
{
    d->label = label;
}

double KPlotPoint::barWidth() const
{
    return d->barWidth;
}

void KPlotPoint::setBarWidth( double w )
{
    d->barWidth = w;
}
