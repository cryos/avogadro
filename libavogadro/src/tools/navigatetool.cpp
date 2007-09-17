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

#define TESS_LEVEL 32
#define RIBBON_WIDTH 0.05
#define RIBBON_ARROW_WIDTH 0.15
#define RIBBON_ARROW_LENGTH 0.25
#define RIBBON_APERTURE 0.07

using namespace std;
using namespace OpenBabel;
using namespace Avogadro;
using namespace Eigen;

NavigateTool::NavigateTool(QObject *parent) : Tool(parent), m_clickedAtom(0), m_leftButtonPressed(false), m_midButtonPressed(false), m_rightButtonPressed(false),
m_rotationEyecandy(new RotationEyecandy)
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
  delete m_rotationEyecandy;
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
  yAngleEyecandy = 0.;
  xAngleEyecandy = 0.;

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
      xAngleEyecandy += deltaDragging.x() * ROTATION_SPEED;
      yAngleEyecandy += deltaDragging.y() * ROTATION_SPEED;
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
      xAngleEyecandy += deltaDragging.x() * ROTATION_SPEED;
      yAngleEyecandy += deltaDragging.y() * ROTATION_SPEED;
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
    m_rotationEyecandy->draw(widget, m_clickedAtom, xAngleEyecandy, yAngleEyecandy);
  }

  else if(m_midButtonPressed) {
    if(m_clickedAtom) {
      double renderRadius = qMax(widget->radius(m_clickedAtom) * 1.1 + 0.2,
                                 0.1 * widget->camera()->distance(m_clickedAtom->pos()));
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
    Vector3d center;
    double renderRadius;
    double shift;
    if(clickedAtom())
    {
      center = clickedAtom()->pos();
      renderRadius = qMax(widget->radius(clickedAtom()) * 1.1 + 0.2,
                            0.1 * widget->camera()->distance(center));
      shift = widget->radius(clickedAtom());
    }
    else
    {
      center = widget->center();
      renderRadius = qMax(qMax(widget->radius(), CAMERA_NEAR_DISTANCE),
                          0.1 * widget->camera()->distance(center));
      shift = 0.;
    }
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
    v = center + shift*zAxis;
    glBegin(GL_QUAD_STRIP);
    glVertex3dv((v + 0.05*renderRadius*yAxis).array());
    glVertex3dv((v - 0.05*renderRadius*yAxis).array());
    v += 0.6*renderRadius * xAxis;
    glVertex3dv((v + 0.05*renderRadius*yAxis).array());
    glVertex3dv((v - 0.05*renderRadius*yAxis).array());
    glEnd();
    glBegin(GL_TRIANGLES);
    glVertex3dv((v + 0.1*renderRadius*yAxis).array());
    glVertex3dv((v - 0.1*renderRadius*yAxis).array());
    glVertex3dv((v + 0.2*renderRadius*xAxis).array());
    glEnd();
    // Horizontal arrow, pointing right
    v = center + shift*zAxis;
    glBegin(GL_QUAD_STRIP);
    glVertex3dv((v - 0.05*renderRadius*yAxis).array());
    glVertex3dv((v + 0.05*renderRadius*yAxis).array());
    v -= 0.6*renderRadius * xAxis;
    glVertex3dv((v - 0.05*renderRadius*yAxis).array());
    glVertex3dv((v + 0.05*renderRadius*yAxis).array());
    glEnd();
    glBegin(GL_TRIANGLES);
    glVertex3dv((v - 0.1*renderRadius*yAxis).array());
    glVertex3dv((v + 0.1*renderRadius*yAxis).array());
    glVertex3dv((v - 0.2*renderRadius*xAxis).array());
    glEnd();
    // Vertical arrow, pointing up
    v = center + shift*zAxis;
    glBegin(GL_QUAD_STRIP);
    glVertex3dv((v - 0.05*renderRadius*xAxis).array());
    glVertex3dv((v + 0.05*renderRadius*xAxis).array());
    v += 0.6*renderRadius * yAxis;
    glVertex3dv((v - 0.05*renderRadius*xAxis).array());
    glVertex3dv((v + 0.05*renderRadius*xAxis).array());
    glEnd();
    glBegin(GL_TRIANGLES);
    glVertex3dv((v - 0.1*renderRadius*xAxis).array());
    glVertex3dv((v + 0.1*renderRadius*xAxis).array());
    glVertex3dv((v + 0.2*renderRadius*yAxis).array());
    glEnd();
    // Vertical arrow, pointing down
    v = center + shift*zAxis;
    glBegin(GL_QUAD_STRIP);
    glVertex3dv((v + 0.05*renderRadius*xAxis).array());
    glVertex3dv((v - 0.05*renderRadius*xAxis).array());
    v -= 0.6*renderRadius * yAxis;
    glVertex3dv((v + 0.05*renderRadius*xAxis).array());
    glVertex3dv((v - 0.05*renderRadius*xAxis).array());
    glEnd();
    glBegin(GL_TRIANGLES);
    glVertex3dv((v + 0.1*renderRadius*xAxis).array());
    glVertex3dv((v - 0.1*renderRadius*xAxis).array());
    glVertex3dv((v - 0.2*renderRadius*yAxis).array());
    glEnd();

    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
    glDepthMask(GL_TRUE);
  }

  return true;
}

Atom *NavigateTool::clickedAtom()
{
  return m_clickedAtom;
}

void RotationEyecandy::drawHorizRibbon()
{
  glBegin(GL_QUAD_STRIP);
  for(int i = 0; i <= TESS_LEVEL; i++) {
    double alpha = m_xAngleStart + (static_cast<double>(i) / TESS_LEVEL)
                                * (m_xAngleEnd - m_xAngleStart);
    Vector3d v = cos(alpha) * m_xAxis + sin(alpha) * m_zAxis;
    Vector3d v1 = v - RIBBON_WIDTH * m_yAxis;
    Vector3d v2 = v + RIBBON_WIDTH * m_yAxis;
    glNormal3dv(v.array());
    glVertex3dv((m_center + m_renderRadius * v1).array());
    glVertex3dv((m_center + m_renderRadius * v2).array());
  }
  glEnd();
}

void RotationEyecandy::drawVertRibbon()
{
  glBegin(GL_QUAD_STRIP);
  for(int i = 0; i <= TESS_LEVEL; i++) {
    double alpha = m_yAngleStart + (static_cast<double>(i) / TESS_LEVEL)
                                * (m_yAngleEnd - m_yAngleStart);
    Vector3d v = cos(alpha) * m_yAxis + sin(alpha) * m_zAxis;
    Vector3d v1 = v - RIBBON_WIDTH * m_xAxis;
    Vector3d v2 = v + RIBBON_WIDTH * m_xAxis;
    glNormal3dv(v.array());
    glVertex3dv((m_center + m_renderRadius * v2).array());
    glVertex3dv((m_center + m_renderRadius * v1).array());
  }
  glEnd();
}

void RotationEyecandy::drawLeftArrow()
{
  Vector3d v = cos(m_xAngleEnd) * m_xAxis + sin(m_xAngleEnd) * m_zAxis;
  Vector3d v1 = v + RIBBON_ARROW_WIDTH * m_yAxis;
  Vector3d v2 = v - RIBBON_ARROW_WIDTH * m_yAxis;
  Vector3d v3 = v + RIBBON_ARROW_LENGTH * v.cross(m_yAxis);
  glBegin(GL_TRIANGLES);
  glNormal3dv(v.array());
  glVertex3dv((m_center + m_renderRadius * v1).array());
  glVertex3dv((m_center + m_renderRadius * v3).array());
  glVertex3dv((m_center + m_renderRadius * v2).array());
  glEnd();
}

void RotationEyecandy::drawRightArrow()
{
  Vector3d v = cos(m_xAngleStart) * m_xAxis + sin(m_xAngleStart) * m_zAxis;
  Vector3d v1 = v - RIBBON_ARROW_WIDTH * m_yAxis;
  Vector3d v2 = v + RIBBON_ARROW_WIDTH * m_yAxis;
  Vector3d v3 = v - RIBBON_ARROW_LENGTH * v.cross(m_yAxis);
  glBegin(GL_TRIANGLES);
  glNormal3dv(v.array());
  glVertex3dv((m_center + m_renderRadius * v1).array());
  glVertex3dv((m_center + m_renderRadius * v3).array());
  glVertex3dv((m_center + m_renderRadius * v2).array());
  glEnd();
}

void RotationEyecandy::drawUpArrow()
{
  Vector3d v = cos(m_yAngleStart) * m_yAxis + sin(m_yAngleStart) * m_zAxis;
  Vector3d v1 = v - RIBBON_ARROW_WIDTH * m_xAxis;
  Vector3d v2 = v + RIBBON_ARROW_WIDTH * m_xAxis;
  Vector3d v3 = v + RIBBON_ARROW_LENGTH * v.cross(m_xAxis);
  glBegin(GL_TRIANGLES);
  glNormal3dv(v.array());
  glVertex3dv((m_center + m_renderRadius * v1).array());
  glVertex3dv((m_center + m_renderRadius * v2).array());
  glVertex3dv((m_center + m_renderRadius * v3).array());
  glEnd();
}

void RotationEyecandy::drawDownArrow()
{
  Vector3d v = cos(m_yAngleEnd) * m_yAxis + sin(m_yAngleEnd) * m_zAxis;
  Vector3d v1 = v + RIBBON_ARROW_WIDTH * m_xAxis;
  Vector3d v2 = v - RIBBON_ARROW_WIDTH * m_xAxis;
  Vector3d v3 = v - RIBBON_ARROW_LENGTH * v.cross(m_xAxis);
  glBegin(GL_TRIANGLES);
  glNormal3dv(v.array());
  glVertex3dv((m_center + m_renderRadius * v1).array());
  glVertex3dv((m_center + m_renderRadius * v2).array());
  glVertex3dv((m_center + m_renderRadius * v3).array());
  glEnd();
}

void RotationEyecandy::draw(GLWidget *widget, Atom *clickedAtom, double xAngle, double yAngle)
{
  if(clickedAtom)
  {
    m_center = clickedAtom->pos();
    m_renderRadius = qMax(widget->radius(clickedAtom) * 1.1 + 0.2,
                          0.1 * widget->camera()->distance(m_center));
  }
  else
  {
    m_center = widget->center();
    m_renderRadius = qMax(qMax(widget->radius(), CAMERA_NEAR_DISTANCE),
                          0.1 * widget->camera()->distance(m_center));
  }
                          
  m_xAngleStart = 2.0 * M_PI * (0.25 + RIBBON_APERTURE) - xAngle;
  m_xAngleEnd = 2.0 * M_PI * (1.25 - RIBBON_APERTURE) - xAngle;
  m_yAngleStart = 2.0 * M_PI * (0.25 + RIBBON_APERTURE) + yAngle;
  m_yAngleEnd = 2.0 * M_PI * (1.25 - RIBBON_APERTURE) + yAngle;
  m_xAxis = widget->camera()->backtransformedXAxis();
  m_yAxis = widget->camera()->backtransformedYAxis();
  m_zAxis = widget->camera()->backtransformedZAxis();

  
  glEnable(GL_BLEND);
  glDepthMask(GL_FALSE);
  Color(1.0, 1.0, 0.3, 0.7).applyAsMaterials();
  
  //draw back faces
  glCullFace(GL_FRONT);
  drawHorizRibbon();
  drawVertRibbon();
  drawRightArrow();
  drawLeftArrow();
  drawUpArrow();
  drawDownArrow();
  
  //draw front faces
  glCullFace(GL_BACK); // this restores the default culling behaviour
  drawHorizRibbon();
  drawVertRibbon();
  drawRightArrow();
  drawLeftArrow();
  drawUpArrow();
  drawDownArrow();
          
  glDisable(GL_BLEND);
  glDepthMask(GL_TRUE);
}

#include "navigatetool.moc"

Q_EXPORT_PLUGIN2(navigatetool, NavigateToolFactory)
