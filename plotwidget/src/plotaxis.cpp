/**********************************************************************
  PlotAxis -- Part of the Avogadro 2D plotting interface

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

#include "plotaxis.h"
#include <QDebug>
#include <math.h> //for log10(), pow(), modf()

namespace Avogadro {

  class PlotAxis::Private
  {
  public:
    Private( PlotAxis *qq )
      : q( qq )
      , m_visible( true )
      , m_showTickLabels( false )
      , m_labelFmt( 'g' )
      , m_labelFieldWidth( 0 )
      , m_labelPrec( -1 )
    {
    }

    PlotAxis *q;

    bool m_visible : 1; // Property "visible" defines if Axis is drawn or not.
    bool m_showTickLabels : 1;
    char m_labelFmt; // Number format for number labels, see QString::arg()
    QString m_label; // The label of the axis.
    int m_labelFieldWidth; // Field width for number labels, see QString::arg()
    int m_labelPrec; // Number precision for number labels, see QString::arg()
    QList<double> m_MajorTickMarks, m_MinorTickMarks;
  };

  PlotAxis::PlotAxis( const QString &label )
    : d( new Private( this ) )
  {
    d->m_label = label;
  }

  PlotAxis::~PlotAxis()
  {
    delete d;
  }

  bool PlotAxis::isVisible() const
  {
    return d->m_visible;
  }

  void PlotAxis::setVisible( bool visible )
  {
    d->m_visible = visible;
  }

  bool PlotAxis::areTickLabelsShown() const
  {
    return d->m_showTickLabels;
  }

  void PlotAxis::setTickLabelsShown( bool b )
  {
    d->m_showTickLabels = b;
  }

  void PlotAxis::setLabel( const QString& label )
  {
    d->m_label = label;
  }

  QString PlotAxis::label() const
  {
    return d->m_label;
  }

  void PlotAxis::setTickLabelFormat( char format, int fieldWidth, int precision )
  {
    d->m_labelFieldWidth = fieldWidth;
    d->m_labelFmt = format;
    d->m_labelPrec = precision;
  }

  int PlotAxis::tickLabelWidth() const
  {
    return d->m_labelFieldWidth;
  }

  char PlotAxis::tickLabelFormat() const
  {
    return d->m_labelFmt;
  }

  int PlotAxis::tickLabelPrecision() const
  {
    return d->m_labelPrec;
  }

  void PlotAxis::setTickMarks( double x0, double length ) {
    d->m_MajorTickMarks.clear();
    d->m_MinorTickMarks.clear();

    //s is the power-of-ten factor of length:
    //length = t * s; s = 10^(pwr).  e.g., length=350.0 then t=3.5, s = 100.0; pwr = 2.0
    double pwr = 0.0;
    if (length > 0)	modf( log10( length ), &pwr );
    if (length < 0)	modf( log10(-length ), &pwr );
    double s = pow( 10.0, pwr );
    double t = 0;
    if (length > 0)	t = length / s;
    if (length < 0)	t =-length / s;

    double TickDistance = 0.0; //The distance between major tickmarks
    int NumMajorTicks = 0; //will be between 3 and 5
    int NumMinorTicks = 0; //The number of minor ticks between major ticks (will be 4 or 5)

    //adjust s and t such that t is between 3 and 5:
    if ( t < 3.0 ) {
      t *= 10.0;
      s /= 10.0;
      // t is now between 3 and 30
    }

    if ( t < 6.0 ) { //accept current values
      TickDistance = s;
      NumMajorTicks = int( t );
      NumMinorTicks = 5;
    } else if ( t < 10.0 ) { // adjust by a factor of 2
      TickDistance = s * 2.0;
      NumMajorTicks = int( t / 2.0 );
      NumMinorTicks = 4;
    } else if ( t < 20.0 ) { //adjust by a factor of 4
      TickDistance = s * 4.0;
      NumMajorTicks = int( t / 4.0 );
      NumMinorTicks = 4;
    } else { //adjust by a factor of 5
      TickDistance = s * 5.0;
      NumMajorTicks = int( t / 5.0 );
      NumMinorTicks = 5;
    }

    //We have determined the number of tickmarks and their separation
    //Now we determine their positions in the Data space.

    //Tick0 is the position of a "virtual" tickmark; the first major tickmark
    //position beyond the "minimum" edge of the data range.
    double Tick0 = x0 - fmod( x0, TickDistance );
    if ( x0 < 0.0 ) {
      Tick0 -= TickDistance;
      NumMajorTicks++;
    }

    if (length < 0) {
      TickDistance *= -1;
    }

    for ( int i=0; i<NumMajorTicks+1; i++ ) {
      double xmaj = Tick0 + i*TickDistance;
      if ( (length > 0 && xmaj >= x0 && xmaj <= x0 + length) || (length < 0 && xmaj <= x0 && xmaj >= x0 + length) ) {
        d->m_MajorTickMarks.append( xmaj );
      }

      for ( int j=1; j<NumMinorTicks; j++ ) {
        double xmin = xmaj + TickDistance*j/NumMinorTicks;
        if ( (length > 0 && xmin >= x0 && xmin <= x0 + length) || (length < 0 && xmin <= x0 && xmin >= x0 + length) ) {
          d->m_MinorTickMarks.append( xmin );
        }
      }
    }
  }

  QString PlotAxis::tickLabel( double val ) const {
    if ( d->m_labelFmt == 't' ) {
      while ( val <   0.0 ) val += 24.0;
      while ( val >= 24.0 ) val -= 24.0;

      int h = int(val);
      int m = int( 60.*(val - h) );
      return QString( "%1:%2" ).arg( h, 2, 10, QLatin1Char('0') ).arg( m, 2, 10, QLatin1Char('0') );
    }

    return QString( "%1" ).arg( val, d->m_labelFieldWidth, d->m_labelFmt, d->m_labelPrec );
  }

  QList< double > PlotAxis::majorTickMarks() const
  {
    return d->m_MajorTickMarks;
  }

  QList< double > PlotAxis::minorTickMarks() const
  {
    return d->m_MinorTickMarks;
  }

}
