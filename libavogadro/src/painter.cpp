/**********************************************************************
  Painter - drawing spheres, cylinders and text

  Copyright (C) 2007 Benoit Jacob
  Copyright (C) 2007 Donald Ephraim Curtis
  Copyright (C) 2007 Marcus D. Hanwell
  Copyright (C) 2011 David C. Lonie

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

  void Painter::drawQuadrilateral(const Eigen::Vector3d & p1,
                                  const Eigen::Vector3d & p2,
                                  const Eigen::Vector3d & p3,
                                  const Eigen::Vector3d & p4,
                                  double w)
  {
    this->drawLine(p1, p2, w);
    this->drawLine(p2, p3, w);
    this->drawLine(p3, p4, w);
    this->drawLine(p4, p1, w);
  }

} // end namespace Avogadro
