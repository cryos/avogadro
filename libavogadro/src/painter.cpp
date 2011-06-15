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

  void Painter::drawLineLoop(const QList<Eigen::Vector3d> & points,
                             const double w)
  {
    if (points.size() < 2) {
      return;
    }

    QList<Eigen::Vector3d>::const_iterator it = points.constBegin();
    QList<Eigen::Vector3d>::const_iterator it_nearEnd = points.constEnd() - 1;

    while (it != it_nearEnd) {
      this->drawLine(*it, *(++it), w);
    }

    this->drawLine(points.last(), points.first(), w);
  }

  void Painter::drawBoxEdges(const Eigen::Vector3d &offset,
                             const Eigen::Vector3d &v1,
                             const Eigen::Vector3d &v2,
                             const Eigen::Vector3d &v3,
                             const double linewidth)
  {
    //       6------8  c1 = origin
    //      /:     /|  c2 = origin + v1
    //     / :    / |  c3 = origin + v2
    //    /  4---/--7  c4 = origin + v3
    //   /  /   /  /   c5 = origin + v1 + v2
    //  3------5  /    c6 = origin + v2 + v3
    //  | /    | /     c7 = origin + v1 + v3
    //  |/     |/      c8 = origin + v1 + v2 + v3
    //  1------2
    const Eigen::Vector3d &c1 (offset);
    const Eigen::Vector3d  c2 (c1 + v1);
    const Eigen::Vector3d  c3 (c1 + v2);
    const Eigen::Vector3d  c4 (c1 + v3);
    const Eigen::Vector3d  c5 (c2 + v2);
    const Eigen::Vector3d  c6 (c3 + v3);
    const Eigen::Vector3d  c7 (c2 + v3);
    const Eigen::Vector3d  c8 (c5 + v3);
    this->drawBoxEdges(c1, c2, c3, c4, c5, c6, c7, c8, linewidth);
  }

  void Painter::drawBoxEdges(const Eigen::Vector3d &c1,
                             const Eigen::Vector3d &c2,
                             const Eigen::Vector3d &c3,
                             const Eigen::Vector3d &c4,
                             const Eigen::Vector3d &c5,
                             const Eigen::Vector3d &c6,
                             const Eigen::Vector3d &c7,
                             const Eigen::Vector3d &c8,
                             const double w)
  {
    //      6------8
    //     /:     /|
    //    / :    / |
    //   /  4---/--7
    //  /  /   /  /
    // 3------5  /
    // | /    | /
    // |/     |/
    // 1------2
    // Near "plane":
    this->drawLine(c1, c2, w);
    this->drawLine(c2, c5, w);
    this->drawLine(c5, c3, w);
    this->drawLine(c3, c1, w);

    // Far "plane":
    this->drawLine(c4, c7, w);
    this->drawLine(c7, c8, w);
    this->drawLine(c8, c6, w);
    this->drawLine(c6, c4, w);

    // Connect
    this->drawLine(c1, c4, w);
    this->drawLine(c2, c7, w);
    this->drawLine(c5, c8, w);
    this->drawLine(c3, c6, w);
  }

} // end namespace Avogadro
