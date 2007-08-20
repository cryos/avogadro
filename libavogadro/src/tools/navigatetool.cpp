/**********************************************************************
  NavigateTool - Navigation Tool for Avogadro

  Copyright (C) 2007 by Marcus D. Hanwell
  Copyright (C) 2006,2007 by Benoit Jacob

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

#include "navigatetool.h"
#include "navigate.h"
#include <avogadro/primitive.h>
#include <avogadro/color.h>
#include <avogadro/glwidget.h>
#include <avogadro/camera.h>

#include <openbabel/obiter.h>
#include <openbabel/mol.h>

#include <QtPlugin>

#define TESS_LEVEL 20

using namespace std;
using namespace OpenBabel;
using namespace Avogadro;
using namespace Eigen;

NavigateTool::NavigateTool(QObject *parent) : Tool(parent), m_clickedAtom(0), m_leftButtonPressed(false), m_midButtonPressed(false), m_rightButtonPressed(false)
{
  QAction *action = activateAction();
  action->setIcon(QIcon(QString::fromUtf8(":/navigate/navigate.png")));
  action->setToolTip(tr("Navigation Tool (F9)\n\n"
        "Left Mouse:   Click and drag to rotate the view\n"
        "Middle Mouse: Click and drag to zoom in or out\n"
        "Right Mouse:  Click and drag to move the view"));
  action->setShortcut(Qt::Key_F9);
}

NavigateTool::~NavigateTool()
{
}

int NavigateTool::usefulness() const
{
  return 2500000;
}

QUndoCommand* NavigateTool::mousePress(GLWidget *widget, const QMouseEvent *event)
{
  m_lastDraggingPosition = event->pos();
  m_leftButtonPressed = (event->buttons() & Qt::LeftButton
                         && event->modifiers() == Qt::NoModifier);
  // On the Mac, either use a three-button mouse
  // or hold down the Option key (AltModifier in Qt notation)
  m_midButtonPressed = ( (event->buttons() & Qt::MidButton) ||
                         (event->buttons() & Qt::LeftButton && event->modifiers() & Qt::AltModifier) );
  // Hold down the Command key (ControlModifier in Qt notation) for right button
  m_rightButtonPressed = ( (event->buttons() & Qt::RightButton) ||
                           (event->buttons() & Qt::LeftButton && event->modifiers() & Qt::ControlModifier) );

  m_clickedAtom = widget->computeClickedAtom(event->pos());

  // Initialise the angle variables on any new mouse press
  vAngle = 0.;
  hAngle = 0.;

  widget->update();
  return 0;
}

QUndoCommand* NavigateTool::mouseRelease(GLWidget *widget, const QMouseEvent*)
{
  m_leftButtonPressed = false;
  m_midButtonPressed = false;
  m_rightButtonPressed = false;
  m_clickedAtom = 0;

  widget->update();
  return 0;
}

QUndoCommand* NavigateTool::mouseMove(GLWidget *widget, const QMouseEvent *event)
{
  if(!widget->molecule()) {
    return 0;
  }

  QPoint deltaDragging = event->pos() - m_lastDraggingPosition;

  // Mouse navigation has two modes - atom centred when an atom is clicked
  // and scene if no atom has been clicked.

  if( m_clickedAtom )
  {
    if (event->buttons() & Qt::LeftButton && event->modifiers() == Qt::NoModifier)
    {
      // Atom centred rotation
      hAngle += deltaDragging.x();
      vAngle += deltaDragging.y();
      Navigate::rotate(widget, m_clickedAtom->pos(), deltaDragging.x(), deltaDragging.y());
    }
  // On the Mac, either use a three-button mouse
  // or hold down the Option key (AltModifier in Qt notation)
    else if ( (event->buttons() & Qt::MidButton) ||
              (event->buttons() & Qt::LeftButton && event->modifiers() & Qt::AltModifier) )
    {
      // Perform the rotation
      Navigate::tilt(widget, m_clickedAtom->pos(), deltaDragging.x());

      // Perform the zoom toward clicked atom
      Navigate::zoom(widget, m_clickedAtom->pos(), deltaDragging.y());
    }
    // On the Mac, either use a three-button mouse
    // or hold down the Command key (ControlModifier in Qt notation)
    else if ( (event->buttons() & Qt::RightButton) ||
              (event->buttons() & Qt::LeftButton && event->modifiers() & Qt::ControlModifier) )
    {
      // translate the molecule following mouse movement
      Navigate::translate(widget, m_clickedAtom->pos(), m_lastDraggingPosition, event->pos());
    }
  }
  else // Nothing clicked on
  {
    if (event->buttons() & Qt::LeftButton
        && event->modifiers() == Qt::NoModifier)
    {
      // rotation around the center of the molecule
      Navigate::rotate(widget, widget->center(), deltaDragging.x(), deltaDragging.y());
    }
  // On the Mac, either use a three-button mouse
  // or hold down the Option key (AltModifier in Qt notation)
    else if ( (event->buttons() & Qt::MidButton) ||
              (event->buttons() & Qt::LeftButton && event->modifiers() & Qt::AltModifier) )
    {
      // Perform the rotation
      Navigate::tilt(widget, widget->center(), deltaDragging.x());

      // Perform the zoom toward molecule center
      Navigate::zoom(widget, widget->center(), deltaDragging.y());
    }
    // On the Mac, either use a three-button mouse
    // or hold down the Command key (ControlModifier in Qt notation)
    else if ( (event->buttons() & Qt::RightButton) ||
              (event->buttons() & Qt::LeftButton && event->modifiers() & Qt::ControlModifier) )
    {
      // translate the molecule following mouse movement
      Navigate::translate(widget, widget->center(), m_lastDraggingPosition, event->pos());
    }
  }

  m_lastDraggingPosition = event->pos();
  widget->update();

  return 0;
}

QUndoCommand* NavigateTool::wheel(GLWidget *widget, const QWheelEvent *event )
{
  m_clickedAtom = widget->computeClickedAtom(event->pos());
  if( m_clickedAtom )
  {
    // Perform the zoom toward clicked atom
    Navigate::zoom(widget, m_clickedAtom->pos(), - MOUSE_WHEEL_SPEED * event->delta());
  }
  else
  {
    // Perform the zoom toward molecule center
    Navigate::zoom(widget, widget->center(), - MOUSE_WHEEL_SPEED * event->delta());
  }
  widget->update();

  return 0;
}

bool NavigateTool::paint(GLWidget *widget)
{
  if(m_leftButtonPressed) {
    if(m_clickedAtom) {
      // Draw ribbons around the atom rotation is centred on and arrows at the end
      double renderRadius = widget->radius(m_clickedAtom);
      renderRadius += 0.04;
      glEnable(GL_BLEND);
      glDepthMask(GL_FALSE);
      Color(1.0, 1.0, 0.3, 0.7).applyAsMaterials();

      // Set up the axes and some vectors to work with
      Vector3d xAxis = widget->camera()->backtransformedXAxis();
      Vector3d yAxis = widget->camera()->backtransformedYAxis();
      Vector3d zAxis = widget->camera()->backtransformedZAxis();
      Vector3d v, v1, v2, v3;

      // Horizontal arrows
      // The start and stop angles for the ribbons
      double angle_start = 2.0 * M_PI * 0.30 - hAngle / 180.;
      double angle_end = 2.0 * M_PI * 1.20 - hAngle / 180.;

      // Horizontal ribbon, back face
      glBegin(GL_QUAD_STRIP);
      for(int i = 0; i <= TESS_LEVEL; i++) {
        double alpha = angle_start + (static_cast<double>(i) / TESS_LEVEL)
                                   * (angle_end - angle_start);
        v = cos(alpha) * xAxis + sin(alpha) * zAxis;
        v1 = v - 0.1  * yAxis;
        v2 = v + 0.1  * yAxis;
        glNormal3dv(v.array());
        glVertex3dv((m_clickedAtom->pos() + renderRadius * v2).array());
        glVertex3dv((m_clickedAtom->pos() + renderRadius * v1).array());
      }
      glEnd();

      // Horizontal ribbon, front face
      glBegin(GL_QUAD_STRIP);
      for(int i = 0; i <= TESS_LEVEL; i++) {
        double alpha = angle_start + (static_cast<double>(i) / TESS_LEVEL)
                                   * (angle_end - angle_start);
        v = cos(alpha) * xAxis + sin(alpha) * zAxis;
        v1 = v - 0.1  * yAxis;
        v2 = v + 0.1  * yAxis;
        glNormal3dv(v.array());
        glVertex3dv((m_clickedAtom->pos() + renderRadius * v1).array());
        glVertex3dv((m_clickedAtom->pos() + renderRadius * v2).array());
      }
      glEnd();

      // Left arrow, pointing rightwards
      v = cos(angle_start) * xAxis + sin(angle_start) * zAxis;
      v1 = v - 0.2 * yAxis;
      v2 = v + 0.2 * yAxis;
      v3 = v + 0.2 * xAxis;
      glBegin(GL_TRIANGLES);
      glNormal3dv(v.array());
      glVertex3dv((m_clickedAtom->pos() + renderRadius * v1).array());
      glVertex3dv((m_clickedAtom->pos() + renderRadius * v3).array());
      glVertex3dv((m_clickedAtom->pos() + renderRadius * v2).array());
      glEnd();

      // Right arrow, pointing leftwards
      v = cos(angle_end) * xAxis + sin(angle_end) * zAxis;
      v1 = v + 0.2 * yAxis;
      v2 = v - 0.2 * yAxis;
      v3 = v - 0.2 * xAxis;
      glBegin(GL_TRIANGLES);
      glNormal3dv(v.array());
      glVertex3dv((m_clickedAtom->pos() + renderRadius * v1).array());
      glVertex3dv((m_clickedAtom->pos() + renderRadius * v3).array());
      glVertex3dv((m_clickedAtom->pos() + renderRadius * v2).array());
      glEnd();

      // Vertical arrows
      // The start and stop angles for the ribbons.
      angle_start = 2.0 * M_PI * 0.30 + vAngle / 180.;
      angle_end = 2.0 * M_PI * 1.20 + vAngle / 180.;
      // Vertical ribbon, back face
      glBegin(GL_QUAD_STRIP);
      for(int i = 0; i <= TESS_LEVEL; i++) {
        double alpha = angle_start + (static_cast<double>(i) / TESS_LEVEL)
                                   * (angle_end - angle_start);
        v = cos(alpha) * yAxis + sin(alpha) * zAxis;
        v1 = v - 0.1  * xAxis;
        v2 = v + 0.1  * xAxis;
        glNormal3dv(v.array());
        glVertex3dv((m_clickedAtom->pos() + renderRadius * v2).array());
        glVertex3dv((m_clickedAtom->pos() + renderRadius * v1).array());
      }
      glEnd();

      // horizontal ribbon, front face
      glBegin(GL_QUAD_STRIP);
      for(int i = 0; i <= TESS_LEVEL; i++) {
        double alpha = angle_start + (static_cast<double>(i) / TESS_LEVEL)
                                   * (angle_end - angle_start);
        v = cos(alpha) * yAxis + sin(alpha) * zAxis;
        v1 = v - 0.1  * xAxis;
        v2 = v + 0.1  * xAxis;
        glNormal3dv(v.array());
        glVertex3dv((m_clickedAtom->pos() + renderRadius * v1).array());
        glVertex3dv((m_clickedAtom->pos() + renderRadius * v2).array());
      }
      glEnd();

      // left arrow, pointing rightwards
      v = cos(angle_start) * yAxis + sin(angle_start) * zAxis;
      v1 = v - 0.2 * xAxis;
      v2 = v + 0.2 * xAxis;
      v3 = v + 0.2 * yAxis;
      glBegin(GL_TRIANGLES);
      glNormal3dv(v.array());
      glVertex3dv((m_clickedAtom->pos() + renderRadius * v1).array());
      glVertex3dv((m_clickedAtom->pos() + renderRadius * v2).array());
      glVertex3dv((m_clickedAtom->pos() + renderRadius * v3).array());
      glEnd();

      // right arrow, pointing leftwards
      v = cos(angle_end) * yAxis + sin(angle_end) * zAxis;
      v1 = v + 0.2 * xAxis;
      v2 = v - 0.2 * xAxis;
      v3 = v - 0.2 * yAxis;
      glBegin(GL_TRIANGLES);
      glNormal3dv(v.array());
      glVertex3dv((m_clickedAtom->pos() + renderRadius * v1).array());
      glVertex3dv((m_clickedAtom->pos() + renderRadius * v2).array());
      glVertex3dv((m_clickedAtom->pos() + renderRadius * v3).array());
      glEnd();

      glDisable(GL_BLEND);
      glDepthMask(GL_TRUE);
    }
    else
    {
      widget->painter()->setColor(1.0, 1.0, 0.3, 0.7);
      widget->painter()->drawSphere(widget->center(), 0.10);
    }
  }

  else if(m_midButtonPressed) {
    if(m_clickedAtom) {
      double renderRadius = widget->radius(m_clickedAtom);
      renderRadius += 0.10;
      glEnable( GL_BLEND );
      widget->painter()->setColor(1.0, 1.0, 0.3, 0.7);
      widget->painter()->drawSphere(m_clickedAtom->pos(), renderRadius);
      glDisable( GL_BLEND );
    }
    else
    {
      widget->painter()->setColor(1.0, 1.0, 0.3, 0.7);
      widget->painter()->drawSphere(widget->center(), 0.10);
    }
  }

  else if(m_rightButtonPressed) {
    if(m_clickedAtom) {
      // Draw arrows coming out of the atom
      double renderRadius = widget->radius(m_clickedAtom);
      renderRadius += 0.04;
      glEnable(GL_BLEND);
      glDisable(GL_LIGHTING);
      glDepthMask(GL_FALSE);
      glColor4f(1.0, 1.0, 0.3, 0.7);
      //Color(1.0, 1.0, 0.3, 0.7).applyAsMaterials();

      // Set up the axes and some vectors to work with
      Vector3d xAxis = widget->camera()->backtransformedXAxis();
      Vector3d yAxis = widget->camera()->backtransformedYAxis();
      Vector3d zAxis = widget->camera()->backtransformedZAxis();
      Vector3d v;

      // Horizontal arrow, pointing left
      v = m_clickedAtom->pos() + renderRadius*zAxis;
      glBegin(GL_QUAD_STRIP);
      glVertex3dv((v + 0.05*yAxis).array());
      glVertex3dv((v - 0.05*yAxis).array());
      v += 0.6*renderRadius * xAxis;
      glVertex3dv((v + 0.05*yAxis).array());
      glVertex3dv((v - 0.05*yAxis).array());
      glEnd();
      glBegin(GL_TRIANGLES);
      glVertex3dv((v + 0.1*yAxis).array());
      glVertex3dv((v - 0.1*yAxis).array());
      glVertex3dv((v + 0.2*renderRadius*xAxis).array());
      glEnd();
      // Horizontal arrow, pointing right
      v = m_clickedAtom->pos() + renderRadius*zAxis;
      glBegin(GL_QUAD_STRIP);
      glVertex3dv((v - 0.05*yAxis).array());
      glVertex3dv((v + 0.05*yAxis).array());
      v -= 0.6*renderRadius * xAxis;
      glVertex3dv((v - 0.05*yAxis).array());
      glVertex3dv((v + 0.05*yAxis).array());
      glEnd();
      glBegin(GL_TRIANGLES);
      glVertex3dv((v - 0.1*yAxis).array());
      glVertex3dv((v + 0.1*yAxis).array());
      glVertex3dv((v - 0.2*renderRadius*xAxis).array());
      glEnd();
      // Vertical arrow, pointing up
      v = m_clickedAtom->pos() + renderRadius*zAxis;
      glBegin(GL_QUAD_STRIP);
      glVertex3dv((v - 0.05*xAxis).array());
      glVertex3dv((v + 0.05*xAxis).array());
      v += 0.6*renderRadius * yAxis;
      glVertex3dv((v - 0.05*xAxis).array());
      glVertex3dv((v + 0.05*xAxis).array());
      glEnd();
      glBegin(GL_TRIANGLES);
      glVertex3dv((v - 0.1*xAxis).array());
      glVertex3dv((v + 0.1*xAxis).array());
      glVertex3dv((v + 0.2*renderRadius*yAxis).array());
      glEnd();
      // Vertical arrow, pointing down
      v = m_clickedAtom->pos() + renderRadius*zAxis;
      glBegin(GL_QUAD_STRIP);
      glVertex3dv((v + 0.05*xAxis).array());
      glVertex3dv((v - 0.05*xAxis).array());
      v -= 0.6*renderRadius * yAxis;
      glVertex3dv((v + 0.05*xAxis).array());
      glVertex3dv((v - 0.05*xAxis).array());
      glEnd();
      glBegin(GL_TRIANGLES);
      glVertex3dv((v + 0.1*xAxis).array());
      glVertex3dv((v - 0.1*xAxis).array());
      glVertex3dv((v - 0.2*renderRadius*yAxis).array());
      glEnd();
/*      for(int i = 0; i <= TESS_LEVEL; i++) {
        double alpha = angle_start + (static_cast<double>(i) / TESS_LEVEL)
                                   * (angle_end - angle_start);
        v = cos(alpha) * xAxis + sin(alpha) * zAxis;
        v1 = v - 0.1  * yAxis;
        v2 = v + 0.1  * yAxis;
        glNormal3dv(v.array());
        glVertex3dv((m_clickedAtom->pos() + renderRadius * v2).array());
        glVertex3dv((m_clickedAtom->pos() + renderRadius * v1).array());
      } */

      glDisable(GL_BLEND);
      glEnable(GL_LIGHTING);
      glDepthMask(GL_TRUE);
    }
    else
    {
      widget->painter()->setColor(1.0, 1.0, 0.3, 0.7);
      widget->painter()->drawSphere(widget->center(), 0.10);
    }
  }

  return true;
}

#include "navigatetool.moc"

Q_EXPORT_PLUGIN2(navigatetool, NavigateToolFactory)
