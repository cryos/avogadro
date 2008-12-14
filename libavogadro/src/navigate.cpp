/**********************************************************************
  Navigate - Navigation Functions for Avogadro

  Copyright (C) 2007 by Marcus D. Hanwell
  Copyright (C) 2007 by Benoit Jacob

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

#include "navigate.h"

using namespace Eigen;

namespace Avogadro {

  Navigate::Navigate()
  {
  }

  Navigate::~Navigate()
  {
  }

  void Navigate::zoom(GLWidget *widget, const Eigen::Vector3d &goal,
                      double delta)
  {
    Vector3d transformedGoal = widget->camera()->modelview() * goal;
    double distanceToGoal = transformedGoal.norm();

    double t = ZOOM_SPEED * delta;
    const double minDistanceToGoal = 2.0 * CAMERA_NEAR_DISTANCE;
    double u = minDistanceToGoal / distanceToGoal - 1.0;

    if(t < u) {
      t = u;
    }

    widget->camera()->modelview().pretranslate(transformedGoal * t);
  }

  void Navigate::translate(GLWidget *widget, const Eigen::Vector3d &what,
                           const QPoint &from, const QPoint &to)
  {
    Vector3d fromPos = widget->camera()->unProject(from, what);
    Vector3d toPos = widget->camera()->unProject(to, what);
    widget->camera()->translate(toPos - fromPos);
  }

  void Navigate::translate(GLWidget *widget, const Eigen::Vector3d &what,
                           double deltaX, double deltaY)
  {
    widget->camera()->translate(Vector3d(deltaX * 0.05, deltaY * 0.05, 0));
  }

  void Navigate::rotate(GLWidget *widget, const Eigen::Vector3d &center,
                        double deltaX, double deltaY)
  {
    // For interactive use, we should switch the X and Y axes
    rotate(widget, center, deltaY, deltaX, 0.0);
  }

  void Navigate::tilt(GLWidget *widget, const Eigen::Vector3d &center,
                      double delta)
  {
    rotate(widget, center, 0.0, 0.0, delta);
  }

  void Navigate::rotate(GLWidget *widget, const Eigen::Vector3d &center,
                        double deltaX, double deltaY, double deltaZ)
  {
    Vector3d xAxis = widget->camera()->backTransformedXAxis();
    Vector3d yAxis = widget->camera()->backTransformedYAxis();
    Vector3d zAxis = widget->camera()->backTransformedZAxis();
    widget->camera()->translate(center);
    widget->camera()->rotate(deltaX * ROTATION_SPEED, xAxis);
    widget->camera()->rotate(deltaY * ROTATION_SPEED, yAxis);
    widget->camera()->rotate(deltaZ * ROTATION_SPEED, zAxis);
    widget->camera()->translate(-center);
  }

}
