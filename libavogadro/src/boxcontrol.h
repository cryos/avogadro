/**********************************************************************
  BoxControl - 

  Copyright (C) 2008 Tim Vandermeersch

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

#ifndef BOXCONTROL_H
#define BOXCONTROL_H

#include <avogadro/global.h>
#include <avogadro/color.h>
#include <avogadro/point.h>
#include <avogadro/line.h>

namespace Avogadro {

  class A_EXPORT BoxControl : public QObject
  {
    Q_OBJECT 

    public:
      /**
       * Constructor.
       */
      BoxControl();
      /**
       * Destructor.
       */
      ~BoxControl();
      /**
       * Add (show) the point & line primitives to the current GLWidget.
       */
      void addPrimitives();
      /**
       * Remove (hide) the point & line primitives to the current GLWidget.
       */
      void removePrimitives();
      /**
       * Return true if the user has modified the box by dragging 
       * one of the points.
       */
      bool isModified() const { return m_modified; }
      /**
       * Set the modified flag to @p value.
       */
      void setModified(bool value) { m_modified = value; }
      /**
       * Set the box based on two opposite corners.
       * @param p1 Corner one.
       * @param p2 Corner two.
       */
      void setOppositeCorners(const Eigen::Vector3d &p1, const Eigen::Vector3d &p2);
      /**
       * Get the minimum position (corner one).
       */
      Eigen::Vector3d min() { return m_min; }
      /**
       * Get the maximum position (corner two).
       */
      Eigen::Vector3d max() { return m_max; }
    signals:
      /**
       * This signal is emitted when the user changes the box by dragging 
       * one of the points.
       */
      void modified();

    public slots:
      // Handle points clicking...
      void mousePressEvent(Point *point, QMouseEvent * event);
      void mouseMoveEvent(Point *point, QMouseEvent * event);
      void mouseReleaseEvent(Point *point, QMouseEvent * event);


    private:
      /**
       * Update the positions for the points & lines.
       */
      void updatePrimitives();
      
      Eigen::Vector3d m_min, m_max;
      QVector<Point*> m_points;
      QVector<Line*>  m_lines;
      QPoint          m_lastDraggingPosition;
      bool            m_modified;
      bool            m_added;
  };

} // end namespace Avogadro

#endif
