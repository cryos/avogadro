/**********************************************************************
  Point - Point class derived from the base Primitive class

  Copyright (C) 2008 Tim Vandermeersch

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

#include "point.h"

namespace Avogadro {

  Point::Point(QObject *parent) : Primitive(PointType, parent), 
      m_radius(0.5), m_color(Color(1.0, 1.0, 0.0, 1.0))
  { }

  Point::~Point()
  { }

} // End namespace Avogadro

#include "point.moc"
