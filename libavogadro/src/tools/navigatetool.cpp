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
#include "eyecandy.h"
#include <avogadro/primitive.h>
#include <avogadro/color.h>
#include <avogadro/glwidget.h>
#include <avogadro/camera.h>

#include <openbabel/obiter.h>
#include <openbabel/mol.h>

#include <QtPlugin>

using namespace std;
using namespace OpenBabel;
using namespace Avogadro;
using namespace Eigen;

NavigateTool::NavigateTool(QObject *parent) : Tool(parent), m_clickedAtom(0), m_leftButtonPressed(false), m_midButtonPressed(false), m_rightButtonPressed(false),
m_eyecandy(new Eyecandy)
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
  delete m_eyecandy;
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
                         (event->buttons() & Qt::LeftButton && event->modifiers() == Qt::AltModifier) );
  // Hold down the Command key (ControlModifier in Qt notation) for right button
  m_rightButtonPressed = ( (event->buttons() & Qt::RightButton) ||
                           (event->buttons() & Qt::LeftButton && (event->modifiers()== Qt::ControlModifier || event->modifiers() == Qt::MetaModifier )) );

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
              (event->buttons() & Qt::LeftButton && (event->modifiers() == Qt::ControlModifier || event->modifiers() == Qt::MetaModifier) ) )
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
              (event->buttons() & Qt::LeftButton && event->modifiers() == Qt::ControlModifier) )
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
    m_eyecandy->drawRotation(widget, m_clickedAtom, xAngleEyecandy, yAngleEyecandy);
  }

  else if(m_midButtonPressed) {
    m_eyecandy->drawZoom(widget, m_clickedAtom);
  }

  else if(m_rightButtonPressed) {
    m_eyecandy->drawTranslation(widget, m_clickedAtom);
  }

  return true;
}

#include "navigatetool.moc"

Q_EXPORT_PLUGIN2(navigatetool, NavigateToolFactory)
