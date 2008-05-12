/**********************************************************************
  Navigate - Navigation Functions for Avogadro

  Copyright (C) 2007 by Marcus D. Hanwell

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

#ifndef NAVIGATE_H
#define NAVIGATE_H

#include <avogadro/global.h>
#include <avogadro/glwidget.h>
#include <avogadro/camera.h>
#include <QPoint>

namespace Avogadro {

  /**
   * @class Navigate
   * @brief Navigation functions common to several tools.
   * @author Marcus D. Hanwell
   *
   * This class contains navigation functions that are used
   * by several tools.
   */
  class A_EXPORT Navigate
  {
    public:
      /**
       * Constructor.
       */
      Navigate();
      /**
       * Destructor.
       */
      virtual ~Navigate();

      /**
       * Zooms toward a given point by the given amount.
       * @param widget the GLWidget being operated on.
       * @param goal the point that is being zoomed toward.
       * @param delta the amount to zoom by.
       */
      static void zoom(GLWidget *widget, const Eigen::Vector3d &goal,
                       double delta);

      /**
       * Translate between the from and to positions relative to what.
       * @param widget the GLWidget being operated on.
       * @param what the point that is being translated about.
       * @param from the starting position.
       * @param to the ending position.
       */
      static void translate(GLWidget *widget, const Eigen::Vector3d &what,
                            const QPoint &from, const QPoint &to);

      /**
       * Rotate about center by the amounts deltaX and deltaY in tha x and y axes.
       * @param widget the GLWidget being operated on.
       * @param center the point at the center of rotation.
       * @param deltaX the amount to rotate about the x axis in degrees.
       * @param deltaY the amount to rotate about the y axis in degrees.
       */
      static void rotate(GLWidget *widget, const Eigen::Vector3d &center,
                         double deltaX, double deltaY);

      /**
       * Tilt about center by the amount delta z axis.
       * @param widget the GLWidget being operated on.
       * @param center the point at the center of rotation.
       * @param delta the amount to rotate about the z axis in degrees.
       */
      static void tilt(GLWidget *widget, const Eigen::Vector3d &center,
                       double delta);

      /**
       * Rotate about center by deltaX, deltaY, and deltaZ in the x, y and z axes
       * A generalization of the rotate() and tilt() methods.
       * @param widget the GLWidget being operated on.
       * @param center the point at the center of rotation.
       * @param deltaX the amount to rotate about the x axis in degrees.
       * @param deltaY the amount to rotate about the y axis in degrees.
       * @param deltaZ the amount to rotate about the z axis in degrees.
       */
      static void rotate(GLWidget *widget, const Eigen::Vector3d &center,
                         double deltaX, double deltaY, double deltaZ);
  };

}
#endif
