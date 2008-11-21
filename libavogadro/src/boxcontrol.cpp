/**********************************************************************
  BoxControl - Engine for display of isosurfaces

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

#include "boxcontrol.h"

#include <config.h>
#include <avogadro/primitive.h>
#include <avogadro/atom.h>
#include <avogadro/cube.h>
#include <avogadro/molecule.h>

#include <avogadro/glwidget.h>

#include <Eigen/Geometry>

#include <QGLWidget>
#include <QReadWriteLock>
#include <QDebug>

using namespace std;
using namespace OpenBabel;
using namespace Eigen;

namespace Avogadro {

  BoxControl::BoxControl() : m_modified(false), m_added(false)
  {
    m_min.setZero();
    m_max.setZero();

    // Create the eight points of the box and connect the signals
    m_points.resize(8);
    for (int i = 0; i < 8; ++i) {
      m_points[i] = new Point;
      connect(m_points[i], SIGNAL(mousePressEvent(Point*,QMouseEvent*)), 
          this, SLOT(mousePressEvent(Point*,QMouseEvent*)));
      connect(m_points[i], SIGNAL(mouseMoveEvent(Point*,QMouseEvent*)), 
          this, SLOT(mouseMoveEvent(Point*,QMouseEvent*)));
      connect(m_points[i], SIGNAL(mouseReleaseEvent(Point*,QMouseEvent*)), 
          this, SLOT(mouseReleaseEvent(Point*,QMouseEvent*)));
    }

    // Create the twelve edges
    m_lines.resize(12);
    for (int i = 0; i < 12; ++i) {
      m_lines[i] = new Line;
    }
 
  }
  
  BoxControl::~BoxControl() 
  {
//    removePrimitives();
    foreach (Point *point, m_points) {
      delete point;
      point = 0;
    }
    foreach (Line *line, m_lines) {
      delete line;
      line = 0;
    }  
  }

  void BoxControl::addPrimitives() 
  {
    if (m_added) 
      return;
    GLWidget *glwidget = GLWidget::current();
    if (!glwidget)
      return;

    m_added = true;
    foreach (Point *point, m_points) {
      glwidget->addPrimitive( point );
    }
    foreach (Line *line, m_lines) {
      glwidget->addPrimitive( line );
    }
 
    glwidget->update();
  }

  void BoxControl::removePrimitives() 
  {
    if (!m_added)
      return;
    GLWidget *glwidget = GLWidget::current();
    if (!glwidget)
      return;

    m_added = false;
    foreach (Point *point, m_points) {
      glwidget->removePrimitive( point );
    }
    foreach (Line *line, m_lines) {
      glwidget->removePrimitive( line );
    }
 
    glwidget->update();
  }
   
  void BoxControl::setOppositeCorners(const Eigen::Vector3d &p1, const Eigen::Vector3d &p2)
  { 
    // Make sure m_min has the smallest values and m_max the biggest
    if (p1.x() < p2.x()) {
      m_min.x() = p1.x();
      m_max.x() = p2.x();
    } else {
      m_min.x() = p2.x();
      m_max.x() = p1.x();
    }
    if (p1.y() < p2.y()) {
      m_min.y() = p1.y();
      m_max.y() = p2.y();
    } else {
      m_min.y() = p2.y();
      m_max.y() = p1.y();
    }
    if (p1.z() < p2.z()) {
      m_min.z() = p1.z();
      m_max.z() = p2.z();
    } else {
      m_min.z() = p2.z();
      m_max.z() = p1.z();
    }
    updatePrimitives();
  }
    
  void BoxControl::updatePrimitives() 
  {
    /*
     *        4--------7
     *       /|       /|
     *      / |      / |
     *     2--------6  |
     *     |  3-----|--5
     *     | /      | /
     *     |/       |/
     *     0--------1
     *
     */
    m_points[0]->setPos( m_min );
    m_points[1]->setPos(Eigen::Vector3d(m_max.x(), m_min.y(), m_min.z()));
    m_points[2]->setPos(Eigen::Vector3d(m_min.x(), m_max.y(), m_min.z()));
    m_points[3]->setPos(Eigen::Vector3d(m_min.x(), m_min.y(), m_max.z()));
    m_points[4]->setPos(Eigen::Vector3d(m_min.x(), m_max.y(), m_max.z()));
    m_points[5]->setPos(Eigen::Vector3d(m_max.x(), m_min.y(), m_max.z()));
    m_points[6]->setPos(Eigen::Vector3d(m_max.x(), m_max.y(), m_min.z()));
    m_points[7]->setPos( m_max );

    // 0-1
    m_lines[0]->setBegin( Vector3d(m_min.x(), m_min.y(), m_min.z()) );
    m_lines[0]->setEnd  ( Vector3d(m_max.x(), m_min.y(), m_min.z()) );
    // 0-2
    m_lines[1]->setBegin( Vector3d(m_min.x(), m_min.y(), m_min.z()) );
    m_lines[1]->setEnd  ( Vector3d(m_min.x(), m_max.y(), m_min.z()) );
    // 0-3
    m_lines[2]->setBegin( Vector3d(m_min.x(), m_min.y(), m_min.z()) );
    m_lines[2]->setEnd  ( Vector3d(m_min.x(), m_min.y(), m_max.z()) );
    // 1-6
    m_lines[3]->setBegin( Vector3d(m_max.x(), m_min.y(), m_min.z()) );
    m_lines[3]->setEnd  ( Vector3d(m_max.x(), m_max.y(), m_min.z()) );
    // 1-5
    m_lines[4]->setBegin( Vector3d(m_max.x(), m_min.y(), m_min.z()) );
    m_lines[4]->setEnd  ( Vector3d(m_max.x(), m_min.y(), m_max.z()) );
    // 2-6
    m_lines[5]->setBegin( Vector3d(m_min.x(), m_max.y(), m_min.z()) );
    m_lines[5]->setEnd  ( Vector3d(m_max.x(), m_max.y(), m_min.z()) );
    // 2-4
    m_lines[6]->setBegin( Vector3d(m_min.x(), m_max.y(), m_min.z()) );
    m_lines[6]->setEnd  ( Vector3d(m_min.x(), m_max.y(), m_max.z()) );
    // 3-4
    m_lines[7]->setBegin( Vector3d(m_min.x(), m_min.y(), m_max.z()) );
    m_lines[7]->setEnd  ( Vector3d(m_min.x(), m_max.y(), m_max.z()) );
    // 3-5
    m_lines[8]->setBegin( Vector3d(m_min.x(), m_min.y(), m_max.z()) );
    m_lines[8]->setEnd  ( Vector3d(m_max.x(), m_min.y(), m_max.z()) );
    // 7-6
    m_lines[9]->setBegin( Vector3d(m_max.x(), m_max.y(), m_max.z()) );
    m_lines[9]->setEnd  ( Vector3d(m_max.x(), m_max.y(), m_min.z()) );
    // 7-5
    m_lines[10]->setBegin( Vector3d(m_max.x(), m_max.y(), m_max.z()) );
    m_lines[10]->setEnd  ( Vector3d(m_max.x(), m_min.y(), m_max.z()) );
    // 7-4
    m_lines[11]->setBegin( Vector3d(m_max.x(), m_max.y(), m_max.z()) );
    m_lines[11]->setEnd  ( Vector3d(m_min.x(), m_max.y(), m_max.z()) );
 
  }


  void BoxControl::mousePressEvent(Point *point, QMouseEvent * event)
  {
    m_lastDraggingPosition = event->pos();
  }
  
  void BoxControl::mouseMoveEvent(Point *point, QMouseEvent * event) 
  {
    GLWidget *glwidget = GLWidget::current();
    Vector3d fromPos = glwidget->camera()->unProject(m_lastDraggingPosition, point->pos());
    Vector3d toPos = glwidget->camera()->unProject(event->pos(), point->pos());
    Vector3d pointTranslation = toPos - fromPos;
    
    // Opposite corners in our box:  0-7  1-4  2-5  3-6
    switch (m_points.indexOf( point )) {
      case 0:
        setOppositeCorners(pointTranslation + point->pos(), m_points[7]->pos());
        break;
      case 1:
        setOppositeCorners(pointTranslation + point->pos(), m_points[4]->pos());
        break;
      case 2:
        setOppositeCorners(pointTranslation + point->pos(), m_points[5]->pos());
        break;
      case 3:
        setOppositeCorners(pointTranslation + point->pos(), m_points[6]->pos());
        break;
      case 4:
        setOppositeCorners(pointTranslation + point->pos(), m_points[1]->pos());
        break;
      case 5:
        setOppositeCorners(pointTranslation + point->pos(), m_points[2]->pos());
        break;
      case 6:
        setOppositeCorners(pointTranslation + point->pos(), m_points[3]->pos());
        break;
      case 7:
        setOppositeCorners(pointTranslation + point->pos(), m_points[0]->pos());
        break;
    }

    m_modified = true;
    m_lastDraggingPosition = event->pos();
    updatePrimitives();
    glwidget->update();
  }

  void BoxControl::mouseReleaseEvent(Point *point, QMouseEvent * event) 
  {
    emit modified();  
  }

}

#include "boxcontrol.moc"

