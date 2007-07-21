/**********************************************************************
  GLWidget - general OpenGL display

  Copyright (C) 2006,2007 Geoffrey R. Hutchison
  Copyright (C) 2006,2007 Donald Ephraim Curtis

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

#ifndef __PAINTERDEVICE_H
#define __PAINTERDEVICE_H

namespace Avogadro {

  class Painter;
  class Camera;
  class Primitive;

  class PainterDevice
  {
  public:
    PainterDevice() {}
    virtual ~PainterDevice() {}

    virtual Painter *painter() const = 0;
    virtual Camera *camera() const = 0;
    virtual bool isSelected( const Primitive *p ) const = 0;
    virtual double radius( const Primitive *p ) const = 0;

    virtual int width() = 0;
    virtual int height() = 0;
  };


} // End namespace Avogadro

#endif
