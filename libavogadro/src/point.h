/**********************************************************************
  Point - Point class derived from the base Primitive class

  Copyright (C) 2008 Tim Vandermeersch

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

#ifndef POINT_H
#define POINT_H

#include <avogadro/primitive.h>
#include <avogadro/color.h>

#include <QList>
#include <QMouseEvent>

namespace Avogadro {

  /**
   * @class Point point.h <avogadro/point.h>
   * @brief Representation for a point in space
   * @author Tim Vandermeersch
   *
   * The Point class is a Primitive subclass that provides a generic way
   * of addressing points. 
   */
  class PointPrivate;
  class A_EXPORT Point : public Primitive
  {
    Q_OBJECT

    public:
      /**
       * Constructor
       *
       * @param parent the object parent.
       */
      Point(QObject *parent=0);

      ~Point();

      /**
        * Returns the position of the point.
        * @return The position of the point.
        */
      inline const Eigen::Vector3d &pos () const
      {
        return m_pos;
      }

      /**
       * Sets the position of the point.
       * @param vec Position of the point.
       */
      inline void setPos(const Eigen::Vector3d &vec)
      {
        m_pos = vec;
      }

      /**
       * @return The radius of the point.
       */
      double radius() const 
      {
        return m_radius;
      }
      /**
       * Set the radius of the point to @p value.
       */
      void setRadius(double value)
      {
        m_radius = value;
      }

      /**
       * @return The color of the point.
       */
      const Color* color()
      {
        return &m_color;
      }
      /**
       * Set the color of the point to @p color.
       */
      void setColor(const Color &color)
      {
        float red = color.red();
        float green = color.green();
        float blue = color.blue();
        float alpha = color.alpha();
        m_color.set(red, green, blue, alpha);
      }

      void mousePressed(QMouseEvent *event) { emit mousePressEvent(this, event); }
      void mouseMoved(QMouseEvent *event) { emit mouseMoveEvent(this, event); }
      void mouseReleased(QMouseEvent *event) { emit mouseReleaseEvent(this, event); }
    signals:
      void mousePressEvent(Point *point, QMouseEvent * event);
      void mouseMoveEvent(Point *point, QMouseEvent * event);
      void mouseReleaseEvent(Point *point, QMouseEvent * event);

    private:
      Eigen::Vector3d m_pos;
      double          m_radius;
      Color           m_color;
      Q_DECLARE_PRIVATE(Point)
  };

} // End namespace Avoagdro

#endif
