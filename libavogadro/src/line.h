/**********************************************************************
  Line - Line class derived from the base Primitive class

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

#ifndef LINE_H
#define LINE_H

#include <avogadro/primitive.h>
#include <avogadro/color.h>

#include <QList>
#include <QMouseEvent>

namespace Avogadro {

  /**
   * @class Line line.h <avogadro/line.h>
   * @brief Line Class
   * @author Tim Vandermeersch
   *
   * The Line class is a Primitive subclass that provides a generic way
   * of addressing lines. 
   */
  class LinePrivate;
  class A_EXPORT Line : public Primitive
  {
    Q_OBJECT

    public:
      /**
       * Constructor
       *
       * @param parent the object parent.
       */
      Line(QObject *parent=0);

      ~Line();

      /**
        * Returns the begin position of the line.
        * @return The begin position of the line.
        */
      inline const Eigen::Vector3d &begin () const
      {
        return m_begin;
      }

      /**
        * Returns the end position of the line.
        * @return The end position of the line.
        */
      inline const Eigen::Vector3d &end () const
      {
        return m_end;
      }

      /**
       * Sets the begin position of the line.
       * @param vec Begin position of the line.
       */
      inline void setBegin(const Eigen::Vector3d &vec)
      {
        m_begin = vec;
      }

      /**
       * Sets the end position of the line.
       * @param vec end position of the line.
       */
      inline void setEnd(const Eigen::Vector3d &vec)
      {
        m_end = vec;
      }

      /**
       * @return The width of the line.
       */
      double width() const 
      {
        return m_width;
      }
      /**
       * Set the width of the line to @p value.
       */
      void setWidth(double value)
      {
        m_width = value;
      }
 
      /**
       * @return The color of the line.
       */
      const Color* color()
      {
        return &m_color;
      }
      /**
       * Set the color of the line to @p color.
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
      void mousePressEvent(Line *line, QMouseEvent * event);
      void mouseMoveEvent(Line *line, QMouseEvent * event);
      void mouseReleaseEvent(Line *line, QMouseEvent * event);

    private:
      Eigen::Vector3d m_begin;
      Eigen::Vector3d m_end;
      double          m_width;
      Color           m_color;
      Q_DECLARE_PRIVATE(Line)
  };

} // End namespace Avoagdro

#endif
