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

#ifndef __EYECANDY_H
#define __EYECANDY_H

#include <avogadro/glwidget.h>
#include <avogadro/camera.h>
#include <QPoint>

namespace Avogadro {

  class Eyecandy
  {
      void drawRotationHorizRibbon();
      void drawRotationVertRibbon();
      void drawRotationRightArrow();
      void drawRotationLeftArrow();
      void drawRotationUpArrow();
      void drawRotationDownArrow();
      
      double m_yAngleStart, m_yAngleEnd, m_xAngleStart, m_xAngleEnd;
      double m_renderRadius;
      Eigen::Vector3d m_center, m_xAxis, m_yAxis, m_zAxis;
    
    public:
      Eyecandy() {}
      ~Eyecandy() {}
      void drawRotation(GLWidget *widget, Atom *clickedAtom, double xAngle, double yAngle);
      void drawTranslation(GLWidget *widget, Atom *clickedAtom);
  };

}

#endif
