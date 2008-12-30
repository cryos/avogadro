/**********************************************************************
  BondCentricTool - Bond Centric Manipulation Tool for Avogadro

  Copyright (C) 2007 by Shahzad Ali
  Copyright (C) 2007 by Ross Braithwaite
  Copyright (C) 2007 by James Bunt
  Copyright (C) 2007,2008 by Marcus D. Hanwell
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

#include "bondcentrictool.h"
#include "quaternion.h"

#include <iostream>

#include <avogadro/navigate.h>
#include <avogadro/atom.h>
#include <avogadro/bond.h>
#include <avogadro/color.h>
#include <avogadro/glwidget.h>
#include <avogadro/camera.h>
#include <avogadro/toolgroup.h>

#include <QtPlugin>
#include <QString>

#include <QDebug>

using namespace std;
using namespace Eigen;

namespace Avogadro {

  // ############################ BondCentricTool ################################

  // ##########  Constructor  ##########

  BondCentricTool::BondCentricTool(QObject *parent) : Tool(parent),
  m_molecule(NULL),
  m_settingsWidget(NULL),
  m_clickedAtom(NULL),
  m_clickedBond(NULL),
  m_selectedBond(NULL),
  m_skeleton(NULL),
  m_referencePoint(NULL),
  m_currentReference(NULL),
  m_directionVector(NULL),
  m_snapped(false),
  m_toolGroup(NULL),
  m_leftButtonPressed(false),
  m_midButtonPressed(false),
  m_rightButtonPressed(false),
  m_movedSinceButtonPressed(false),
  m_showAngles(true),
  m_snapToEnabled(true),
  m_snapToAngle(10)
  {
    QAction *action = activateAction();
    action->setIcon(QIcon(QString::fromUtf8(":/bondcentric/bondcentric.png")));
    action->setToolTip(tr("Bond Centric Manipulation Tool\n\n"
          "Left Mouse:   Click and drag to rotate the view\n"
          "Middle Mouse: Click and drag to zoom in or out\n"
          "Right Mouse:  Click and drag to move the view\n\n"
          "Left Click & drag on a Bond to set the Manipulation Plane:\n"
          "- Left Click & Drag one of the Atoms in the Bond to change the angle\n"
          "- Right Click & Drag one of the Atoms in the Bond to change the length"));
    //action->setShortcut(Qt::Key_F9);
    connect(action,SIGNAL(toggled(bool)),this,SLOT(toolChanged(bool)));
  }

  // ##########  Desctructor  ##########

  BondCentricTool::~BondCentricTool()
  {
    delete m_referencePoint;
    m_referencePoint = NULL;
    delete m_currentReference;
    m_currentReference = NULL;
    delete m_directionVector;
    m_directionVector = NULL;

    if (m_settingsWidget)
    {
      m_snapToAngleLabel->deleteLater();
      m_spacer->deleteLater();
      m_showAnglesBox->deleteLater();
      m_snapToCheckBox->deleteLater();
      m_snapToAngleBox->deleteLater();
      m_layout->deleteLater();

      m_settingsWidget->deleteLater();
    }
  }

  // ##########  clearData  ##########

  void BondCentricTool::clearData()
  {
    m_clickedAtom = NULL;
    m_clickedBond = NULL;
    m_selectedBond = NULL;
    delete m_referencePoint;
    m_referencePoint = NULL;
    delete m_currentReference;
    m_currentReference = NULL;
    delete m_directionVector;
    m_directionVector = NULL;
    m_toolGroup = NULL;
    m_leftButtonPressed = false;
    m_midButtonPressed = false;
    m_rightButtonPressed = false;
    m_movedSinceButtonPressed = false;
    m_snapped = false;
  }

  // ##########  moleculeChanged  ##########

  void BondCentricTool::setMolecule(Molecule* molecule)
  {
    if(m_molecule) {
      disconnect(m_molecule, 0 , this, 0);
    }

    if (molecule) {
      m_molecule = molecule;
      connect(molecule, SIGNAL(primitiveRemoved(Primitive*)), this,
          SLOT(primitiveRemoved(Primitive*)));
    }

    clearData();
  }

  // ##########  primitiveRemoved  ##########

  void BondCentricTool::primitiveRemoved(Primitive *primitive)
  {
    if (primitive == m_clickedAtom || primitive == m_clickedBond ||
        primitive == m_selectedBond) {
      clearData();
    }
  }

  // ##########  toolChanged  ##########

  void BondCentricTool::toolChanged(bool checked)
  {
    if(!checked && m_molecule)
    {
      m_molecule->update();
      clearData();
    }
  }

  // ##########  usefulness  ##########

  int BondCentricTool::usefulness() const
  {
    return 2000000;
  }

  // ##########  mousePress  ##########

  QUndoCommand* BondCentricTool::mousePressEvent(GLWidget *widget, QMouseEvent *event)
  {
    m_undo = 0;
    event->accept();
    Molecule *molecule = widget->molecule();

    m_lastDraggingPosition = event->pos();
    m_movedSinceButtonPressed = false;

#ifdef Q_WS_MAC
    m_leftButtonPressed = (event->buttons() & Qt::LeftButton
        && event->modifiers() == Qt::NoModifier);
    // On the Mac, either use a three-button mouse
    // or hold down the Shift key
    m_midButtonPressed = ((event->buttons() & Qt::MidButton) ||
        (event->buttons() & Qt::LeftButton && event->modifiers()
         & Qt::ShiftModifier));
    // Hold down the Command key (ControlModifier in Qt notation) for right button
    m_rightButtonPressed = ((event->buttons() & Qt::RightButton) ||
        (event->buttons() & Qt::LeftButton &&
         (event->modifiers() == Qt::ControlModifier ||
          event->modifiers() == Qt::MetaModifier)));
#else
    m_leftButtonPressed = (event->buttons() & Qt::LeftButton);
    m_midButtonPressed = (event->buttons() & Qt::MidButton);
    m_rightButtonPressed = (event->buttons() & Qt::RightButton);
#endif

    m_clickedAtom = NULL;
    m_clickedBond = NULL;

    int oldName = m_selectedBond ? m_selectedBond->index() : -1;

    // Check if the mouse clicked on any Atoms or Bonds.
    Primitive *clickedPrim = widget->computeClickedPrimitive(event->pos());

    if (clickedPrim && clickedPrim->type() == Primitive::AtomType)
    {
      // Atom clicked on.
      m_clickedAtom = (Atom*)clickedPrim;

      if ((m_rightButtonPressed || m_leftButtonPressed) && isAtomInBond(m_clickedAtom, m_selectedBond))
      {
        //Create an undo instance for this manipulation
        m_undo = new BondCentricMoveCommand(widget->molecule());

        // Populate the skeleton in preparation to alter the angle or length of the bond.
        m_skeleton = new SkeletonTree();
        m_skeleton->populate(m_clickedAtom, m_selectedBond, widget->molecule());

        if (m_leftButtonPressed)
        {
          // If the bond is to be rotated, save off a vector used in the calculation.
          // If the vector is calculated every mouse movement it causes the molecule to
          // jitter, saving it now prevents this.

          Atom *otherAtom;
          if (m_clickedAtom == molecule->atomById(m_selectedBond->beginAtomId()))
            otherAtom = molecule->atomById(m_selectedBond->endAtomId());
          else
            otherAtom = molecule->atomById(m_selectedBond->beginAtomId());

          Vector3d centerProj = widget->camera()->project(*otherAtom->pos());
          centerProj -= Vector3d(0,0,centerProj.z());
          Vector3d clickedProj = widget->camera()->project(*m_clickedAtom->pos());
          clickedProj -= Vector3d(0,0,clickedProj.z());

          if ((clickedProj - centerProj).norm() == 0)
          {
            // Have no way of testing this as the chance of this happening is almost 0
            m_directionVector = new Vector3d(1, 0, 0);
          }
          else
          {
            m_directionVector = new Vector3d(clickedProj - centerProj);
            *m_directionVector = m_directionVector->normalized();
          }
        }
      }
      else if (m_selectedBond)
      {
        // Check if one of the atoms one bond away from the selected bond has
        // been clicked on in preparation to alter dihedral angles.

        Atom *dihedralRotCen = NULL;
        Bond *skeleBond = NULL;

        Atom *beginAtom = molecule->atomById(m_selectedBond->beginAtomId());
        Atom *endAtom = molecule->atomById(m_selectedBond->endAtomId());

        // Check which atom in the selected bond the atom being manipulated will
        // be rotated around to change its dihedral angle.
        if ((skeleBond = molecule->bond(m_clickedAtom, beginAtom)))
          dihedralRotCen = beginAtom;
        else if ((skeleBond = molecule->bond(m_clickedAtom, endAtom)))
          dihedralRotCen = endAtom;

        bool skeletonSet = false;
        if (m_rightButtonPressed && skeleBond)
        {
          //Create an undo instance for this manipulation
          m_undo = new BondCentricMoveCommand(widget->molecule());

          // Populate the skeleton in preparation to alter the dihedral angle of the
          // clicked atom.
          m_skeleton = new SkeletonTree();
          m_skeleton->populate(m_clickedAtom, skeleBond, widget->molecule());
          skeletonSet = true;
        }
        else if (m_leftButtonPressed && dihedralRotCen)
        {
          //Create an undo instance for this manipulation
          m_undo = new BondCentricMoveCommand(widget->molecule());

          // Populate the skeleton in preparation to alter the dihedral angle of all
          // the atoms bonded to this end of the bond (essentially twisting this end
          // of the bond).
          m_skeleton = new SkeletonTree();
          m_skeleton->populate(dihedralRotCen, m_selectedBond, widget->molecule());
          skeletonSet = true;
        }

        if (skeletonSet)
        {
          // If the dihedral angle is to be changed, save off a vector used in the
          // calculation. If the vector is calculated every mouse movement it causes
          // the molecule to jitter, saving it now prevents this.

          Vector3d centerProj = widget->camera()->project(*dihedralRotCen->pos());
          centerProj -= Vector3d(0,0,centerProj.z());
          Vector3d clickedProj = widget->camera()->project(*m_clickedAtom->pos());
          clickedProj -= Vector3d(0,0,clickedProj.z());

          if ((clickedProj - centerProj).norm() == 0)
          {
            // Have no way of testing this as the chance of this happening is almost 0
            m_directionVector = new Vector3d(1, 0, 0);
          }
          else
          {
            m_directionVector = new Vector3d(clickedProj - centerProj);
            *m_directionVector = m_directionVector->normalized();
          }
        }
      }
    }
    else if (clickedPrim && clickedPrim->type() == Primitive::BondType)
    {
      // Bond clicked on.
      m_clickedBond = static_cast<Bond *>(clickedPrim);

      // If the Bond was clicked on with the left mouse button, set it as the
      // currently selected bond and reset the reference point (if the Bond has
      // changed).
      if (m_leftButtonPressed)
      {
        m_selectedBond = m_clickedBond;

        if (m_selectedBond->index() != oldName)
        {
          delete m_referencePoint;
          m_referencePoint = NULL;

          delete m_currentReference;
          m_currentReference = NULL;

          m_snapped = false;

          Atom *leftAtom = molecule->atomById(m_selectedBond->beginAtomId());
          Atom *rightAtom = molecule->atomById(m_selectedBond->endAtomId());

          Vector3d left = *leftAtom->pos();
          Vector3d right = *rightAtom->pos();
          Vector3d leftToRight = right - left;

          Vector3d x = Vector3d(1, 0, 0);
          Vector3d y = Vector3d(0, 1, 0);

          Vector3d A = leftToRight.cross(x);
          Vector3d B = leftToRight.cross(y);

          m_referencePoint = A.norm() >= B.norm() ? new Vector3d(A) : new Vector3d(B);
          *m_referencePoint = m_referencePoint->normalized();

          Vector3d *reference = calculateSnapTo(m_selectedBond,
              m_referencePoint, m_snapToAngle);

          if (reference && m_snapToEnabled)
          {
            m_snapped = true;
            m_currentReference = reference;
            *m_currentReference = m_currentReference->normalized();
          }
          else {
            m_currentReference = new Vector3d(*m_referencePoint);
          }
        }
      }
    }

    widget->update();
    return 0;
  }

  // ##########  mouseRelease  ##########

  QUndoCommand* BondCentricTool::mouseReleaseEvent(GLWidget *widget, QMouseEvent *event)
  {
    event->accept();
    delete m_directionVector;
    m_directionVector = NULL;

    if (!m_clickedAtom && !m_clickedBond && !m_movedSinceButtonPressed)
    {
      delete m_referencePoint;
      m_referencePoint = NULL;
      delete m_currentReference;
      m_currentReference = NULL;
      m_snapped = false;
      m_selectedBond = NULL;
    }
    else if (!m_movedSinceButtonPressed) {
      m_undo = 0;
    }

    if (m_skeleton)
    {
      delete m_skeleton;
      m_skeleton = NULL;
    }

    m_leftButtonPressed = false;
    m_midButtonPressed = false;
    m_rightButtonPressed = false;
    m_clickedAtom = NULL;
    m_clickedBond = NULL;

    widget->update();
    return m_undo;
  }

  // ##########  mouseMove  ##########

  QUndoCommand* BondCentricTool::mouseMoveEvent(GLWidget *widget, QMouseEvent *event)
  {
    if (!m_molecule) {
      return 0;
    }
    event->accept();

    Molecule *molecule = widget->molecule();

    QPoint deltaDragging = event->pos() - m_lastDraggingPosition;

    if (deltaDragging.manhattanLength() > 2) {
      m_movedSinceButtonPressed = true;
    }

    // Mouse navigation has two modes - atom centred when an atom is clicked
    // and scene if no atom has been clicked.

#ifdef Q_WS_MAC
    if (event->buttons() & Qt::LeftButton && event->modifiers() == Qt::NoModifier)
#else
      if (event->buttons() & Qt::LeftButton)
#endif
      {
        if (m_clickedBond && m_selectedBond && m_referencePoint)
        {
          Atom *beginAtom = molecule->atomById(m_selectedBond->beginAtomId());
          Atom *endAtom = molecule->atomById(m_selectedBond->endAtomId());

          Vector3d rotationVector = *endAtom->pos() - *beginAtom->pos();
          rotationVector = rotationVector / rotationVector.norm();

          Vector3d begin = widget->camera()->project(*beginAtom->pos());
          Vector3d end = widget->camera()->project(*endAtom->pos());

          Vector3d zAxis = Vector3d(0, 0, 1);
          Vector3d beginToEnd = end - begin;
          beginToEnd -= Vector3d(0, 0, beginToEnd.z());

          Vector3d direction = zAxis.cross(beginToEnd);
          direction = direction / direction.norm();

          Vector3d mouseMoved = Vector3d(deltaDragging.x(), deltaDragging.y(), 0);

          double magnitude = mouseMoved.dot(direction) / direction.norm();

          *m_referencePoint = performRotation(magnitude * (M_PI / 180.0),
              rotationVector, Vector3d(0, 0, 0),
              *m_referencePoint);

          Eigen::Vector3d *reference = calculateSnapTo(m_selectedBond,
              m_referencePoint, m_snapToAngle);
          if (reference && m_snapToEnabled)
          {
            m_snapped = true;
            delete m_currentReference;
            m_currentReference = reference;
            *m_currentReference = m_currentReference->normalized();
          }
          else
          {
            m_snapped = false;
            delete m_currentReference;
            m_currentReference = new Vector3d(*m_referencePoint);
          }
        }
        else if (isAtomInBond(m_clickedAtom, m_selectedBond))
        {
          //Do atom rotation.
          Atom *otherAtom;

          if (m_clickedAtom == molecule->atomById(m_selectedBond->beginAtomId()))
            otherAtom = molecule->atomById(m_selectedBond->endAtomId());
          else
            otherAtom = molecule->atomById(m_selectedBond->beginAtomId());

          Vector3d center = *otherAtom->pos();
          Vector3d clicked = *m_clickedAtom->pos();

          Vector3d centerProj = widget->camera()->project(center);
          centerProj -= Vector3d(0,0,centerProj.z());
          Vector3d referenceProj = widget->camera()->project(*m_currentReference + center);
          referenceProj -= Vector3d(0,0,referenceProj.z());

          Vector3d referenceVector = referenceProj - centerProj;
          referenceVector = referenceVector.normalized();

          Vector3d rotationVector = referenceVector.cross(*m_directionVector);
          rotationVector = rotationVector.normalized();

          Vector3d currMouseVector = Vector3d(event->pos().x(), event->pos().y(), 0)
            - centerProj;
          if(currMouseVector.norm() > 5)
          {
            currMouseVector = currMouseVector.normalized();
            double mouseAngle = acos(m_directionVector->dot(currMouseVector) /
                currMouseVector.squaredNorm());

            if(mouseAngle > 0)
            {
              Vector3d tester;

              tester = performRotation(mouseAngle, rotationVector, Vector3d(0, 0, 0),
                  *m_directionVector);
              double testAngle1 = acos(tester.dot(currMouseVector) /
                  currMouseVector.squaredNorm());

              tester = performRotation(-mouseAngle, rotationVector, Vector3d(0, 0, 0),
                  *m_directionVector);
              double testAngle2 = acos(tester.dot(currMouseVector) /
                  currMouseVector.squaredNorm());

              if(testAngle1 > testAngle2 || OpenBabel::IsNan((double)testAngle2)) {
                mouseAngle = -mouseAngle;
              }

              Vector3d direction = clicked - center;
              if (m_skeleton)
              {
                Vector3d currCrossDir = m_currentReference->cross(direction).normalized();

                m_skeleton->skeletonRotate(mouseAngle, currCrossDir, center);
                *m_referencePoint = performRotation(mouseAngle, currCrossDir,
                    Vector3d(0, 0, 0), *m_referencePoint);
                *m_currentReference = performRotation(mouseAngle, currCrossDir,
                    Vector3d(0, 0, 0), *m_currentReference);
                *m_directionVector = performRotation(mouseAngle, rotationVector,
                    Vector3d(0, 0, 0), *m_directionVector);
              }
            }
          }
        }
        else if (m_selectedBond && m_clickedAtom &&
            (molecule->bond(m_clickedAtom->id(), m_selectedBond->beginAtomId()) ||
             molecule->bond(m_clickedAtom->id(), m_selectedBond->endAtomId())))
        {
          // Do multiple dihedral rotation (twising of the bond).

          Atom *beginAtom = molecule->atomById(m_selectedBond->beginAtomId());
          Atom *endAtom = molecule->atomById(m_selectedBond->endAtomId());

          Vector3d center;
          Vector3d other;
          if (molecule->bond(m_clickedAtom, beginAtom))
          {
            center = *beginAtom->pos();
            other = *endAtom->pos();
          }
          else {
            center = *endAtom->pos();
            other = *beginAtom->pos();
          }

          Vector3d clicked = *m_clickedAtom->pos();

          Vector3d axis = Vector3d(0, 0, ((widget->camera()->modelview() * other).z() >=
                (widget->camera()->modelview() * center).z() ? -1 : 1));

          Vector3d centerProj = widget->camera()->project(center);
          centerProj -= Vector3d(0,0,centerProj.z());

          Vector3d currMouseVector = Vector3d(event->pos().x(), event->pos().y(), 0)
            - centerProj;

          if(currMouseVector.norm() > 5)
          {
            currMouseVector = currMouseVector.normalized();
            double mouseAngle = acos(m_directionVector->dot(currMouseVector) /
                currMouseVector.squaredNorm());

            if(mouseAngle > 0)
            {
              Vector3d tester;

              tester = performRotation(mouseAngle, axis, Vector3d(0, 0, 0), *m_directionVector);
              double testAngle1 = acos(tester.dot(currMouseVector) /
                  currMouseVector.squaredNorm());

              tester = performRotation(-mouseAngle, axis, Vector3d(0, 0, 0), *m_directionVector);
              double testAngle2 = acos(tester.dot(currMouseVector) /
                  currMouseVector.squaredNorm());

              if(testAngle1 > testAngle2 || OpenBabel::IsNan((double)testAngle2)) {
                mouseAngle = -mouseAngle;
              }

              if (m_skeleton)
              {
                *m_directionVector = performRotation(mouseAngle, axis,
                    Vector3d(0, 0, 0), *m_directionVector);

                axis = (other - center).normalized();
                m_skeleton->skeletonRotate(mouseAngle, axis, center);
              }
            }
          }
        }
        else {
          // rotation around the center of the molecule
          Navigate::rotate(widget, widget->center(), deltaDragging.x(), deltaDragging.y());
        }
      }
#ifdef Q_WS_MAC
    // On the Mac, either use a three-button mouse
    // or hold down the Shift key
      else if ((event->buttons() & Qt::MidButton) || (event->buttons() &
            Qt::LeftButton && event->modifiers() & Qt::ShiftModifier))
#else
      else if (event->buttons() & Qt::MidButton)
#endif
      {
        if (m_clickedAtom)
        {
          // Perform the rotation
          Navigate::tilt(widget, *m_clickedAtom->pos(), deltaDragging.x());

          // Perform the zoom toward the center of a clicked atom
          Navigate::zoom(widget, *m_clickedAtom->pos(), deltaDragging.y());
        }
        else if (m_clickedBond)
        {
          Atom *begin = molecule->atomById(m_clickedBond->beginAtomId());
          Atom *end = molecule->atomById(m_clickedBond->endAtomId());

          Vector3d btoe = *end->pos() - *begin->pos();
          double newLen = btoe.norm() / 2;
          btoe = btoe / btoe.norm();

          Vector3d mid = *begin->pos() + btoe * newLen;

          // Perform the rotation
          Navigate::tilt(widget, mid, deltaDragging.x());

          // Perform the zoom toward the centre of a clicked bond
          Navigate::zoom(widget, mid, deltaDragging.y());
        }
        else
        {
          // Perform the rotation
          Navigate::tilt(widget, widget->center(), deltaDragging.x());

          // Perform the zoom toward molecule center
          Navigate::zoom(widget, widget->center(), deltaDragging.y());
        }
      }
#ifdef Q_WS_MAC
    // On the Mac, either use a three-button mouse
    // or hold down the Command key (ControlModifier in Qt notation)
      else if ((event->buttons() & Qt::RightButton) ||
          (event->buttons() & Qt::LeftButton &&
           (event->modifiers() == Qt::ControlModifier || event->modifiers() == Qt::MetaModifier)))
#else
      else if (event->buttons() & Qt::RightButton)
#endif
      {
        if (isAtomInBond(m_clickedAtom, m_selectedBond))
        {
          // Adjust the length of the bond following the mouse movement.

          Atom *otherAtom;

          if (m_clickedAtom == molecule->atomById(m_selectedBond->beginAtomId()))
            otherAtom = molecule->atomById(m_selectedBond->endAtomId());
          else
            otherAtom = molecule->atomById(m_selectedBond->beginAtomId());

          Vector3d clicked = *m_clickedAtom->pos();
          Vector3d other = *otherAtom->pos();
          Vector3d direction = clicked - other;

          Vector3d mouseLast = widget->camera()->unProject(m_lastDraggingPosition);
          Vector3d mouseCurr = widget->camera()->unProject(event->pos());
          Vector3d mouseDir = mouseCurr - mouseLast;

          Vector3d component = mouseDir.dot(direction) / direction.squaredNorm()
                               * direction;

          if (m_skeleton) {
            m_skeleton->skeletonTranslate(component.x(), component.y(), component.z());
          }
        }
        else if (m_selectedBond && m_clickedAtom &&
            (molecule->bond(m_clickedAtom->id(), m_selectedBond->beginAtomId()) ||
             molecule->bond(m_clickedAtom->id(), m_selectedBond->endAtomId())))
        {
          // Do dihedral angle manipulation of the clicked atom.

          Atom *beginAtom = molecule->atomById(m_selectedBond->beginAtomId());
          Atom *endAtom = molecule->atomById(m_selectedBond->endAtomId());

          Vector3d center;
          Vector3d other;
          if (molecule->bond(m_clickedAtom, beginAtom)) {
            center = *beginAtom->pos();
            other = *endAtom->pos();
          }
          else {
            center = *endAtom->pos();
            other = *beginAtom->pos();
          }

          Vector3d clicked = *m_clickedAtom->pos();

          Vector3d axis = Vector3d(0, 0, ((widget->camera()->modelview() * other).z() >=
                (widget->camera()->modelview() * center).z() ? -1 : 1));

          Vector3d centerProj = widget->camera()->project(center);
          centerProj -= Vector3d(0,0,centerProj.z());

          Vector3d currMouseVector = Vector3d(event->pos().x(), event->pos().y(), 0)
            - centerProj;

          if(currMouseVector.norm() > 5)
          {
            currMouseVector = currMouseVector.normalized();
            double mouseAngle = acos(m_directionVector->dot(currMouseVector) /
                currMouseVector.squaredNorm());

            if(mouseAngle > 0)
            {
              Vector3d tester;

              tester = performRotation(mouseAngle, axis, Vector3d(0, 0, 0), *m_directionVector);
              double testAngle1 = acos(tester.dot(currMouseVector) /
                  currMouseVector.squaredNorm());

              tester = performRotation(-mouseAngle, axis, Vector3d(0, 0, 0), *m_directionVector);
              double testAngle2 = acos(tester.dot(currMouseVector) /
                  currMouseVector.squaredNorm());

              if(testAngle1 > testAngle2 || OpenBabel::IsNan((double)testAngle2)) {
                mouseAngle = -mouseAngle;
              }

              if (m_skeleton)
              {
                *m_directionVector = performRotation(mouseAngle, axis,
                    Vector3d(0, 0, 0), *m_directionVector);

                axis = (other - center).normalized();
                m_skeleton->skeletonRotate(mouseAngle, axis, center);
              }
            }
          }
        }
        else {
          // Translate the molecule following mouse movement.
          Navigate::translate(widget, widget->center(), m_lastDraggingPosition, event->pos());
        }
      }

    m_lastDraggingPosition = event->pos();
    widget->update();

    return 0;
  }

  // ##########  wheel  ##########

  QUndoCommand* BondCentricTool::wheelEvent(GLWidget *widget, QWheelEvent *event)
  {
    return 0;
  }

  // ##########  paint  ##########

  bool BondCentricTool::paint(GLWidget *widget)
  {
    if(widget->toolGroup()->activeTool() != this) {
      clearData();
    }

    Molecule *molecule = widget->molecule();

    bool dihedralAtomClicked = false;

    // Draw the dihedral angles and rectangles if they apply.
    if ((m_leftButtonPressed || m_rightButtonPressed) && m_clickedAtom &&
        m_selectedBond && !isAtomInBond(m_clickedAtom, m_selectedBond))
    {
      Atom *begin = molecule->atomById(m_selectedBond->beginAtomId());
      Atom *end = molecule->atomById(m_selectedBond->endAtomId());

      if (molecule->bond(m_clickedAtom, begin) ||
          molecule->bond(m_clickedAtom, end))
      {
        dihedralAtomClicked = true;

        if (m_rightButtonPressed) {
          drawSingleDihedralAngles(widget, m_clickedAtom, m_selectedBond);
        } else {
          drawDihedralAngles(widget, m_clickedAtom, m_selectedBond);
        }
      }
    }

    // The small yellow sphere that resembles the center of the molecule during rotation.
    if (!dihedralAtomClicked &&
        ((m_leftButtonPressed && !m_clickedBond && !isAtomInBond(m_clickedAtom, m_selectedBond))
         || (m_midButtonPressed && !m_clickedBond && !m_clickedAtom)
         || (m_rightButtonPressed && !isAtomInBond(m_clickedAtom, m_selectedBond))))
    {
      drawSphere(widget, widget->center(), 0.10, 1.0);
    }

    // If a single atom was clicked on, display the angles around it.
    if (!dihedralAtomClicked && m_leftButtonPressed && m_clickedAtom &&
        (!m_selectedBond || !isAtomInBond(m_clickedAtom, m_selectedBond)))
    {
      drawAtomAngles(widget, m_clickedAtom);
    }

    // Draw the manipulation rectangle and relative angles.
    if (m_selectedBond && !dihedralAtomClicked)
    {
      Atom *begin = molecule->atomById(m_selectedBond->beginAtomId());
      Atom *end = molecule->atomById(m_selectedBond->endAtomId());

      if (m_currentReference)
      {
        // Draw bond length text.
        QString length = tr("Bond Length:  ") +
          QString::number(m_selectedBond->length(), 10, 1) +
          QString::fromUtf8(" Å (Angstrom)");

        glColor4f(1.0, 1.0, 1.0, 1.0);
        widget->painter()->setColor(1.0, 1.0, 1.0, 1.0);
        widget->painter()->drawText(QPoint(5, widget->height() - 25), length);

        if (m_rightButtonPressed && (m_clickedAtom == begin || m_clickedAtom == end)) {
          drawSkeletonAngles(widget, m_skeleton);
        }
        else
        {
          if (m_showAngles && !dihedralAtomClicked)
          {
            // Draw the angles around the two atoms.
            if (!m_clickedAtom || m_rightButtonPressed || m_midButtonPressed ||
                (m_leftButtonPressed && begin != m_clickedAtom)) {
              drawAngles(widget, begin, m_selectedBond);
            }

            if (!m_clickedAtom || m_rightButtonPressed || m_midButtonPressed ||
                (m_leftButtonPressed && end != m_clickedAtom)) {
              drawAngles(widget, end, m_selectedBond);
            }
          }
          else
          {
            // Draw the angles around the two atoms.
            if (m_leftButtonPressed && end == m_clickedAtom) {
              drawAngles(widget, begin, m_selectedBond);
            }

            if (m_leftButtonPressed && begin == m_clickedAtom) {
              drawAngles(widget, end, m_selectedBond);
            }
          }

          if (m_clickedAtom && m_leftButtonPressed &&
              isAtomInBond(m_clickedAtom, m_selectedBond)) {
            drawSkeletonAngles(widget, m_skeleton);
          }
        }

        // Draw the manipulation rectangle.
        if (m_snapped && m_snapToEnabled)
        {
          double rgb[3] = {1.0, 1.0, 0.2};
          drawManipulationRectangle(widget, m_selectedBond, m_currentReference, rgb);
        }
        else
        {
          double rgb[3] = {0.0, 0.2, 0.8};
          drawManipulationRectangle(widget, m_selectedBond, m_currentReference, rgb);
        }
      }
    }

    return true;
  }

  // ##########  isAtomInBond  ##########

  bool BondCentricTool::isAtomInBond(Atom *atom, Bond *bond)
  {
    if (!atom || !bond) {
      return false;
    }

    if (atom->id() == bond->beginAtomId() || atom->id() == bond->endAtomId())
      return true;
    else
      return false;
  }

  // ##########  drawAngleSector  ##########

  void BondCentricTool::drawAngleSector(GLWidget *widget, Eigen::Vector3d origin,
      Eigen::Vector3d direction1, Eigen::Vector3d direction2,
      bool alternateAngle)
  {
    // Get vectors representing the lines from centre to left and centre to right.
    Eigen::Vector3d u = direction1 - origin;
    Eigen::Vector3d v = direction2 - origin;

    // Calculate the length of the vectors (half the length of the shortest vector.)
    double radius = qMin(u.norm(), v.norm()) * 0.5;
    double lineWidth = 1.5;

    // Adjust the length of u and v to the length calculated above.
    u = (u / u.norm()) * radius;
    v = (v / v.norm()) * radius;

    // Angle between u and v.
    double uvAngle = acos(u.dot(v) / v.squaredNorm()) * 180.0 / M_PI;

    // If angle is less than 1 (will be approximated to 0), attempting to draw
    // will crash, so return.
    if (abs(uvAngle) <= 1) {
      return;
    }

    if (alternateAngle) {
      uvAngle = 360.0 - (uvAngle > 0 ? uvAngle : -uvAngle);
    }

    // Vector perpindicular to both u and v.
    Eigen::Vector3d n = u.cross(v);

    Eigen::Vector3d x = Vector3d(1, 0, 0);
    Eigen::Vector3d y = Vector3d(0, 1, 0);

    if (n.norm() < 1e-16)
    {
      Eigen::Vector3d A = u.cross(x);
      Eigen::Vector3d B = u.cross(y);

      n = A.norm() >= B.norm() ? A : B;
    }

    n = n / n.norm();

    Vector3d point = performRotation((uvAngle / 2 * (M_PI / 180.0)), n,
        Vector3d(0, 0, 0), alternateAngle ? v : u);

    QString angle = QString::number(uvAngle, 10, 1) + QString::fromUtf8("°");
    glColor4f(1.0, 1.0, 1.0, 1.0);
    widget->painter()->setColor(1.0, 1.0, 1.0, 1.0);
    widget->painter()->drawText(point + origin, angle);

    glEnable(GL_BLEND);
    widget->painter()->setColor(0, 0.5, 0, 0.4);
    glDepthMask(GL_FALSE);
    widget->painter()->drawShadedSector(origin, direction1, direction2, radius, alternateAngle);
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);

    widget->painter()->setColor(1.0, 1.0, 1.0, 1.0);
    widget->painter()->drawArc(origin, direction1, direction2, radius, lineWidth, alternateAngle);
  }

  // ##########  drawAtomAngles  ##########

  void BondCentricTool::drawAtomAngles(GLWidget *widget, Atom *atom)
  {
    if (!atom || !widget) {
      return;
    }

    QList<unsigned long> neighbors = atom->neighbors();
    if (neighbors.size() > 1) {
      Atom *u = 0;
      Atom *v = 0;
      foreach (unsigned long a, neighbors) {
        v = m_molecule->atomById(a);
        if (u)
          drawAngleSector(widget, *atom->pos(), *u->pos(), *v->pos());
        u = v;
      }
    }
  }

  // ##########  drawAngles  ##########

  void BondCentricTool::drawAngles(GLWidget *widget, Atom *atom, Bond *bond)
  {
    if (!atom || !bond || !widget || !isAtomInBond(atom, bond)) {
      return;
    }

    Atom *ref = NULL;
    if (atom == bond->beginAtom())
      ref = bond->endAtom();
    else if (atom == bond->endAtom())
      ref = bond->beginAtom();
    else
      return;

    Atom *v = 0;
    QList<unsigned long> neighbors = atom->neighbors();
    foreach (unsigned long a, neighbors) {
      v = m_molecule->atomById(a);
      if (v == ref)
          continue;
      drawAngleSector(widget, *atom->pos(), *ref->pos(), *v->pos());
    }
  }

  // ##########  drawSkeletonAngles  ##########

  void BondCentricTool::drawSkeletonAngles(GLWidget *widget, SkeletonTree *skeleton)
  {
    if (!skeleton || !widget) {
      return;
    }

    Atom *atom = skeleton->rootAtom();
    Bond *bond = skeleton->rootBond();

    drawAngles(widget, atom, bond);
  }

  // ##########  drawDihedralAngle  ##########

  // Dihedral angle between atoms A & D for the four atoms A-B-C-D
  void BondCentricTool::drawDihedralAngle(GLWidget *widget, Atom *A, Atom *D,
                                          Bond *BC, bool alternateAngle)
  {
    if (!A || !D || !BC || !widget) {
      return;
    }

    Atom *B = BC->beginAtom();
    Atom *C = BC->endAtom();

    if (!A->bond(B) || !D->bond(C)) {
      B = BC->endAtom();
      C = BC->beginAtom();

      if (!A->bond(B) || !D->bond(C)) {
        return;
      }
    }

    Eigen::Vector3d BCVec = *C->pos() - *B->pos();
    Eigen::Vector3d BAVec = *A->pos() - *B->pos();
    Eigen::Vector3d CDVec = *D->pos() - *C->pos();

    Eigen::Vector3d tmp = BAVec.cross(BCVec);
    BAVec = BCVec.cross(tmp);
    tmp = CDVec.cross(BCVec);
    CDVec = BCVec.cross(tmp);

    Eigen::Vector3d mid = *B->pos() + (BCVec.normalized() * (BCVec.norm() / 2));

    BAVec = BAVec.normalized() * 1.5;
    CDVec = CDVec.normalized() * 1.5;

    Eigen::Vector3d a = mid + BAVec;
    Eigen::Vector3d d = mid + CDVec;

    //TODO: radius = qMin(BAVec.norm(), CDVec.norm()) * 0.5;
    // if (BAVec.norm() == CDVec.norm())
    //    radius = BAVec.norm() / 2;
    drawAngleSector(widget, mid, a, d, alternateAngle);
  }

  // ##########  drawDihedralAngles  ##########

  void BondCentricTool::drawDihedralAngles(GLWidget *widget, Atom *A, Bond *BC)
  {
    if (!widget || !A || !BC) {
      return;
    }

    Molecule *mol = widget->molecule();

    Atom *B = BC->beginAtom();
    Atom *C = BC->endAtom();

    if (!A->bond(B)) {
      B = BC->endAtom();
      C = BC->beginAtom();

      if (!A->bond(B)) {
        return;
      }
    }

    double minTorsion = 0.0;
    double maxTorsion = 0.0;
    double minNegTorsion = 0.0;
    double maxNegTorsion = 0.0;
    Atom *minTorsAtom = NULL;
    Atom *maxTorsAtom = NULL;
    Atom *minNegTorsAtom = NULL;
    Atom *maxNegTorsAtom = NULL;

    QList<unsigned long> neighbors = C->neighbors();
    Atom *D = 0;
    foreach (unsigned long a, neighbors) {
      D = m_molecule->atomById(a);
      if (D == B)
        continue;

      /// FIXME Implement this for torsions...
      //double torsion = mol->GetTorsion(A, B, C, D);
      double torsion = 0.0;

      if (torsion == 0.0)
        continue;

      if (torsion < 0.0) {
        if (minNegTorsion == 0.0 || torsion > minNegTorsion) {
          minNegTorsion = torsion;
          minNegTorsAtom = D;
        }

        if (torsion < maxNegTorsion) {
          maxNegTorsion = torsion;
          maxNegTorsAtom = D;
        }
      }
      else {
        if (minTorsion == 0.0 || torsion < minTorsion) {
          minTorsion = torsion;
          minTorsAtom = D;
        }

        if (torsion > maxTorsion) {
          maxTorsion = torsion;
          maxTorsAtom = D;
        }
      }
    }

    double rgb[3] = {1.0, 1.0, 0.2};
    drawDihedralRectangle(widget, BC, A, rgb);

    // One positive, one negative angle.
    if (minNegTorsion && minTorsion) {
      drawDihedralRectangle(widget, BC, minTorsAtom, rgb);
      drawDihedralRectangle(widget, BC, minNegTorsAtom, rgb);
      drawDihedralAngle(widget, A, minTorsAtom, BC);
      drawDihedralAngle(widget, A, minNegTorsAtom, BC);
    }
    // Only positive angle(s).
    else if (minTorsion) {
      drawDihedralRectangle(widget, BC, minTorsAtom, rgb);
      if (minTorsAtom != maxTorsAtom) {
        drawDihedralRectangle(widget, BC, maxTorsAtom, rgb);
      }
      drawDihedralAngle(widget, A, minTorsAtom, BC);
      drawDihedralAngle(widget, A, maxTorsAtom, BC, true);
    }
    // Only negative angle(s).
    else if (minNegTorsion) {
      drawDihedralRectangle(widget, BC, minNegTorsAtom, rgb);
      if (minNegTorsAtom != maxNegTorsAtom) {
        drawDihedralRectangle(widget, BC, maxNegTorsAtom, rgb);
      }
      drawDihedralAngle(widget, A, minNegTorsAtom, BC);
      drawDihedralAngle(widget, A, maxNegTorsAtom, BC, true);
    }
  }

  // ##########  drawSingleDihedralAngles  ##########

  void BondCentricTool::drawSingleDihedralAngles(GLWidget *widget, Atom *A, Bond *BC)
  {
    if (!widget || !A || !BC) {
      return;
    }

    Molecule *mol = widget->molecule();

    Atom *B = BC->beginAtom();
    Atom *C = BC->endAtom();

    if (!A->bond(B)) {
      B = BC->endAtom();
      C = BC->beginAtom();

      if (!A->bond(B)) {
        return;
      }
    }

    double minTorsion = 0.0;
    double maxTorsion = 0.0;
    double minNegTorsion = 0.0;
    double maxNegTorsion = 0.0;
    Atom *minTorsAtom = NULL;
    Atom *maxTorsAtom = NULL;
    Atom *minNegTorsAtom = NULL;
    Atom *maxNegTorsAtom = NULL;

    QList<unsigned long> neighbors = C->neighbors();
    Atom *D = 0;
    foreach (unsigned long a, neighbors) {
      D = m_molecule->atomById(a);
      if (D == B)
        continue;

      /// FIXME Implement this to get the torsion...
//      double torsion = mol->GetTorsion(A, B, C, D);
      double torsion = 0.0;

      if (torsion == 0.0)
        continue;

      if (torsion < 0.0) {
        if (minNegTorsion == 0.0 || torsion > minNegTorsion) {
          minNegTorsion = torsion;
          minNegTorsAtom = D;
        }

        if (torsion < maxNegTorsion) {
          maxNegTorsion = torsion;
          maxNegTorsAtom = D;
        }
      }
      else {
        if (minTorsion == 0.0 || torsion < minTorsion) {
          minTorsion = torsion;
          minTorsAtom = D;
        }

        if (torsion > maxTorsion) {
          maxTorsion = torsion;
          maxTorsAtom = D;
        }
      }
    }

    double rgb[3] = {1.0, 1.0, 0.2};
    drawDihedralRectangle(widget, BC, A, rgb);

    // One positive, one negative angle.
    if (minNegTorsion && minTorsion) {
      drawDihedralRectangle(widget, BC, minTorsAtom, rgb);
      drawDihedralRectangle(widget, BC, minNegTorsAtom, rgb);
      drawDihedralAngle(widget, A, minTorsAtom, BC);
      drawDihedralAngle(widget, A, minNegTorsAtom, BC);
    }
    // Only positive angle(s).
    else if (minTorsion) {
      drawDihedralRectangle(widget, BC, minTorsAtom, rgb);
      if (minTorsAtom != maxTorsAtom) {
        drawDihedralRectangle(widget, BC, maxTorsAtom, rgb);
      }
      drawDihedralAngle(widget, A, minTorsAtom, BC);
      drawDihedralAngle(widget, A, maxTorsAtom, BC, true);
    }
    // Only negative angle(s).
    else if (minNegTorsion) {
      drawDihedralRectangle(widget, BC, minNegTorsAtom, rgb);
      if (minNegTorsAtom != maxNegTorsAtom) {
        drawDihedralRectangle(widget, BC, maxNegTorsAtom, rgb);
      }
      drawDihedralAngle(widget, A, minNegTorsAtom, BC);
      drawDihedralAngle(widget, A, maxNegTorsAtom, BC, true);
    }

    neighbors = B->neighbors();
    Atom *u = 0;
    Atom *v = 0;
    foreach (unsigned long a, neighbors) {
      v = m_molecule->atomById(a);
      if (v == C)
        continue;

      if (u)
        drawAngleSector(widget, *B->pos(), *u->pos(), *v->pos());
      u = v;
    }
  }

  // ##########  calcualteSnapTo  ##########

  Eigen::Vector3d* BondCentricTool::calculateSnapTo(Bond *bond,
      Eigen::Vector3d *referencePoint, double maximumAngle)
  {
    if(!referencePoint || !bond ) {
      return NULL;
    }

    double angle = -1;
    Eigen::Vector3d *smallestRef = NULL;
    Atom *b = bond->beginAtom();
    Atom *e = bond->endAtom();
    Atom *t = 0;

    Eigen::Vector3d begin = *b->pos();
    Eigen::Vector3d end = *e->pos();
    Eigen::Vector3d target;

    QList<unsigned long> neighbors = b->neighbors();
    foreach (unsigned long a, neighbors) {
      t = m_molecule->atomById(a);
      if (t == e)
        continue;

      target = *t->pos();

      Eigen::Vector3d u = end - begin;
      Eigen::Vector3d v = target - begin;
      double tAngle = acos(u.dot(v) / (v.norm() * u.norm())) * 180.0 / M_PI;

      if(!(tAngle > 1 && tAngle < 179)) {
        continue;
      }

      Eigen::Vector3d orth1 = u.cross(v);
      Eigen::Vector3d orth2 = referencePoint->cross(u);

      tAngle = acos(orth1.dot(orth2) / (orth1.norm() * orth2.norm())) * 180.0 / M_PI;
      tAngle = tAngle > 90 ? 180 - tAngle : tAngle;

      if(angle < 0) {
        angle = tAngle;
        smallestRef = new Vector3d(v);
      }
      else if(tAngle < angle) {
        angle = tAngle;
        delete smallestRef;
        smallestRef = new Vector3d(v);
      }
    }

    neighbors = e->neighbors();
    foreach (unsigned long a, neighbors) {
      t = m_molecule->atomById(a);
      if (t == b)
        continue;

      target = *t->pos();

      Eigen::Vector3d u = begin - end;
      Eigen::Vector3d v = target - end;
      double tAngle = acos(u.dot(v) / (v.norm() * u.norm())) * 180.0 / M_PI;

      if(!(tAngle > 1 && tAngle < 179)) {
        continue;
      }

      Eigen::Vector3d orth1 = u.cross(v);
      Eigen::Vector3d orth2 = referencePoint->cross(u);

      tAngle = acos(orth1.dot(orth2) / (orth1.norm() * orth2.norm())) * 180.0 / M_PI;
      tAngle = tAngle > 90 ? 180 - tAngle : tAngle;

      if(angle < 0) {
        angle = tAngle;
        smallestRef = new Vector3d(v);
      }
      else if(tAngle < angle) {
        angle = tAngle;
        delete smallestRef;
        smallestRef = new Vector3d(v);
      }
    }

    if (angle > maximumAngle) {
      if (smallestRef) {
        delete smallestRef;
      }
      return NULL;
    }

    return smallestRef;
  }

  // ##########  drawManipulationRectangle  ##########

  void BondCentricTool::drawManipulationRectangle(GLWidget *widget, Bond *bond,
      Eigen::Vector3d *referencePoint, double rgb[3])
  {
    if (!bond || !widget || !referencePoint) {
      return;
    }

    Atom *leftAtom = bond->beginAtom();
    Atom *rightAtom = bond->endAtom();

    Eigen::Vector3d left = *leftAtom->pos();
    Eigen::Vector3d right = *rightAtom->pos();

    Eigen::Vector3d leftToRight = right - left;

    Eigen::Vector3d vec = leftToRight.cross(*referencePoint);
    Eigen::Vector3d planeVec = vec.cross(leftToRight);

    double length = 1;

    planeVec = length * (planeVec / planeVec.norm());

    Eigen::Vector3d topLeft = widget->camera()->modelview() * (left + planeVec);
    Eigen::Vector3d topRight = widget->camera()->modelview() * (right + planeVec);
    Eigen::Vector3d botRight = widget->camera()->modelview() * (right - planeVec);
    Eigen::Vector3d botLeft = widget->camera()->modelview() * (left - planeVec);

    float alpha = 0.4;
    double lineWidth = 1.5;

    glEnable(GL_BLEND);
    widget->painter()->setColor(rgb[0], rgb[1], rgb[2], alpha);
    glDepthMask(GL_FALSE);
    widget->painter()->drawShadedQuadrilateral(topLeft, topRight, botRight, botLeft);
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    widget->painter()->setColor(1.0, 1.0, 1.0, 1.0);
    widget->painter()->drawQuadrilateral(topLeft, topRight, botRight, botLeft, lineWidth);
  }

  // ##########  drawDihedralRectangle  ##########

  void BondCentricTool::drawDihedralRectangle(GLWidget *widget, Bond *bond,
      Atom *atom, double rgb[3])
  {
    if (!bond || !widget || !atom) {
      return;
    }

    Atom *leftAtom = bond->beginAtom();
    Atom *rightAtom = bond->endAtom();

    if (!atom->bond(rightAtom)) {
      leftAtom = rightAtom;
      rightAtom = bond->beginAtom();
    }

    Eigen::Vector3d left = *leftAtom->pos();
    Eigen::Vector3d right = *rightAtom->pos();

    Eigen::Vector3d leftToRight = right - left;

    Eigen::Vector3d A = left + (leftToRight.normalized() * (leftToRight.norm() / 2));

    Eigen::Vector3d rightToAtom = *atom->pos() - *rightAtom->pos();

    Eigen::Vector3d B = right + rightToAtom.dot(leftToRight) / leftToRight.norm() *
      leftToRight.normalized();

    Eigen::Vector3d C;
    Eigen::Vector3d D;

    // Clicked atom is in front of the middle of the bond.
    if ((B-left).norm() < (A-left).norm()) {
      C = *atom->pos() + (right - B);
      B = right;
      D = C - (B - A);
    }
    // Clicked atom is in front of the end of the bond.
    else if ((B-A).norm() < (right-A).norm()) {
      C = *atom->pos() + (right - B);
      B = right;
      D = C - (B - A);
    }
    else {
      C = *atom->pos();
      D = C - (B - A);
    }

    double minHeight = 2;
    double minWidth = 3;

    // Rectangle is too short.
    if ((C-B).norm() < minHeight) {
      C = B + ((C-B).normalized() * minHeight);
      D = A + ((D-A).normalized() * minHeight);
    }

    // Rectangle is too thin.
    if ((B-A).norm() < minWidth) {
      B = A + ((B-A).normalized() * minWidth);
      C = D + ((C-D).normalized() * minWidth);
    }

    Eigen::Vector3d topLeft = widget->camera()->modelview() * D;
    Eigen::Vector3d topRight = widget->camera()->modelview() * C;
    Eigen::Vector3d botRight = widget->camera()->modelview() * B;
    Eigen::Vector3d botLeft = widget->camera()->modelview() * A;

    float alpha = 0.4;
    double lineWidth = 1.5;

    glEnable(GL_BLEND);
    widget->painter()->setColor(rgb[0], rgb[1], rgb[2], alpha);
    glDepthMask(GL_FALSE);
    widget->painter()->drawShadedQuadrilateral(topLeft, topRight, botRight, botLeft);
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    widget->painter()->setColor(1.0, 1.0, 1.0, 1.0);
    widget->painter()->drawQuadrilateral(topLeft, topRight, botRight, botLeft, lineWidth);
  }

  // ##########  drawDihedralRectanglesOfAtom  ##########

  void BondCentricTool::drawDihedralRectanglesOfAtom(GLWidget *widget, Bond *bond,
      Atom *atom, double rgb[3])
  {
    if (!bond || !widget || !atom || !isAtomInBond(atom, bond)) {
      return;
    }

    Atom *other = NULL;
    if (atom == bond->beginAtom())
      other = bond->endAtom();
    else if (atom == bond->endAtom())
      other = bond->beginAtom();
    else
      return;

    foreach (unsigned long a, atom->neighbors()) {
      Atom *atom = m_molecule->atomById(a);
      if (atom == other)
        continue;

      drawDihedralRectangle(widget, bond, atom, rgb);
    }
  }

  // ##########  drawSphere  ##########

  void BondCentricTool::drawSphere(GLWidget *widget, const Eigen::Vector3d &position,
      double radius, float alpha )
  {
    glEnable(GL_BLEND);
    widget->painter()->setColor(1.0, 1.0, 0.3, alpha);
    widget->painter()->drawSphere(&position, radius);
    glDisable(GL_BLEND);
  }

  // ##########  performRotation  ##########

  Eigen::Vector3d BondCentricTool::performRotation(double angle,
      Eigen::Vector3d rotationVector, Eigen::Vector3d centerVector,
      Eigen::Vector3d positionVector)
  {
    Quaternion qLeft = Quaternion::createRotationLeftHalf(angle, rotationVector);
    Quaternion qRight = qLeft.multiplicitiveInverse();

    return Quaternion::performRotationMultiplication(qLeft, positionVector -
        centerVector, qRight) + centerVector;
  }

  // ##########  showAnglesChanged  ##########

  void BondCentricTool::showAnglesChanged(int state)
  {
    m_showAngles = state == Qt::Checked ? true : false;

    if (m_molecule) {
      m_molecule->update();
    }
  }

  // ##########  snapToCheckBoxChanged  ##########

  void BondCentricTool::snapToCheckBoxChanged(int state)
  {
    m_snapToEnabled = state == Qt::Checked ? true : false;
    m_snapToAngleBox->setEnabled(m_snapToEnabled);

    if(!m_selectedBond) {
      return;
    }

    Eigen::Vector3d *reference = calculateSnapTo(m_selectedBond,
        m_referencePoint, m_snapToAngle);
    if (reference && m_snapToEnabled)
    {
      m_snapped = true;
      delete m_currentReference;
      m_currentReference = reference;
      *m_currentReference = m_currentReference->normalized();
    }
    else
    {
      m_snapped = false;
      delete m_currentReference;
      m_currentReference = new Vector3d(*m_referencePoint);
    }

    if (m_molecule) {
      m_molecule->update();
    }
  }

  // ##########  snapToAngleChanged  ##########

  void BondCentricTool::snapToAngleChanged(int newAngle)
  {
    m_snapToAngle = newAngle;

    if(!m_selectedBond) {
      return;
    }

    Eigen::Vector3d *reference = calculateSnapTo(m_selectedBond, m_referencePoint, m_snapToAngle);
    if (reference && m_snapToEnabled)
    {
      m_snapped = true;
      delete m_currentReference;
      m_currentReference = reference;
      *m_currentReference = m_currentReference->normalized();
    }
    else
    {
      m_snapped = false;
      delete m_currentReference;
      m_currentReference = new Vector3d(*m_referencePoint);
    }

    if (m_molecule) {
      m_molecule->update();
    }
  }

  // ##########  settingsWidget  ##########

  QWidget *BondCentricTool::settingsWidget()
  {
    if (!m_settingsWidget)
    {
      m_settingsWidget = new QWidget;

      m_showAnglesBox = new QCheckBox(tr(" Show Angles"), m_settingsWidget);
      m_showAnglesBox->setCheckState(m_showAngles ? Qt::Checked : Qt::Unchecked);

      m_snapToCheckBox = new QCheckBox(tr(" Snap-to Bonds"), m_settingsWidget);
      m_snapToCheckBox->setCheckState(m_snapToEnabled ? Qt::Checked : Qt::Unchecked);

      m_snapToAngleLabel = new QLabel(tr("Snap-to Threshold: "));
      m_snapToAngleLabel->setScaledContents(false);
      m_snapToAngleLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
      m_snapToAngleLabel->setMaximumHeight(20);

      m_snapToAngleBox = new QSpinBox(m_settingsWidget);
      m_snapToAngleBox->setRange(0, 90);
      m_snapToAngleBox->setSingleStep(1);
      m_snapToAngleBox->setValue(m_snapToAngle);
      m_snapToAngleBox->setSuffix(QString::fromUtf8("°"));
      m_snapToAngleBox->setEnabled(m_snapToEnabled);

      m_layout = new QGridLayout();
      m_layout->addWidget(m_showAnglesBox, 0, 0);
      m_layout->addWidget(m_snapToCheckBox, 1, 0);
      m_layout->addWidget(m_snapToAngleLabel, 2, 0);
      m_layout->addWidget(m_snapToAngleBox, 2, 1);
      QVBoxLayout* tmp = new QVBoxLayout;
      tmp->addLayout(m_layout);
      tmp->addStretch(1);

      connect(m_showAnglesBox, SIGNAL(stateChanged(int)), this,
          SLOT(showAnglesChanged(int)));

      connect(m_snapToCheckBox, SIGNAL(stateChanged(int)), this,
          SLOT(snapToCheckBoxChanged(int)));

      connect(m_snapToAngleBox, SIGNAL(valueChanged(int)), this,
          SLOT(snapToAngleChanged(int)));

      m_settingsWidget->setLayout(tmp);

      connect(m_settingsWidget, SIGNAL(destroyed()),
          this, SLOT(settingsWidgetDestroyed()));
    }

    return m_settingsWidget;
  }

  // ##########  settingsWidgetDestroyed  ##########

  void BondCentricTool::settingsWidgetDestroyed() {
    m_settingsWidget = 0;
  }

  // #########################  BondCentricMoveCommand  ##########################

  // ##########  Constructor  ##########

  BondCentricMoveCommand::BondCentricMoveCommand(Molecule *molecule,
      QUndoCommand *parent)
    : QUndoCommand(parent), m_molecule(0)
  {
    // Store the molecule - this call won't actually move an atom
    setText(QObject::tr("Bond Centric Manipulation"));
    m_moleculeCopy = *molecule;
    m_molecule = molecule;
    m_atomIndex = 0;
    undone = false;
  }

  // ##########  Constructor  ##########

  BondCentricMoveCommand::BondCentricMoveCommand(Molecule *molecule,
      Atom *atom, Eigen::Vector3d pos,
      QUndoCommand *parent)
    : QUndoCommand(parent), m_molecule(0)
  {
    // Store the original molecule before any modifications are made
    setText(QObject::tr("Bond Centric Manipulation"));
    m_moleculeCopy = *molecule;
    m_molecule = molecule;
    m_atomIndex = atom->index();
    m_pos = pos;
    undone = false;
  }

  // ##########  redo  ##########

  void BondCentricMoveCommand::redo()
  {
    // Move the specified atom to the location given
    if (undone) {
      Molecule newMolecule = *m_molecule;
      *m_molecule = m_moleculeCopy;
      m_moleculeCopy = newMolecule;
    }
    else if (m_atomIndex) {
      Atom *atom = m_molecule->atom(m_atomIndex);
      atom->setPos(m_pos);
      atom->update();
    }
    QUndoCommand::redo();
  }

  // ##########  undo  ##########

  void BondCentricMoveCommand::undo()
  {
    // Restore our original molecule
    Molecule newMolecule = *m_molecule;
    *m_molecule = m_moleculeCopy;
    m_moleculeCopy = newMolecule;
    undone = true;
  }

  // ##########  mergeWith  ##########

  bool BondCentricMoveCommand::mergeWith (const QUndoCommand *)
  {
    return false;
  }

  // ##########  id  ##########

  int BondCentricMoveCommand::id() const
  {
    //changed from 26011980[manipulatetool]
    return 26011981;
  }

  void BondCentricTool::writeSettings(QSettings &settings) const
  {
    Tool::writeSettings(settings);
    settings.setValue("showAngles", m_showAnglesBox->checkState());
    settings.setValue("snapTo", m_snapToCheckBox->checkState());
    settings.setValue("snapToAngle", m_snapToAngleBox->value());
  }

  void BondCentricTool::readSettings(QSettings &settings)
  {
    Tool::readSettings(settings);
    if(m_showAnglesBox) {
      m_showAnglesBox->setCheckState((Qt::CheckState)settings.value("showAngles", 2).toInt());
    }
    if(m_snapToCheckBox) {
      m_snapToCheckBox->setCheckState((Qt::CheckState)settings.value("snapTo", 2).toInt());
    }
    if(m_snapToAngleBox) {
      m_snapToAngleBox->setValue(settings.value("snapToAngle", 10).toInt());
    }
  }



}

#include "bondcentrictool.moc"

Q_EXPORT_PLUGIN2(bondcentrictool, Avogadro::BondCentricToolFactory)
