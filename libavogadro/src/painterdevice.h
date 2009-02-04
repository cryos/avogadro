/**********************************************************************
  PainterDevice - Painter Device base class.

  Copyright (C) 2007,2008 Marcus D. Hanwell

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.openmolecules.net/>

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

#ifndef PAINTERDEVICE_H
#define PAINTERDEVICE_H

#include <avogadro/painter.h>

namespace Avogadro {

  class Camera;
  class Primitive;
  class Molecule;
  class Color;
  class PrimitiveList;

  class A_EXPORT PainterDevice
  {
  public:
    PainterDevice() {}
    virtual ~PainterDevice() {}

    virtual Painter *painter() const = 0;
    virtual Camera *camera() const = 0;
    virtual bool isSelected( const Primitive *p ) const = 0;
    virtual double radius( const Primitive *p ) const = 0;
    virtual const Molecule *molecule() const = 0;
    virtual Color* colorMap() const = 0;
    virtual PrimitiveList * primitives() const { return 0; }

    virtual int width() = 0;
    virtual int height() = 0;
  };

} // End namespace Avogadro

#endif
