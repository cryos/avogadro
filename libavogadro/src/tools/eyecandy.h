/**********************************************************************
  Eyecandy - Draw arrows etc. inside the Avogadro scene

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

#ifndef EYECANDY_H
#define EYECANDY_H

#include <avogadro/glwidget.h>
#include <avogadro/camera.h>
#include <QPoint>

namespace Avogadro {

  class Eyecandy
  {
    public:
      Eyecandy() : m_color(1.0, 1.0, 0.3, 1.0) {}
      ~Eyecandy() {}

      void setColor(const Color &color);
      Color color() const;

      void drawRotation(GLWidget *widget, const Eigen::Vector3d *center,
                        double radius, double xAngle, double yAngle);

      void drawRotation(GLWidget *widget, Atom *clickedAtom, double xAngle,
                        double yAngle, const Eigen::Vector3d *center);

      void drawTranslation(GLWidget *widget, const Eigen::Vector3d *center,
                           double size, double shift);
      void drawTranslation(GLWidget *widget, Atom *clickedAtom,
                           const Eigen::Vector3d *center);

      void drawZoom(GLWidget *widget, const Eigen::Vector3d *center,
                    double radius);
      void drawZoom(GLWidget *widget, Atom *clickedAtom,
                    const Eigen::Vector3d *center);

    private:
      void drawRotationHorizRibbon();
      void drawRotationVertRibbon();
      void drawRotationRightArrow();
      void drawRotationLeftArrow();
      void drawRotationUpArrow();
      void drawRotationDownArrow();

      double m_yAngleStart, m_yAngleEnd, m_xAngleStart, m_xAngleEnd;
      double m_radius;
      Eigen::Vector3d m_center, m_xAxis, m_yAxis, m_zAxis;
      Color m_color;
  };

}

#endif
