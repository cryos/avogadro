/**********************************************************************
  BondCentricTool - Bond Centric Manipulation Tool for Avogadro

  Copyright (C) 2007 by Shahzad Ali
  Copyright (C) 2007 by Ross Braithwaite
  Copyright (C) 2007 by James Bunt
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

#include "bondcentrictool.h"
#include "quaternion.h"
#include "navigate.h"

#ifdef WIN32
#include <float.h>
#include <math.h>
#define isnan(x) _isnan(x)
#endif

#include <iostream>

#include <avogadro/primitive.h>
#include <avogadro/color.h>
#include <avogadro/glwidget.h>
#include <avogadro/camera.h>
#include <avogadro/toolgroup.h>

#include <openbabel/obiter.h>
#include <openbabel/mol.h>

#include <QtPlugin>
#include <QString>

using namespace std;
using namespace OpenBabel;
using namespace Avogadro;
using namespace Eigen;

// ############################ BondCentricTool ################################

// ##########  Constructor  ##########

BondCentricTool::BondCentricTool(QObject *parent) : Tool(parent),
                                                    m_settingsWidget(NULL),
                                                    m_clickedAtom(NULL),
                                                    m_clickedBond(NULL),
                                                    m_selectedBond(NULL),
                                                    m_skeleton(NULL),
                                                    m_referencePoint(NULL),
                                                    m_currentReference(NULL),
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
  m_toolGroup = NULL;
  m_leftButtonPressed = false;
  m_midButtonPressed = false;
  m_rightButtonPressed = false;
  m_movedSinceButtonPressed = false;
  m_snapped = false;
}

// ##########  moleculeChanged  ##########

void BondCentricTool::moleculeChanged(Molecule* previous, Molecule* next)
{
  if (previous) {
    disconnect(previous, 0 , this, 0);
  }

  if (next) {
    connect((Primitive*)next, SIGNAL(primitiveRemoved(Primitive*)), this,
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
  if(!checked && m_glwidget)
  {
    m_glwidget->update();
    clearData();
  }
}

// ##########  usefulness  ##########

int BondCentricTool::usefulness() const
{
  return 2000000;
}

// ##########  mousePress  ##########

QUndoCommand* BondCentricTool::mousePress(GLWidget *widget, const QMouseEvent *event)
{
  if(m_glwidget != widget)
  {
    disconnect(widget, 0 , this, 0);
    connect(widget, SIGNAL(moleculeChanged(Molecule*, Molecule*)), this,
            SLOT(moleculeChanged(Molecule*, Molecule*)));
    m_glwidget = widget;
    moleculeChanged(NULL, m_glwidget->molecule());
  }

  m_undo = 0;

  m_lastDraggingPosition = event->pos();
  m_movedSinceButtonPressed = false;

#ifdef Q_WS_MAC
  m_leftButtonPressed = (event->buttons() & Qt::LeftButton
                         && event->modifiers() == Qt::NoModifier);
  // On the Mac, either use a three-button mouse
  // or hold down the Option key (AltModifier in Qt notation)
  m_midButtonPressed = ((event->buttons() & Qt::MidButton) ||
                        (event->buttons() & Qt::LeftButton && event->modifiers()
                        & Qt::AltModifier));
  // Hold down the Command key (ControlModifier in Qt notation) for right button
  m_rightButtonPressed = ((event->buttons() & Qt::RightButton) ||
                          (event->buttons() & Qt::LeftButton && event->modifiers()
                          & Qt::ControlModifier));
#else
  m_leftButtonPressed = (event->buttons() & Qt::LeftButton);
  m_midButtonPressed = (event->buttons() & Qt::MidButton);
  m_rightButtonPressed = (event->buttons() & Qt::RightButton);
#endif

  m_clickedAtom = NULL;
  m_clickedBond = NULL;

  int oldName = m_selectedBond ? m_selectedBond->GetIdx() : -1;

  // Check if the mouse clicked on any Atoms or Bonds.
  Primitive *clickedPrim = m_glwidget->computeClickedPrimitive(event->pos());

  if (clickedPrim && clickedPrim->type() == Primitive::AtomType)
  {
    // Atom clicked on.
    m_clickedAtom = (Atom*)clickedPrim;

    if ((m_rightButtonPressed || m_leftButtonPressed) && isAtomInBond(m_clickedAtom, m_selectedBond))
    {
      //Create an undo instance for this manipulation
      m_undo = new BondCentricMoveCommand(m_glwidget->molecule());

      m_skeleton = new SkeletonTree();
      m_skeleton->populate(m_clickedAtom, m_selectedBond, m_glwidget->molecule());
    }
  }
  else if (clickedPrim && clickedPrim->type() == Primitive::BondType)
  {
    // Bond clicked on.
    m_clickedBond = (Bond*)clickedPrim;

    // If the Bond was clicked on with the left mouse button, set it as the
    // currently selected bond and reset the reference point (if the Bond has
    // changed).
    if (m_leftButtonPressed)
    {
      m_selectedBond = m_clickedBond;

      if ((int)m_selectedBond->GetIdx() != oldName)
      {
        delete m_referencePoint;
        m_referencePoint = NULL;

        delete m_currentReference;
        m_currentReference = NULL;

        m_snapped = false;

        Atom *leftAtom = static_cast<Atom*>(m_selectedBond->GetBeginAtom());
        Atom *rightAtom = static_cast<Atom*>(m_selectedBond->GetEndAtom());

        Vector3d left = leftAtom->pos();
        Vector3d right = rightAtom->pos();
        Vector3d leftToRight = right - left;

        Vector3d x = Vector3d(1, 0, 0);
        Vector3d y = Vector3d(0, 1, 0);

        Vector3d A = leftToRight.cross(x);
        Vector3d B = leftToRight.cross(y);

        m_referencePoint = A.norm() >= B.norm() ? new Vector3d(A) : new Vector3d(B);
        *m_referencePoint = m_referencePoint->normalized();

        Vector3d *reference = calculateSnapTo(widget, m_selectedBond,
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
    /*else if (m_rightButtonPressed)
    {
      m_selectedBond = m_clickedBond;

      delete m_referencePoint;
      m_referencePoint = NULL;

      delete m_currentReference;
      m_currentReference = NULL;
    }*/
  }

  m_glwidget->update();
  return 0;
}

// ##########  mouseRelease  ##########

QUndoCommand* BondCentricTool::mouseRelease(GLWidget *widget, const QMouseEvent*)
{
  if (!m_clickedAtom && !m_clickedBond && !m_movedSinceButtonPressed)
  {
    delete m_referencePoint;
    m_referencePoint = NULL;
    delete m_currentReference;
    m_currentReference = NULL;
    m_snapped = false;
    m_selectedBond = NULL;
  }
//  else if (!m_clickedAtom && m_clickedBond && !m_movedSinceButtonPressed) {
  else if (!m_movedSinceButtonPressed) {
    m_undo = 0;
  }
  if (m_skeleton)
  {
    delete m_skeleton;
    m_skeleton = NULL;
  }

  if(m_glwidget != widget)
  {
    disconnect(widget, 0 , this, 0);
    connect(widget, SIGNAL(moleculeChanged(Molecule*, Molecule*)), this,
            SLOT(moleculeChanged(Molecule*, Molecule*)));
    m_glwidget = widget;
    moleculeChanged(NULL, m_glwidget->molecule());
  }

  m_leftButtonPressed = false;
  m_midButtonPressed = false;
  m_rightButtonPressed = false;
  m_clickedAtom = NULL;
  m_clickedBond = NULL;

  m_glwidget->update();
  return m_undo;
}

// ##########  mouseMove  ##########

QUndoCommand* BondCentricTool::mouseMove(GLWidget *widget, const QMouseEvent *event)
{
  if(m_glwidget != widget)
  {
    disconnect(widget, 0 , this, 0);
    connect(widget, SIGNAL(moleculeChanged(Molecule*, Molecule*)), this,
            SLOT(moleculeChanged(Molecule*, Molecule*)));
    m_glwidget = widget;
    moleculeChanged(NULL, m_glwidget->molecule());
  }

  if (!m_glwidget->molecule()) {
    return 0;
  }

  QPoint deltaDragging = event->pos() - m_lastDraggingPosition;

  if ((event->pos() - m_lastDraggingPosition).manhattanLength() > 2) {
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
      Atom *beginAtom = static_cast<Atom*>(m_selectedBond->GetBeginAtom());
      Atom *endAtom = static_cast<Atom*>(m_selectedBond->GetEndAtom());

      Vector3d rotationVector = beginAtom->pos() - endAtom->pos();
      rotationVector = rotationVector / rotationVector.norm();

      Vector3d begin = widget->camera()->project(beginAtom->pos());
      Vector3d end = widget->camera()->project(endAtom->pos());

      Vector3d zAxis = Vector3d(0, 0, 1);
      Vector3d beginToEnd = end - begin;
      beginToEnd -= Vector3d(0, 0, beginToEnd.z());

      Vector3d direction = zAxis.cross(beginToEnd);
      direction = direction / direction.norm();

      Vector3d mouseMoved = Vector3d( - deltaDragging.x(), - deltaDragging.y(), 0);

      double magnitude = mouseMoved.dot(direction) / direction.norm();

      *m_referencePoint = performRotation(magnitude * (M_PI / 180.0),
                                          rotationVector, Vector3d(0, 0, 0),
                                          *m_referencePoint);

      Eigen::Vector3d *reference = calculateSnapTo(widget, m_selectedBond,
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

      if (m_clickedAtom == static_cast<Atom*>(m_selectedBond->GetBeginAtom()))
        otherAtom = static_cast<Atom*>(m_selectedBond->GetEndAtom());
      else
        otherAtom = static_cast<Atom*>(m_selectedBond->GetBeginAtom());

      Vector3d center = otherAtom->pos();
      Vector3d clicked = m_clickedAtom->pos();

      Vector3d centerProj = widget->camera()->project(center);
      centerProj -= Vector3d(0,0,centerProj.z());
      Vector3d clickedProj = widget->camera()->project(clicked);
      clickedProj -= Vector3d(0,0,clickedProj.z());
      Vector3d referenceProj = widget->camera()->project(*m_currentReference + center);
      referenceProj -= Vector3d(0,0,referenceProj.z());

      Vector3d referenceVector = referenceProj - centerProj;
      referenceVector = referenceVector.normalized();

      Vector3d directionVector = clickedProj - centerProj;
      directionVector = directionVector.normalized();

      Vector3d rotationVector = referenceVector.cross(directionVector);
      rotationVector = rotationVector.normalized();

      Vector3d currMouseVector = Vector3d(event->pos().x(), event->pos().y(), 0)
                                  - centerProj;
      if(currMouseVector.norm() > 5)
      {
        currMouseVector = currMouseVector.normalized();
        double mouseAngle = acos(directionVector.dot(currMouseVector) /
                            currMouseVector.norm2());

        if(mouseAngle > 0)
        {
          Vector3d tester;

          tester = performRotation(mouseAngle, rotationVector, Vector3d(0, 0, 0),
                                   directionVector);
          double testAngle1 = acos(tester.dot(currMouseVector) /
                                   currMouseVector.norm2());

          tester = performRotation(-mouseAngle, rotationVector, Vector3d(0, 0, 0),
                                   directionVector);
          double testAngle2 = acos(tester.dot(currMouseVector) /
                                   currMouseVector.norm2());

          if(testAngle1 > testAngle2 || isnan(testAngle2)) {
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
          }
        }
      }
    }
    else if (m_selectedBond && (areAtomsBonded(m_clickedAtom, static_cast<Atom*>(m_selectedBond->GetBeginAtom())) ||
             areAtomsBonded(m_clickedAtom, static_cast<Atom*>(m_selectedBond->GetEndAtom()))))
    {
      
    }
    else {
      // rotation around the center of the molecule
      Navigate::rotate(m_glwidget, m_glwidget->center(), deltaDragging.x(), deltaDragging.y());
    }
  }
#ifdef Q_WS_MAC
  // On the Mac, either use a three-button mouse
  // or hold down the Option key (AltModifier in Qt notation)
  else if ((event->buttons() & Qt::MidButton) || (event->buttons() &
           Qt::LeftButton && event->modifiers() & Qt::AltModifier))
#else
  else if (event->buttons() & Qt::MidButton)
#endif
  {
    if (m_clickedAtom)
    {
      // Perform the rotation
      Navigate::tilt(m_glwidget, m_clickedAtom->pos(), deltaDragging.x());

      // Perform the zoom toward the center of a clicked atom
      Navigate::zoom(m_glwidget, m_clickedAtom->pos(), deltaDragging.y());
    }
    else if (m_clickedBond)
    {
      Atom *begin = static_cast<Atom *>(m_clickedBond->GetBeginAtom());
      Atom *end = static_cast<Atom *>(m_clickedBond->GetEndAtom());

      Vector3d btoe = end->pos() - begin->pos();
      double newLen = btoe.norm() / 2;
      btoe = btoe / btoe.norm();

      Vector3d mid = begin->pos() + btoe * newLen;

      // Perform the rotation
      Navigate::tilt(m_glwidget, mid, deltaDragging.x());

      // Perform the zoom toward the centre of a clicked bond
      Navigate::zoom(m_glwidget, mid, deltaDragging.y());
    }
    else
    {
      // Perform the rotation
      Navigate::tilt(m_glwidget, m_glwidget->center(), deltaDragging.x());

      // Perform the zoom toward molecule center
      Navigate::zoom(m_glwidget, m_glwidget->center(), deltaDragging.y());
    }
  }
#ifdef Q_WS_MAC
  // On the Mac, either use a three-button mouse
  // or hold down the Command key (ControlModifier in Qt notation)
  else if ((event->buttons() & Qt::RightButton) || (event->buttons() &
           Qt::LeftButton && event->modifiers() & Qt::ControlModifier))
#else
  else if (event->buttons() & Qt::RightButton)
#endif
  {
    if (isAtomInBond(m_clickedAtom, m_selectedBond))
    {
      // Adjust the length of the bond following the mouse movement.

      Atom *otherAtom;

      if (m_clickedAtom == static_cast<Atom*>(m_selectedBond->GetBeginAtom()))
        otherAtom = static_cast<Atom*>(m_selectedBond->GetEndAtom());
      else
        otherAtom = static_cast<Atom*>(m_selectedBond->GetBeginAtom());

      Vector3d clicked = m_clickedAtom->pos();
      Vector3d other = otherAtom->pos();
      Vector3d direction = clicked - other;

      Vector3d mouseLast = widget->camera()->unProject(m_lastDraggingPosition);
      Vector3d mouseCurr = widget->camera()->unProject(event->pos());
      Vector3d mouseDir = mouseCurr - mouseLast;

      Vector3d component = mouseDir.dot(direction) / direction.norm2() * direction;

      if (m_skeleton) {
        m_skeleton->skeletonTranslate(component.x(), component.y(), component.z());
      }
    }
    else if (m_selectedBond && (areAtomsBonded(m_clickedAtom, static_cast<Atom*>(m_selectedBond->GetBeginAtom())) ||
             areAtomsBonded(m_clickedAtom, static_cast<Atom*>(m_selectedBond->GetEndAtom()))))
    {
      
    }
    else {
      // Translate the molecule following mouse movement.
      Navigate::translate(m_glwidget, m_glwidget->center(), m_lastDraggingPosition, event->pos());
    }
  }

  m_lastDraggingPosition = event->pos();
  m_glwidget->update();

  return 0;
}

// ##########  wheel  ##########

QUndoCommand* BondCentricTool::wheel(GLWidget *widget, const QWheelEvent *event)
{
  if(m_glwidget != widget)
  {
    disconnect(widget, 0 , this, 0);
    connect(widget, SIGNAL(moleculeChanged(Molecule*, Molecule*)), this,
            SLOT(moleculeChanged(Molecule*, Molecule*)));
    m_glwidget = widget;
    moleculeChanged(NULL, m_glwidget->molecule());
  }

  m_clickedAtom = NULL;
  m_clickedBond = NULL;

  Primitive *clickedPrim = m_glwidget->computeClickedPrimitive(event->pos());

  if (clickedPrim && clickedPrim->type() == Primitive::AtomType)
  {
    Atom *clickedAtom = (Atom*)clickedPrim;
    // Perform the zoom toward clicked atom
    Navigate::zoom(m_glwidget, clickedAtom->pos(), - MOUSE_WHEEL_SPEED * event->delta());
  }
  else if (clickedPrim && clickedPrim->type() == Primitive::BondType)
  {
    Bond *clickedBond = (Bond*)clickedPrim;

    Atom *begin = static_cast<Atom *>(clickedBond->GetBeginAtom());
    Atom *end = static_cast<Atom *>(clickedBond->GetEndAtom());

    Vector3d btoe = end->pos() - begin->pos();
    double newLen = btoe.norm() / 2;
    btoe = btoe / btoe.norm();

    Vector3d mid = begin->pos() + btoe * newLen;

    // Perform the zoom toward the centre of a clicked bond
    Navigate::zoom(m_glwidget, mid, - MOUSE_WHEEL_SPEED * event->delta());
  }
  else {
    // Perform the zoom toward molecule center
    Navigate::zoom(m_glwidget, m_glwidget->center(), - MOUSE_WHEEL_SPEED * event->delta());
  }

  m_glwidget->update();

  return 0;
}

// ##########  paint  ##########

bool BondCentricTool::paint(GLWidget *widget)
{
  if(widget->toolGroup()->activeTool() != this) {
    clearData();
  }

  bool dihedralAtomClicked = false;

  if ((m_leftButtonPressed || m_rightButtonPressed) && m_clickedAtom &&
      m_selectedBond && !isAtomInBond(m_clickedAtom, m_selectedBond))
  {
    Atom *begin = static_cast<Atom*>(m_selectedBond->GetBeginAtom());
    Atom *end = static_cast<Atom*>(m_selectedBond->GetEndAtom());

    if (areAtomsBonded(m_clickedAtom, begin) || areAtomsBonded(m_clickedAtom, end))
    {
      dihedralAtomClicked = true;
      drawDihedralAngles(widget, m_clickedAtom, m_selectedBond);
    }
  }

  if (!dihedralAtomClicked &&
       ((m_leftButtonPressed && !m_clickedBond && !isAtomInBond(m_clickedAtom, m_selectedBond))
       || (m_midButtonPressed && !m_clickedBond && !m_clickedAtom)
       || (m_rightButtonPressed && !isAtomInBond(m_clickedAtom, m_selectedBond)))) {
    drawSphere(widget, widget->center(), 0.10, 1.0);
  }

  if (!dihedralAtomClicked && m_leftButtonPressed && m_clickedAtom &&
      (!m_selectedBond || !isAtomInBond(m_clickedAtom, m_selectedBond)))
  {
    drawAtomAngles(widget, m_clickedAtom);
  }

  if (m_selectedBond && !dihedralAtomClicked)
  {
    Atom *begin = static_cast<Atom*>(m_selectedBond->GetBeginAtom());
    Atom *end = static_cast<Atom*>(m_selectedBond->GetEndAtom());

    if (m_currentReference)
    {
      // Draw bond length text.
      QString length = tr("Bond Length:  ") +
                      QString::number(m_selectedBond->GetLength(), 10, 1) +
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

  if (atom == static_cast<Atom*>(bond->GetBeginAtom())) {
    return true;
  }

  return atom == static_cast<Atom*>(bond->GetEndAtom());
}

// ##########  areAtomsBonded  ##########

bool BondCentricTool::areAtomsBonded(Atom *atom1, Atom *atom2)
{
  if (!atom1 || !atom2 || atom1 == atom2) {
    return false;
  }

  OBBondIterator bondIter = atom1->EndBonds();

  Atom *a = (Atom*)atom1->BeginNbrAtom(bondIter);

  do
  {
    if (a == atom2) {
      return true;
    }
  }
  while ((a = (Atom*)atom1->NextNbrAtom(bondIter)) != NULL);

  return false;
}

// ##########  drawAtomAngles  ##########

void BondCentricTool::drawAtomAngles(GLWidget *widget, Atom *atom)
{
  if (!atom || !widget) {
    return;
  }

  OBBondIterator bondIter = atom->EndBonds();

  Atom *u = (Atom*)atom->BeginNbrAtom(bondIter);
  Atom *v = NULL;

  if (u != NULL)
  {
    do
    {
      OBBondIterator tmpIter = bondIter;

      while ((v = (Atom*)atom->NextNbrAtom(tmpIter)) != NULL) {
        drawAngleSector(widget, atom->pos(), u->pos(), v->pos());
      }
    }
    while((u = (Atom*)atom->NextNbrAtom(bondIter)) != NULL);
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

  Atom *ref = NULL;
  if (atom == static_cast<Atom*>(bond->GetBeginAtom())) {
    ref = static_cast<Atom*>(bond->GetEndAtom());
  }
  else if (atom == static_cast<Atom*>(bond->GetEndAtom())) {
    ref = static_cast<Atom*>(bond->GetBeginAtom());
  }
  else {
    return;
  }

  OBBondIterator bondIter = atom->EndBonds();
  Atom *v = (Atom*)atom->BeginNbrAtom(bondIter);

  if (v != NULL)
  {
    do
    {
      if (v == ref) {
        continue;
      }

      if (!skeleton->containsAtom(v)) {
        drawAngleSector(widget, atom->pos(), ref->pos(), v->pos());
      }
    }
    while ((v = (Atom*)atom->NextNbrAtom(bondIter)) != NULL);
  }
}

// ##########  drawDihedralAngle  ##########

// Dihedral angle between atoms A & D for the four atoms A-B-C-D
void BondCentricTool::drawDihedralAngle(GLWidget *widget, Atom *A, Atom *D, Bond *BC)
{
  if (!A || !D || !BC || !widget) {
    return;
  }

  Atom *B = static_cast<Atom*>(BC->GetBeginAtom());
  Atom *C = static_cast<Atom*>(BC->GetEndAtom());

  if (!areAtomsBonded(A, B) || !areAtomsBonded(D, C))
  {
    B = static_cast<Atom*>(BC->GetEndAtom());
    C = static_cast<Atom*>(BC->GetBeginAtom());

    if (!areAtomsBonded(A, B) || !areAtomsBonded(D, C)) {
      return;
    }
  }

  Eigen::Vector3d BCVec = C->pos() - B->pos();
  Eigen::Vector3d BAVec = A->pos() - B->pos();
  Eigen::Vector3d CDVec = D->pos() - C->pos();

  Eigen::Vector3d tmp = BAVec.cross(BCVec);
  BAVec = BCVec.cross(tmp);
  tmp = CDVec.cross(BCVec);
  CDVec = BCVec.cross(tmp);

  Eigen::Vector3d mid = B->pos() + (BCVec.normalized() * (BCVec.norm() / 2));

  BAVec = BAVec.normalized() * 1.5;
  CDVec = CDVec.normalized() * 1.5;

  Eigen::Vector3d a = mid + BAVec;
  Eigen::Vector3d d = mid + CDVec;

  // radius = qMin(BAVec.norm(), CDVec.norm()) * 0.5;
  // if (BAVec.norm() == CDVec.norm())
  //    radius = BAVec.norm() / 2;
  drawAngleSector(widget, mid, a, d);
}

// ##########  drawDihedralAngles  ##########

void BondCentricTool::drawDihedralAngles(GLWidget *widget, Atom *A, Bond *BC)
{
  if (!widget || !A || !BC) {
    return;
  }

  Molecule *mol = widget->molecule();

  Atom *B = static_cast<Atom*>(BC->GetBeginAtom());
  Atom *C = static_cast<Atom*>(BC->GetEndAtom());

  if (!areAtomsBonded(A, B))
  {
    B = static_cast<Atom*>(BC->GetEndAtom());
    C = static_cast<Atom*>(BC->GetBeginAtom());

    if (!areAtomsBonded(A, B)) {
      return;
    }
  }

  double torsion1 = 0.0;
  double torsion2 = 0.0;
  Atom *atom1 = NULL;
  Atom *atom2 = NULL;

  OBBondIterator bondIter = C->EndBonds();
  Atom *D = (Atom*)C->BeginNbrAtom(bondIter);

  if (D != NULL)
  {
    do
    {
      if (D == B) {
        continue;
      }

      double torsion = mol->GetTorsion(A, B, C, D);

      if (torsion == 0.0) {
        continue;
      } else if (torsion < 0.0) {
        torsion *= -1.0;
      }

      cout << "Torsion: " << torsion << endl;

      if (torsion1 == 0.0)
      {
        torsion1 = torsion;
        atom1 = D;
      }
      else if (torsion < torsion1)
      {
        torsion2 = torsion1;
        atom2 = atom1;
        torsion1 = torsion;
        atom1 = D;
      }
      else if (torsion2 == 0.0 || torsion < torsion2)
      {
        torsion2 = torsion;
        atom2 = D;
      }
    }
    while ((D = (Atom*)C->NextNbrAtom(bondIter)) != NULL);
  }

  double rgb[3] = {1.0, 1.0, 0.2};
  drawDihedralRectangle(widget, BC, A, rgb);
  drawDihedralRectangle(widget, BC, atom1, rgb);
  drawDihedralRectangle(widget, BC, atom2, rgb);

  drawDihedralAngle(widget, A, atom1, BC);
  drawDihedralAngle(widget, A, atom2, BC);
}

// ##########  drawAngles  ##########

void BondCentricTool::drawAngles(GLWidget *widget, Atom *atom, Bond *bond)
{
  if (!atom || !bond || !widget) {
    return;
  }

  assert(isAtomInBond(atom, bond));

  Atom *ref = NULL;
  if (atom == static_cast<Atom*>(bond->GetBeginAtom())) {
    ref = static_cast<Atom*>(bond->GetEndAtom());
  }
  else if (atom == static_cast<Atom*>(bond->GetEndAtom())) {
    ref = static_cast<Atom*>(bond->GetBeginAtom());
  }
  else {
    return;
  }

  OBBondIterator bondIter = atom->EndBonds();
  Atom *v = (Atom*)atom->BeginNbrAtom(bondIter);

  if (v != NULL)
  {
    do
    {
      if (v == ref) {
        continue;
      }

      drawAngleSector(widget, atom->pos(), ref->pos(), v->pos());
    }
    while ((v = (Atom*)atom->NextNbrAtom(bondIter)) != NULL);
  }
}

// ##########  drawAngleSector  ##########

void BondCentricTool::drawAngleSector(GLWidget *widget, Eigen::Vector3d origin,
                                      Eigen::Vector3d direction1, Eigen::Vector3d direction2)
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
  double uvAngle = acos(u.dot(v) / v.norm2()) * 180.0 / M_PI;

  // If angle is less than 1 (will be approximated to 0), attempting to draw
  // will crash, so return.
  if (abs(uvAngle) <= 1) {
    return;
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
                                    Vector3d(0, 0, 0), u);

  QString angle = QString::number(uvAngle, 10, 1) + QString::fromUtf8("°");
  glColor4f(1.0, 1.0, 1.0, 1.0);
  widget->painter()->setColor(1.0, 1.0, 1.0, 1.0);
  widget->painter()->drawText(point + origin, angle);

  glEnable(GL_BLEND);
  widget->painter()->setColor(0, 0.5, 0, 0.4);
  glDepthMask(GL_FALSE);
  widget->painter()->drawShadedSector(origin, direction1, direction2, radius);
  glDepthMask(GL_TRUE);
  glDisable(GL_BLEND);

  widget->painter()->setColor(1.0, 1.0, 1.0, 1.0);
  widget->painter()->drawArc(origin, direction1, direction2, radius, lineWidth);
}

// ##########  calcualteSnapTo  ##########

Eigen::Vector3d* BondCentricTool::calculateSnapTo(GLWidget *widget, Bond *bond,
    Eigen::Vector3d *referencePoint, double maximumAngle)
{
  if(!referencePoint || !bond || !widget) {
    return NULL;
  }

  double angle = -1;
  Eigen::Vector3d *smallestRef = NULL;
  Atom *b = static_cast<Atom*>(bond->GetBeginAtom());
  Atom *e = static_cast<Atom*>(bond->GetEndAtom());

  OBBondIterator bondIter = b->EndBonds();
  Atom *t = (Atom*)b->BeginNbrAtom(bondIter);

  Eigen::Vector3d begin = b->pos();
  Eigen::Vector3d end = e->pos();
  Eigen::Vector3d target;

  if (t != NULL)
  {
    do
    {
      if (t == e) {
        continue;
      }

      target = t->pos();

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

      if(angle < 0)
      {
        angle = tAngle;
        smallestRef = new Vector3d(v);
      }
      else if(tAngle < angle)
      {
        angle = tAngle;
        delete smallestRef;
        smallestRef = new Vector3d(v);
      }
    }
    while ((t = (Atom*)b->NextNbrAtom(bondIter)) != NULL);
  }

  bondIter = e->EndBonds();
  t = (Atom*)e->BeginNbrAtom(bondIter);

  if (t != NULL)
  {
    do
    {
      if (t == b) {
        continue;
      }

      target = t->pos();

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

      if(angle < 0)
      {
        angle = tAngle;
        smallestRef = new Vector3d(v);
      }
      else if(tAngle < angle)
      {
        angle = tAngle;
        delete smallestRef;
        smallestRef = new Vector3d(v);
      }
    }
    while ((t = (Atom*)e->NextNbrAtom(bondIter)) != NULL);
  }

  if (angle > maximumAngle)
  {
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

  Atom *leftAtom = static_cast<Atom*>(bond->GetBeginAtom());
  Atom *rightAtom = static_cast<Atom*>(bond->GetEndAtom());

  Eigen::Vector3d left = leftAtom->pos();
  Eigen::Vector3d right = rightAtom->pos();

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

  Atom *leftAtom = static_cast<Atom*>(bond->GetBeginAtom());
  Atom *rightAtom = static_cast<Atom*>(bond->GetEndAtom());

  Eigen::Vector3d left = leftAtom->pos();
  Eigen::Vector3d right = rightAtom->pos();

  Eigen::Vector3d leftToRight = right - left;

  Eigen::Vector3d A = left + (leftToRight.normalized() * (leftToRight.norm() / 2));

  Eigen::Vector3d leftToAtom = atom->pos() - leftAtom->pos();

  Eigen::Vector3d B = left + leftToAtom.dot(leftToRight) / leftToRight.norm() *
                      leftToRight.normalized();

  Eigen::Vector3d D = atom->pos() - (B - A);

  Eigen::Vector3d topLeft = widget->camera()->modelview() * D;
  Eigen::Vector3d topRight = widget->camera()->modelview() * atom->pos();
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
  if (!bond || !widget || !atom) {
    return;
  }

  assert(isAtomInBond(atom, bond));

  Atom *other = NULL;
  if (atom == static_cast<Atom*>(bond->GetBeginAtom())) {
    other = static_cast<Atom*>(bond->GetEndAtom());
  }
  else if (atom == static_cast<Atom*>(bond->GetEndAtom())) {
    other = static_cast<Atom*>(bond->GetBeginAtom());
  }
  else {
    return;
  }

  OBBondIterator bondIter = atom->EndBonds();

  Atom *a = (Atom*)atom->BeginNbrAtom(bondIter);

  do
  {
    if (a == other) {
      continue;
    }

    drawDihedralRectangle(widget, bond, a, rgb);
  }
  while ((a = (Atom*)atom->NextNbrAtom(bondIter)) != NULL);
}

// ##########  drawSphere  ##########

void BondCentricTool::drawSphere(GLWidget *widget,  const Eigen::Vector3d &position,
                                 double radius, float alpha )
{
  glEnable(GL_BLEND);
  widget->painter()->setColor(1.0, 1.0, 0.3, alpha);
  widget->painter()->drawSphere(position, radius);
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

  if (m_glwidget) {
    m_glwidget->update();
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

  Eigen::Vector3d *reference = calculateSnapTo(m_glwidget, m_selectedBond,
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

  if (m_glwidget) {
    m_glwidget->update();
  }
}

// ##########  snapToAngleChanged  ##########

void BondCentricTool::snapToAngleChanged(int newAngle)
{
  m_snapToAngle = newAngle;

  if(!m_selectedBond) {
    return;
  }

  Eigen::Vector3d *reference = calculateSnapTo(m_glwidget, m_selectedBond,
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

  if (m_glwidget) {
    m_glwidget->update();
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

    m_snapToCheckBox = new QCheckBox(tr(" Enable Snap-to"), m_settingsWidget);
    m_snapToCheckBox->setCheckState(m_snapToEnabled ? Qt::Checked : Qt::Unchecked);

    m_snapToAngleLabel = new QLabel(tr("Snap-to Angle: "));
    m_snapToAngleLabel->setScaledContents(false);
    m_snapToAngleLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_snapToAngleLabel->setMaximumHeight(20);

    m_snapToAngleBox = new QSpinBox(m_settingsWidget);
    m_snapToAngleBox->setRange(0, 90);
    m_snapToAngleBox->setSingleStep(1);
    m_snapToAngleBox->setValue(m_snapToAngle);
    m_snapToAngleBox->setSuffix(QString::fromUtf8("°"));
    m_snapToAngleBox->setEnabled(m_snapToEnabled);

    m_spacer = new QLabel(tr(""));

    m_layout = new QGridLayout();
    m_layout->setSpacing(2);
    m_layout->addWidget(m_showAnglesBox, 1, 0);
    m_layout->setRowMinimumHeight(2, 10);
    m_layout->addWidget(m_snapToCheckBox, 3, 0);
    m_layout->setRowMinimumHeight(4, 10);
    m_layout->addWidget(m_snapToAngleLabel, 5, 0);
    m_layout->addWidget(m_snapToAngleBox, 6, 0);
    m_layout->addWidget(m_spacer, 7, 0);
    m_layout->setRowStretch(7, 1);

    connect(m_showAnglesBox, SIGNAL(stateChanged(int)), this,
            SLOT(showAnglesChanged(int)));

    connect(m_snapToCheckBox, SIGNAL(stateChanged(int)), this,
            SLOT(snapToCheckBoxChanged(int)));

    connect(m_snapToAngleBox, SIGNAL(valueChanged(int)), this,
            SLOT(snapToAngleChanged(int)));

    m_settingsWidget->setLayout(m_layout);

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
  m_atomIndex = atom->GetIdx();
  m_pos = pos;
  undone = false;
}

// ##########  redo  ##########

void BondCentricMoveCommand::redo()
{
  // Move the specified atom to the location given
  if (undone)
  {
    Molecule newMolecule = *m_molecule;
    *m_molecule = m_moleculeCopy;
    m_moleculeCopy = newMolecule;
  }
  else if (m_atomIndex)
  {
    m_molecule->BeginModify();
    Atom *atom = static_cast<Atom *>(m_molecule->GetAtom(m_atomIndex));
    atom->setPos(m_pos);
    m_molecule->EndModify();
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

#include "bondcentrictool.moc"

Q_EXPORT_PLUGIN2(bondcentrictool, BondCentricToolFactory)
