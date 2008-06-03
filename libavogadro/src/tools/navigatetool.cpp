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
#include "eyecandy.h"
#include <avogadro/navigate.h>
#include <avogadro/primitive.h>
#include <avogadro/color.h>
#include <avogadro/glwidget.h>
#include <avogadro/camera.h>

#include <openbabel/obiter.h>
#include <openbabel/mol.h>

#include <QtPlugin>

using namespace std;
using namespace OpenBabel;
using namespace Eigen;

namespace Avogadro {

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

  void NavigateTool::computeReferencePoint(GLWidget *widget)
  {
    // Remember to account for the situation where no molecule is loaded or it is empty
    if(!widget->molecule())
      m_referencePoint = Vector3d(0., 0., 0.);
    else if(!widget->molecule()->NumAtoms())
      m_referencePoint = Vector3d(0., 0., 0.);
    else if(m_clickedAtom)
      m_referencePoint = m_clickedAtom->pos();
    else {
      // let's set m_referencePoint to be the center of the visible
      // part of the molecule.
      Vector3d atomsBarycenter(0., 0., 0.);
      double sumOfWeights = 0.;
      std::vector<OpenBabel::OBNodeBase*>::iterator i;
      for ( Atom *atom = static_cast<Atom*>(widget->molecule()->BeginAtom(i));
            atom; atom = static_cast<Atom*>(widget->molecule()->NextAtom(i))) {
        Vector3d transformedAtomPos = widget->camera()->modelview() * atom->pos();
        double atomDistance = transformedAtomPos.norm();
        double dot = transformedAtomPos.z() / atomDistance;
        double weight = exp(-30. * (1. + dot));
        sumOfWeights += weight;
        atomsBarycenter += weight * atom->pos();
      }
      atomsBarycenter /= sumOfWeights;
      m_referencePoint = atomsBarycenter;
    }
  }

  QUndoCommand* NavigateTool::mousePress(GLWidget *widget, const QMouseEvent *event)
  {
    m_lastDraggingPosition = event->pos();
    // Make sure there aren't modifier keys clicked with the left button
    // If the user has a Mac and only a one-button mouse, everything
    // looks like a left button
    if (event->buttons() & Qt::LeftButton &&
        event->modifiers() == Qt::NoModifier)
    {
      m_leftButtonPressed = true;
      // Set the cursor - this needs to be reset to Qt::ArrowCursor after
      widget->setCursor(Qt::ClosedHandCursor);
    }

    // On a Mac, click and hold the Shift key
    if (event->buttons() & Qt::MidButton ||
        (event->buttons() & Qt::LeftButton &&
         event->modifiers() == Qt::ShiftModifier))
    {
      m_midButtonPressed = true;
      // Set the cursor - this needs to be reset to Qt::ArrowCursor after
      widget->setCursor(Qt::SizeVerCursor);
    }

    // On a Mac, click and hold either the Command or Control Keys
    // (Control or Meta in Qt-speak)
    if (event->buttons() & Qt::RightButton ||
        (event->buttons() & Qt::LeftButton &&
         (event->modifiers() == Qt::ControlModifier
          || event->modifiers() == Qt::MetaModifier)))
    {
      m_rightButtonPressed = true;
      // Set the cursor - this needs to be reset to Qt::ArrowCursor after
      // Currently, there's a Qt/Mac bug -- SizeAllCursor looks like a spreadsheet cursor
#ifdef Q_WS_MAC
          widget->setCursor(Qt::CrossCursor);
#else 
          widget->setCursor(Qt::SizeAllCursor);
#endif
    }

    m_clickedAtom = widget->computeClickedAtom(event->pos());
    computeReferencePoint(widget);

    // Initialise the angle variables on any new mouse press
    m_yAngleEyecandy = 0.;
    m_xAngleEyecandy = 0.;

    widget->update();
    return 0;
  }

  QUndoCommand* NavigateTool::mouseRelease(GLWidget *widget, const QMouseEvent*)
  {
    m_leftButtonPressed = false;
    m_midButtonPressed = false;
    m_rightButtonPressed = false;
    m_clickedAtom = 0;

    // Set the cursor back to the default cursor
    widget->setCursor(Qt::ArrowCursor);

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
    // and scene if no atom has been clicked. However we don't need two codepaths
    // here because the m_referencePoint has already been computed in mousePress,
    // and that is the only difference between the two modes.

    // update eyecandy angle
    m_xAngleEyecandy += deltaDragging.x() * ROTATION_SPEED;
    m_yAngleEyecandy += deltaDragging.y() * ROTATION_SPEED;

    if (event->buttons() & Qt::LeftButton && event->modifiers() == Qt::NoModifier)
    {
      Navigate::rotate(widget, m_referencePoint, deltaDragging.x(), deltaDragging.y());
    }
    // On the Mac, either use a three-button mouse
    // or hold down the Shift key
    else if ( (event->buttons() & Qt::MidButton) ||
        (event->buttons() & Qt::LeftButton && event->modifiers() & Qt::ShiftModifier) )
    {
      // Perform the rotation
      Navigate::tilt(widget, m_referencePoint, deltaDragging.x());

      // Perform the zoom toward clicked atom
      Navigate::zoom(widget, m_referencePoint, deltaDragging.y());
    }
    // On the Mac, either use a three-button mouse
    // or hold down the Command key (ControlModifier in Qt notation)
    else if ( (event->buttons() & Qt::RightButton) ||
        (event->buttons() & Qt::LeftButton && (event->modifiers() == Qt::ControlModifier || event->modifiers() == Qt::MetaModifier) ) )
    {
      // translate the molecule following mouse movement
      Navigate::translate(widget, m_referencePoint, m_lastDraggingPosition, event->pos());
    }

    m_lastDraggingPosition = event->pos();
    widget->update();

    return 0;
  }

  QUndoCommand* NavigateTool::wheel(GLWidget *widget, const QWheelEvent *event )
  {
    m_clickedAtom = 0; // no need for mouse wheel to detect exactly the atom,
                       // the referencePoint will be accurate enough, and
                       // on large molecules doing a gl selection on every
                       // mousewheel event is too expensive.
    computeReferencePoint(widget); // needs m_clickedAtom to be set.
    Navigate::zoom(widget, m_referencePoint, - MOUSE_WHEEL_SPEED * event->delta());
    widget->update();

    return 0;
  }

  bool NavigateTool::paint(GLWidget *widget)
  {
    if(m_leftButtonPressed) {
      m_eyecandy->drawRotation(widget, m_clickedAtom, m_xAngleEyecandy, m_yAngleEyecandy, m_referencePoint);
    }

    else if(m_midButtonPressed) {
      m_eyecandy->drawZoom(widget, m_clickedAtom, m_referencePoint);
    }

    else if(m_rightButtonPressed) {
      m_eyecandy->drawTranslation(widget, m_clickedAtom, m_referencePoint);
    }

    return true;
  }
}

#include "navigatetool.moc"

Q_EXPORT_PLUGIN2(navigatetool, Avogadro::NavigateToolFactory)
