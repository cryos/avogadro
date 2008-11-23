/**********************************************************************
  ManipulateTool - Manipulation Tool for Avogadro

  Copyright (C) 2007 by Marcus D. Hanwell
  Copyright (C) 2007 by Geoffrey R. Hutchison
  Copyright (C) 2007 by Benoit Jacob

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.sourceforge.net/>

  Some code is based on Open Babel
  For more information, see <http://openbabel.sourceforge.net/>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation version 2 of the License.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
 ***********************************************************************/

#include "manipulatetool.h"
#include "eyecandy.h"

#include <avogadro/navigate.h>
#include <avogadro/primitive.h>
#include <avogadro/atom.h>
#include <avogadro/molecule.h>

#include <avogadro/color.h>
#include <avogadro/glwidget.h>
#include <avogadro/camera.h>

#include <QtPlugin>

using Eigen::Vector3d;
using Eigen::Transform3d;
using Eigen::AngleAxisd;

namespace Avogadro {

  ManipulateTool::ManipulateTool(QObject *parent) : Tool(parent),
    m_clickedAtom(0), m_leftButtonPressed(false), m_midButtonPressed(false),
    m_rightButtonPressed(false), m_eyecandy(new Eyecandy)
  {
    m_eyecandy->setColor(Color(1.0, 0.0, 0.0, 1.0));
    QAction *action = activateAction();
    action->setIcon(QIcon(QString::fromUtf8(":/manipulate/manipulate.png")));
    action->setToolTip(tr("Manipulation Tool (F10)\n\n"
          "Left Mouse:   Click and drag to move atoms\n"
          "Middle Mouse: Click and drag to move atoms further away or closer\n"
          "Right Mouse:  Click and drag to rotate selected atoms."));
    action->setShortcut(Qt::Key_F10);
  }

  ManipulateTool::~ManipulateTool()
  {
    delete m_eyecandy;
  }

  int ManipulateTool::usefulness() const
  {
    return 600000;
  }

  void ManipulateTool::zoom(GLWidget *widget, const Eigen::Vector3d *goal,
                            double delta) const
  {
    // Set the cursor - this needs to be reset to Qt::ArrowCursor after
    widget->setCursor(Qt::SizeVerCursor);

    // Move the selected atom(s) in to or out of the screen
    Vector3d transformedGoal = widget->camera()->modelview() * *goal;
    double distanceToGoal = transformedGoal.norm();

    double t = ZOOM_SPEED * delta;
    const double minDistanceToGoal = 2.0 * CAMERA_NEAR_DISTANCE;
    double u = minDistanceToGoal / distanceToGoal - 1.0;

    if( fabs(t) > fabs(u) ) {
      t = u;
    }

    Vector3d atomTranslation = widget->camera()->backTransformedZAxis() * t;

    if (widget->selectedPrimitives().size())
      foreach(Primitive *p, widget->selectedPrimitives())
        if (p->type() == Primitive::AtomType)
          static_cast<Atom *>(p)->setPos(atomTranslation + *static_cast<Atom *>(p)->pos());
    if (m_clickedAtom && !widget->isSelected(m_clickedAtom))
      m_clickedAtom->setPos(atomTranslation + *m_clickedAtom->pos());
    widget->molecule()->update();
  }

  void ManipulateTool::translate(GLWidget *widget, const Eigen::Vector3d *what,
                                 const QPoint &from, const QPoint &to) const
  {
    // Set the cursor - this needs to be reset to Qt::ArrowCursor after
    // Currently, there's a Qt/Mac bug -- SizeAllCursor looks like a spreadsheet cursor
#ifdef Q_WS_MAC
    widget->setCursor(Qt::CrossCursor);
#else
    widget->setCursor(Qt::SizeAllCursor);
#endif

    // Translate the selected atoms in the x and y sense of the view
    Vector3d fromPos = widget->camera()->unProject(from, *what);
    Vector3d toPos = widget->camera()->unProject(to, *what);

    Vector3d atomTranslation = toPos - fromPos;

    if (widget->selectedPrimitives().size())
      foreach(Primitive *p, widget->selectedPrimitives())
        if (p->type() == Primitive::AtomType)
          static_cast<Atom *>(p)->setPos(atomTranslation + *static_cast<Atom *>(p)->pos());
    if (m_clickedAtom && !widget->isSelected(m_clickedAtom))
      m_clickedAtom->setPos(atomTranslation + *m_clickedAtom->pos());
    widget->molecule()->update();
  }

  void ManipulateTool::rotate(GLWidget *widget, const Eigen::Vector3d *center,
                              double deltaX, double deltaY) const
  {
    // Set the cursor - this needs to be reset to Qt::ArrowCursor after
    widget->setCursor(Qt::ClosedHandCursor);

    // Rotate the selected atoms about the center
    // rotate only selected primitives
    Transform3d fragmentRotation;
    fragmentRotation.matrix().setIdentity();
    fragmentRotation.translation() = *center;
    fragmentRotation.rotate(
      AngleAxisd(deltaY * ROTATION_SPEED, widget->camera()->backTransformedXAxis()));
    fragmentRotation.rotate(
      AngleAxisd(deltaX * ROTATION_SPEED, widget->camera()->backTransformedYAxis()));
    fragmentRotation.translate(- *center);

    foreach(Primitive *p, widget->selectedPrimitives())
      if (p->type() == Primitive::AtomType)
        static_cast<Atom *>(p)->setPos(fragmentRotation * *static_cast<Atom *>(p)->pos());
    widget->molecule()->update();
  }

  void ManipulateTool::tilt(GLWidget *widget, const Eigen::Vector3d *center,
                            double delta) const
  {
    // Tilt the selected atoms about the center
    Transform3d fragmentRotation;
    fragmentRotation.matrix().setIdentity();
    fragmentRotation.translation() = *center;
    fragmentRotation.rotate(AngleAxisd(delta * ROTATION_SPEED, widget->camera()->backTransformedZAxis()));
    fragmentRotation.translate(- *center);

    foreach(Primitive *p, widget->selectedPrimitives())
      if (p->type() == Primitive::AtomType)
        static_cast<Atom *>(p)->setPos(fragmentRotation * *static_cast<Atom *>(p)->pos());
    widget->molecule()->update();
  }

  QUndoCommand* ManipulateTool::mousePress(GLWidget *widget, const QMouseEvent *event)
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
      // Currently, there's a Qt/Mac bug -- SizeAllCursor looks like a spreadsheet cursor
#ifdef Q_WS_MAC
      widget->setCursor(Qt::CrossCursor);
#else
      widget->setCursor(Qt::SizeAllCursor);
#endif
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
      widget->setCursor(Qt::ClosedHandCursor);
    }

    m_clickedAtom = widget->computeClickedAtom(event->pos());

    // update eyecandy angle
    m_xAngleEyecandy = 0;
    m_yAngleEyecandy = 0;

    widget->update();

    QUndoCommand* undo = new MoveAtomCommand(widget->molecule());
    return undo;
  }

  QUndoCommand* ManipulateTool::mouseRelease(GLWidget *widget, const QMouseEvent*)
  {
    m_leftButtonPressed = false;
    m_midButtonPressed = false;
    m_rightButtonPressed = false;
    m_clickedAtom = 0;

    // Set the cursor back to the default cursor
    widget->setCursor(Qt::ArrowCursor);

    widget->update();
    QUndoCommand* undo = new MoveAtomCommand(widget->molecule());
    return undo;
  }

  QUndoCommand* ManipulateTool::mouseMove(GLWidget *widget, const QMouseEvent *event)
  {
    if(!widget->molecule())
      return 0;

    // Get the currently selected atoms from the view
    PrimitiveList currentSelection = widget->selectedPrimitives();

    QPoint deltaDragging = event->pos() - m_lastDraggingPosition;

    // Manipulation can be performed in two ways - centred on an individual atom

    // update eyecandy angle
    m_xAngleEyecandy += deltaDragging.x() * ROTATION_SPEED;
    m_yAngleEyecandy += deltaDragging.y() * ROTATION_SPEED;

    if (m_clickedAtom)
    {
      if (m_leftButtonPressed)
      {
        // translate the molecule following mouse movement
        translate(widget, m_clickedAtom->pos(), m_lastDraggingPosition,
                  event->pos());
      }
      else if (m_midButtonPressed)
      {
        if (deltaDragging.y() == 0)
          // Perform the rotation
          tilt(widget, m_clickedAtom->pos(), deltaDragging.x());
        else
          // Perform the zoom toward clicked atom
          zoom(widget, m_clickedAtom->pos(), deltaDragging.y());
      }
      else if (m_rightButtonPressed)
      {
        // Atom centred rotation
        rotate(widget, m_clickedAtom->pos(), deltaDragging.x(),
               deltaDragging.y());
      }
    }
    else if (currentSelection.size())
    {
      // Some atoms are selected - work out where the center is
      m_selectedPrimitivesCenter.setZero();
      int numPrimitives = 0;
      foreach(Primitive *hit, currentSelection)
      {
        if (hit->type() == Primitive::AtomType)
        {
          Atom *atom = static_cast<Atom *>(hit);
          m_selectedPrimitivesCenter += *atom->pos();
          numPrimitives++;
        }
      }
      m_selectedPrimitivesCenter /= numPrimitives;

      if (m_leftButtonPressed)
      {
        // translate the molecule following mouse movement
        translate(widget, &m_selectedPrimitivesCenter, m_lastDraggingPosition,
                  event->pos());
      }
      else if (m_midButtonPressed)
      {
        // Perform the rotation
        tilt(widget, &m_selectedPrimitivesCenter, deltaDragging.x());

        // Perform the zoom toward molecule center
        zoom(widget, &m_selectedPrimitivesCenter, deltaDragging.y());
      }
      else if(m_rightButtonPressed)
      {
        // rotation around the center of the selected atoms
        rotate(widget, &m_selectedPrimitivesCenter, deltaDragging.x(), deltaDragging.y());
      }
    }

    m_lastDraggingPosition = event->pos();
    widget->update();

    return 0;
  }

  QUndoCommand* ManipulateTool::wheel(GLWidget*widget, const QWheelEvent*event)
  {
    // let's set the reference to be the center of the visible
    // part of the molecule.
    Eigen::Vector3d atomsBarycenter(0., 0., 0.);
    double sumOfWeights = 0.;
    QList<Atom*> atoms = widget->molecule()->atoms();
    foreach (Atom *atom, atoms) {
      Eigen::Vector3d transformedAtomPos = widget->camera()->modelview() * *atom->pos();
      double atomDistance = transformedAtomPos.norm();
      double dot = transformedAtomPos.z() / atomDistance;
      double weight = exp(-30. * (1. + dot));
      sumOfWeights += weight;
      atomsBarycenter += weight * *atom->pos();
    }
    atomsBarycenter /= sumOfWeights;

    Navigate::zoom(widget, atomsBarycenter, - MOUSE_WHEEL_SPEED * event->delta());
    widget->update();

    return NULL;
  }

  bool ManipulateTool::paint(GLWidget *widget)
  {
    int selectedSize = widget->selectedPrimitives().size();
    if(m_clickedAtom)
    {
      if(m_leftButtonPressed)
      {
        m_eyecandy->drawTranslation(widget, m_clickedAtom, m_clickedAtom->pos());
      }
      else if(m_midButtonPressed)
      {
        m_eyecandy->drawZoom(widget, m_clickedAtom, m_clickedAtom->pos());
      }
      else if(m_rightButtonPressed && selectedSize)
      {
        m_eyecandy->drawRotation(widget, m_clickedAtom,
            m_xAngleEyecandy, m_yAngleEyecandy, m_clickedAtom->pos());
      }
    }
    else if(selectedSize)
    {
      if(m_leftButtonPressed)
      {
        m_eyecandy->drawTranslation(widget, &m_selectedPrimitivesCenter, 1.5, 0.);
      }
      else if(m_midButtonPressed)
      {
        m_eyecandy->drawZoom(widget, &m_selectedPrimitivesCenter, 1.5);
      }
      else if(m_rightButtonPressed)
      {
        m_eyecandy->drawRotation(widget, &m_selectedPrimitivesCenter, 3.,
            m_xAngleEyecandy, m_yAngleEyecandy);
      }
    }

    return true;
  }

  MoveAtomCommand::MoveAtomCommand(Molecule *molecule, QUndoCommand *parent) : QUndoCommand(parent), m_molecule(0)
  {
    // Store the molecule - this call won't actually move an atom
    setText(QObject::tr("Manipulate Atom"));
    m_moleculeCopy = *molecule;
    m_molecule = molecule;
    undone = false;
  }

  MoveAtomCommand::MoveAtomCommand(Molecule *molecule, int type, QUndoCommand *parent) : QUndoCommand(parent), m_molecule(0)
  {
    // Store the original molecule before any modifications are made
    setText(QObject::tr("Manipulate Atom"));
    m_moleculeCopy = *molecule;
    m_molecule = molecule;
    m_type =type;
    undone = false;
  }

  void MoveAtomCommand::redo()
  {
    // Move the specified atom to the location given
    if (undone)
    {
      Molecule newMolecule = *m_molecule;
      *m_molecule = m_moleculeCopy;
      m_moleculeCopy = newMolecule;
    }
    QUndoCommand::redo();
  }

  void MoveAtomCommand::undo()
  {
    // Restore our original molecule
    Molecule newMolecule = *m_molecule;
    *m_molecule = m_moleculeCopy;
    m_moleculeCopy = newMolecule;
    undone = true;
  }

  bool MoveAtomCommand::mergeWith (const QUndoCommand *)
  {
    // Just return true to repeated calls - we have stored the original molecule
    return true;
  }

  int MoveAtomCommand::id() const
  {
    return 26011980;
  }
}

#include "manipulatetool.moc"

Q_EXPORT_PLUGIN2(manipulatetool, Avogadro::ManipulateToolFactory)
