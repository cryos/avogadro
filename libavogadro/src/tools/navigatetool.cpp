/**********************************************************************
  NavigateTool - Navigation Tool for Avogadro

  Copyright (C) 2007-2009 by Marcus D. Hanwell
  Copyright (C) 2006,2007 by Benoit Jacob

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

#include "navigatetool.h"
#include "eyecandy.h"

#include "ui_navigatesettingswidget.h"

#include <avogadro/navigate.h>
#include <avogadro/atom.h>
#include <avogadro/molecule.h>
#include <avogadro/color.h>
#include <avogadro/glwidget.h>
#include <avogadro/camera.h>

namespace Avogadro {

  using Eigen::Vector3d;

  class NavigateSettingsWidget : public QWidget,
                                 public Ui::NavigateSettingsWidget
  {
  public:
    NavigateSettingsWidget(QWidget *parent=0) : QWidget(parent)
    {
      setupUi(this);
    }
  };

  NavigateTool::NavigateTool(QObject *parent) : Tool(parent), m_clickedAtom(0),
      m_leftButtonPressed(false), m_midButtonPressed(false),
      m_rightButtonPressed(false), m_eyeCandyEnabled(true),
      m_drawEyeCandy(false), m_draggingInitialized(false),
      m_eyecandy(new Eyecandy), m_settingsWidget(0)
  {
    QAction *action = activateAction();
    action->setIcon(QIcon(QString::fromUtf8(":/navigate/navigate.png")));
    action->setToolTip(tr("Navigation Tool (F9)\n\n"
          "Left Mouse: \tClick and drag to rotate the view.\n"
          "Middle Mouse: Click and drag to zoom in or out.\n"
          "Right Mouse: \tClick and drag to move the view.\n"
          "Double-Click: \tReset the view."));
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
    else if(!widget->molecule()->numAtoms())
      m_referencePoint = Vector3d(0., 0., 0.);
    else if(m_clickedAtom)
      m_referencePoint = *m_clickedAtom->pos();
    else {
      // let's set m_referencePoint to be the center of the visible
      // part of the molecule.
      Vector3d atomsBarycenter(0., 0., 0.);
      double sumOfWeights = 0.;
      QList<Atom*> atoms = widget->molecule()->atoms();
      foreach (Atom *atom, atoms) {
        Vector3d transformedAtomPos = widget->camera()->modelview() * *atom->pos();
        double atomDistance = transformedAtomPos.norm();
        double dot = transformedAtomPos.z() / atomDistance;
        double weight = exp(-30. * (1. + dot));
        sumOfWeights += weight;
        atomsBarycenter += weight * *atom->pos();
      }
      atomsBarycenter /= sumOfWeights;
      m_referencePoint = atomsBarycenter;
    }
  }

  QWidget* NavigateTool::settingsWidget()
  {
    if (!m_settingsWidget) {
      m_settingsWidget = new NavigateSettingsWidget(qobject_cast<QWidget*>(parent()));
      // Restore settings
      m_settingsWidget->eyeCandyCheckBox->setChecked(m_eyeCandyEnabled);
      // Connect some signals and slots
      connect(m_settingsWidget->eyeCandyCheckBox, SIGNAL(stateChanged(int)),
              this, SLOT(enableEyeCandy(int)));
    }
    return m_settingsWidget;
  }

  QUndoCommand* NavigateTool::mousePressEvent(GLWidget *widget, QMouseEvent *event)
  {
    event->accept();
    m_drawEyeCandy = false;
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

  QUndoCommand* NavigateTool::mouseReleaseEvent(GLWidget *widget, QMouseEvent *event)
  {
    event->accept();
    m_leftButtonPressed = false;
    m_midButtonPressed = false;
    m_rightButtonPressed = false;
    m_drawEyeCandy = false;
    m_clickedAtom = 0;
    m_draggingInitialized = false;

    // Set the cursor back to the default cursor
    widget->setCursor(Qt::ArrowCursor);

    widget->update();
    return 0;
  }

  QUndoCommand* NavigateTool::mouseDoubleClickEvent(GLWidget *widget, QMouseEvent *event)
  {
    event->accept();
    m_leftButtonPressed = false;
    m_midButtonPressed = false;
    m_rightButtonPressed = false;
    m_drawEyeCandy = false;
    m_clickedAtom = 0;
    m_draggingInitialized = false;

    // Set the cursor back to the default cursor
    widget->setCursor(Qt::ArrowCursor);

    // reset the camera
    widget->camera()->initializeViewPoint();

    widget->update();
    return 0;
  }

  QUndoCommand* NavigateTool::mouseMoveEvent(GLWidget *widget, QMouseEvent *event)
  {
    if(!widget->molecule()) {
      return 0;
    }

    m_drawEyeCandy = true;
    event->accept();

    QPoint deltaDragging;
    if (m_draggingInitialized) {
      deltaDragging = event->pos() - m_lastDraggingPosition;
    }
    else {
      m_lastDraggingPosition = event->pos();
      m_draggingInitialized = true;
    }

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

  QUndoCommand* NavigateTool::wheelEvent(GLWidget *widget, QWheelEvent *event )
  {
    event->accept();
    m_clickedAtom = 0; // no need for mouse wheel to detect exactly the atom,
                       // the referencePoint will be accurate enough, and
                       // on large molecules doing a gl selection on every
                       // mousewheel event is too expensive.
    computeReferencePoint(widget); // needs m_clickedAtom to be set.
    Navigate::zoom(widget, m_referencePoint, - MOUSE_WHEEL_SPEED * event->delta());
    widget->update();

    return 0;
  }

  QUndoCommand* NavigateTool::keyPressEvent(GLWidget *widget, QKeyEvent *event)
  {
    computeReferencePoint(widget);

    switch (event->key()) {
      case Qt::Key_Left: // Left arrow
      case Qt::Key_H:    // Vi style left
      case Qt::Key_A:    // CS style left
        // On Mac OS X, the arrow keys are considered part of the keypad!?
        // http://doc.trolltech.com/4.4/qt.html#KeyboardModifier-enum
        if (event->modifiers() == Qt::NoModifier
            || event->modifiers() == Qt::KeypadModifier)
          Navigate::rotate(widget, m_referencePoint, -5, 0);
        else if (event->modifiers() & Qt::ShiftModifier)
          Navigate::tilt(widget, m_referencePoint, -5);
        else if (event->modifiers() & Qt::ControlModifier)
          Navigate::translate(widget, m_referencePoint, -5, 0);
        event->accept();
        break;
      case Qt::Key_Right: // Right arrow
      case Qt::Key_L:     // Vi style right
      case Qt::Key_D:     // CS style right
        if (event->modifiers() == Qt::NoModifier
            || event->modifiers() == Qt::KeypadModifier)
          Navigate::rotate(widget, m_referencePoint, 5, 0);
        else if ((event->modifiers() & Qt::ShiftModifier))
          Navigate::tilt(widget, m_referencePoint, 5);
        else if (event->modifiers() & Qt::ControlModifier)
          Navigate::translate(widget, m_referencePoint, 5, 0);
        event->accept();
        break;
      case Qt::Key_Up: // Up arrow
      case Qt::Key_K:  // Vi style up
      case Qt::Key_W:  // CS style up
        if (event->modifiers() == Qt::NoModifier
            || event->modifiers() == Qt::KeypadModifier)
          Navigate::rotate(widget, m_referencePoint, 0, -5);
        else if (event->modifiers() & Qt::ShiftModifier)
          Navigate::zoom(widget, m_referencePoint, -2);
        else if (event->modifiers() & Qt::ControlModifier)
          Navigate::translate(widget, m_referencePoint, 0, -5);
        event->accept();
        break;
      case Qt::Key_Down: // Down arrow
      case Qt::Key_J:    // Vi style down
      case Qt::Key_S:    // CS style down
        if (event->modifiers() == Qt::NoModifier
            || event->modifiers() == Qt::KeypadModifier)
          Navigate::rotate(widget, m_referencePoint, 0, 5);
        else if (event->modifiers() & Qt::ShiftModifier)
          Navigate::zoom(widget, m_referencePoint, 2);
        else if (event->modifiers() & Qt::ControlModifier)
          Navigate::translate(widget, m_referencePoint, 0, 5);
        event->accept();
        break;
      default:
       return 0;
    }
    return 0;
  }

  QUndoCommand* NavigateTool::keyReleaseEvent(GLWidget *, QKeyEvent *)
  {
    return 0;
  }

  bool NavigateTool::paint(GLWidget *widget)
  {
    if (m_eyeCandyEnabled && m_drawEyeCandy) {
      if(m_leftButtonPressed)
        m_eyecandy->drawRotation(widget, m_clickedAtom, m_xAngleEyecandy,
                                 m_yAngleEyecandy, &m_referencePoint);
      else if(m_midButtonPressed)
        m_eyecandy->drawZoom(widget, m_clickedAtom, &m_referencePoint);
      else if(m_rightButtonPressed)
        m_eyecandy->drawTranslation(widget, m_clickedAtom, &m_referencePoint);
    }
    return true;
  }

  void NavigateTool::writeSettings(QSettings &settings) const
  {
    Tool::writeSettings(settings);
    settings.setValue("eyeCandyEnabled", m_eyeCandyEnabled);
  }

  void NavigateTool::readSettings(QSettings &settings)
  {
    Tool::readSettings(settings);
    m_eyeCandyEnabled = settings.value("eyeCandyEnabled", true).toBool();

    if (m_settingsWidget)
      m_settingsWidget->eyeCandyCheckBox->setChecked(m_eyeCandyEnabled);
  }

  void NavigateTool::enableEyeCandy(int enable)
  {
    m_eyeCandyEnabled = enable == 2 ? true : false;
  }

}

Q_EXPORT_PLUGIN2(navigatetool, Avogadro::NavigateToolFactory)
