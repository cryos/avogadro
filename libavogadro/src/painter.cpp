/**********************************************************************
  Painter - drawing spheres, cylinders and text

  Copyright (C) 2007 Benoit Jacob
  Copyright (C) 2007 Donald Ephraim Curtis
  Copyright (C) 2007 Marcus D. Hanwell

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

#include "painter.h"

namespace Avogadro
{

  Painter::Painter()
  {
  }

  Painter::~Painter()
  {
  }

  bool Painter::initialize()
  {
    return false;
  }

  bool Painter::finalize()
  {
    return false;
  }

  void Painter::drawSphere(const Eigen::Vector3d *center, double radius)
  {
    drawSphere(*center, radius);
  }

} // end namespace Avogadro
