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

ManipulateTool::ManipulateTool(QObject *parent) : Tool(parent), m_clickedAtom(0), m_leftButtonPressed(false), m_midButtonPressed(false), m_rightButtonPressed(false)
{
  QAction *action = activateAction();
  action->setIcon(QIcon(QString::fromUtf8(":/navigate/navigate.png")));
  action->setToolTip(tr("Manipulation Tool\n\n"
        "Left Mouse:   Click and drag to move atoms and bonds\n"
        "Middle Mouse: Click and drag to move atoms further away or closer\n"
        "Right Mouse:  Click and drag to rotate atoms and bonds"));
}

ManipulateTool::~ManipulateTool()
{
}

int ManipulateTool::usefulness() const
{
  return 600000;
}

void ManipulateTool::computeClickedAtom(const QPoint& p)
{
  QList<GLHit> hits;
  m_clickedAtom = 0;

  // Perform a OpenGL selection and retrieve the list of hits.
  hits = m_glwidget->hits(p.x()-SEL_BOX_HALF_SIZE,
      p.y()-SEL_BOX_HALF_SIZE,
      SEL_BOX_SIZE, SEL_BOX_SIZE);

  // Find the first atom (if any) in hits - this will be the closest
  foreach( GLHit hit, hits )
  {
    if(hit.type() == Primitive::AtomType)
    {
      m_clickedAtom = static_cast<Atom *>( m_glwidget->molecule()->GetAtom(hit.name()) );
      return;
    }
  }
}

void ManipulateTool::zoom( const Eigen::Vector3d &goal, double delta ) const
{
  Vector3d transformedGoal = m_glwidget->camera()->modelview() * goal;
  double distanceToGoal = transformedGoal.norm();

  double t = ZOOM_SPEED * delta;
  const double minDistanceToGoal = 2.0 * CAMERA_NEAR_DISTANCE;
  double u = minDistanceToGoal / distanceToGoal - 1.0;

  if( fabs(t) > fabs(u) ) {
    t = u;
  }

  MatrixP3d atomTranslation;
  atomTranslation.loadTranslation(m_glwidget->camera()->backTransformedZAxis() * t);

  if (m_glwidget->selectedItems().size())
  {
    FOR_ATOMS_OF_MOL(a, m_glwidget->molecule())
    {
      Atom *atom = static_cast<Atom *>(&*a);
      if (atom->isSelected())
        atom->setPos(atomTranslation * atom->pos());
    }
  }
  if (m_clickedAtom && !m_clickedAtom->isSelected())
    m_clickedAtom->setPos(atomTranslation * m_clickedAtom->pos());
}

void ManipulateTool::translate( const Eigen::Vector3d &what, const QPoint &from, const QPoint &to ) const
{
  Vector3d fromPos = m_glwidget->camera()->unProject(from, what);
  Vector3d toPos = m_glwidget->camera()->unProject(to, what);

  MatrixP3d atomTranslation;
  atomTranslation.loadTranslation(toPos - fromPos);

  if (m_glwidget->selectedItems().size())
  {
    FOR_ATOMS_OF_MOL(a, m_glwidget->molecule())
    {
      Atom *atom = static_cast<Atom *>(&*a);
      if (atom->isSelected())
        atom->setPos(atomTranslation * atom->pos());
    }
  }
  if (m_clickedAtom && !m_clickedAtom->isSelected())
    m_clickedAtom->setPos(atomTranslation * m_clickedAtom->pos());
}

void ManipulateTool::rotate( const Eigen::Vector3d &center, double deltaX, double deltaY ) const
{
  Matrix3d rotation = m_glwidget->camera()->modelview().linearComponent();
  Vector3d XAxis = rotation.row(0);
  Vector3d YAxis = rotation.row(1);
  // rotate only selected primitives
  MatrixP3d fragmentRotation;
  fragmentRotation.loadTranslation(center);
  fragmentRotation.rotate3(deltaY * ROTATION_SPEED, XAxis );
  fragmentRotation.rotate3(deltaX * ROTATION_SPEED, YAxis );
  fragmentRotation.translate(-center);

  FOR_ATOMS_OF_MOL(a, m_glwidget->molecule())
  {
    Atom *atom = static_cast<Atom *>(&*a);
    if (atom->isSelected())
      atom->setPos(fragmentRotation * atom->pos());
  }
}

void ManipulateTool::tilt( const Eigen::Vector3d &center, double delta ) const
{

}

QUndoCommand* ManipulateTool::mousePress(GLWidget *widget, const QMouseEvent *event)
{
  m_glwidget = widget;
  m_lastDraggingPosition = event->pos();
  m_leftButtonPressed = ( event->buttons() & Qt::LeftButton );
  m_midButtonPressed = ( event->buttons() & Qt::MidButton );
  m_rightButtonPressed = ( event->buttons() & Qt::RightButton );
  computeClickedAtom(event->pos());

  widget->update();
  return 0;
}

QUndoCommand* ManipulateTool::mouseRelease(GLWidget *widget, const QMouseEvent *event)
{
  m_glwidget = widget;
  m_leftButtonPressed = false;
  m_midButtonPressed = false;
  m_rightButtonPressed = false;
  m_clickedAtom = 0;

  widget->update();
  return 0;
}

QUndoCommand* ManipulateTool::mouseMove(GLWidget *widget, const QMouseEvent *event)
{
  m_glwidget = widget;
  if(!m_glwidget->molecule()) {
    return 0;
  }

  // Get the currently selected atoms from the view
  QList<Primitive *> currentSelection = m_glwidget->selectedItems();

  QPoint deltaDragging = event->pos() - m_lastDraggingPosition;

  // Manipulation can be performed in two ways - centred on an individual atom

  if (m_clickedAtom)
  {
    if ( event->buttons() & Qt::LeftButton )
    {
      // translate the molecule following mouse movement
      translate( m_clickedAtom->pos(), m_lastDraggingPosition, event->pos() );
    }
    else if ( event->buttons() & Qt::MidButton )
    {
      // Perform the rotation
//      tilt( m_clickedAtom->pos(), deltaDragging.x() );

      // Perform the zoom toward clicked atom
      zoom( m_clickedAtom->pos(), deltaDragging.y() );
    }
    else if ( event->buttons() & Qt::RightButton )
    {
      // Atom centred rotation 
      rotate( m_clickedAtom->pos(), deltaDragging.x(), deltaDragging.y() );
    }
  }
  else if (currentSelection.size())
  {
    // Some atoms are selected - work out where the centre is
    m_selectionCenter.loadZero();
    foreach(Primitive *hit, currentSelection)
    {
      Atom *atom = static_cast<Atom *>(hit);
      m_selectionCenter += atom->pos();
    }
    m_selectionCenter /= currentSelection.size();

    if ( event->buttons() & Qt::LeftButton )
    {
      // translate the molecule following mouse movement
      translate( m_selectionCenter, m_lastDraggingPosition, event->pos() );
    }
    else if ( event->buttons() & Qt::MidButton )
    {
      // Perform the rotation
      tilt( m_selectionCenter, deltaDragging.x() );

      // Perform the zoom toward molecule center
      zoom( m_selectionCenter, deltaDragging.y() );
    }
    else if( event->buttons() & Qt::RightButton )
    {
      // rotation around the center of the selected atoms
      rotate( m_selectionCenter, deltaDragging.x(), deltaDragging.y() );
    }
  }

  m_lastDraggingPosition = event->pos();
  m_glwidget->update();

  return 0;
}

QUndoCommand* ManipulateTool::wheel(GLWidget *widget, const QWheelEvent *event )
{
  return 0;
}

bool ManipulateTool::paint(GLWidget *widget)
{
  if(m_leftButtonPressed || m_midButtonPressed || m_rightButtonPressed) {
    if(m_clickedAtom) {
      double renderRadius = 0.0;
      foreach(Engine *engine, widget->engines())
      {
        if(engine->isEnabled())
        {
          double engineRadius = engine->radius(m_clickedAtom);
          if(engineRadius > renderRadius) {
            renderRadius = engineRadius;
          }
        }
      }
      renderRadius += 0.10;
      drawSphere(widget, m_clickedAtom->GetVector().AsArray(), renderRadius, 0.7);
    }
  }
  return true;
}

void ManipulateTool::drawSphere(GLWidget *widget,  const Eigen::Vector3d &position, double radius, float alpha )
{
  Color( 1.0, 0.3, 0.3, alpha ).applyAsMaterials();
  glEnable( GL_BLEND );
  widget->painter()->drawSphere(position, radius);
  glDisable( GL_BLEND );
}

#include "manipulatetool.moc"

Q_EXPORT_PLUGIN2(manipulatetool, ManipulateToolFactory)
