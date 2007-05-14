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

NavigateTool::NavigateTool(QObject *parent) : Tool(parent), m_clickedAtom(0), m_leftButtonPressed(false), m_midButtonPressed(false), m_rightButtonPressed(false)
{
  QAction *action = activateAction();
  action->setIcon(QIcon(QString::fromUtf8(":/navigate/navigate.png")));
  action->setToolTip(tr("Manipulation Tool (F9)\n\n"
        "Left Mouse:   Click and drag to move the view\n"
        "Middle Mouse: Click and drag to zoom in or out\n"
        "Right Mouse:  Click and drag to rotate the view"));
  action->setShortcut(Qt::Key_F9);
}

NavigateTool::~NavigateTool()
{
}

int NavigateTool::usefulness() const
{
  return 1000000;
}

void NavigateTool::computeClickedAtom(const QPoint& p)
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

void NavigateTool::zoom( const Eigen::Vector3d &goal, double delta ) const
{
  Vector3d transformedGoal = m_glwidget->camera()->modelview() * goal;
  double distanceToGoal = transformedGoal.norm();

  double t = ZOOM_SPEED * delta;
  const double minDistanceToGoal = 2.0 * CAMERA_NEAR_DISTANCE;
  double u = minDistanceToGoal / distanceToGoal - 1.0;

  if( t < u ) {
    t = u;
  }

  m_glwidget->camera()->modelview().pretranslate( transformedGoal * t );
}

void NavigateTool::translate( const Eigen::Vector3d &what, const QPoint &from, const QPoint &to ) const
{
  Vector3d fromPos = m_glwidget->camera()->unProject(from, what);
  Vector3d toPos = m_glwidget->camera()->unProject(to, what);
  m_glwidget->camera()->translate( toPos - fromPos );
}

void NavigateTool::rotate( const Eigen::Vector3d &center, double deltaX, double deltaY ) const
{
  const MatrixP3d & m = m_glwidget->camera()->modelview();
  Vector3d xAxis = m_glwidget->camera()->backtransformedXAxis();
  Vector3d yAxis = m_glwidget->camera()->backtransformedYAxis();
  m_glwidget->camera()->translate( center );
  m_glwidget->camera()->rotate( deltaX * ROTATION_SPEED, yAxis );
  m_glwidget->camera()->rotate( deltaY * ROTATION_SPEED, xAxis );
  m_glwidget->camera()->translate( -center );
}

void NavigateTool::tilt( const Eigen::Vector3d &center, double delta ) const
{
  const MatrixP3d & m = m_glwidget->camera()->modelview();
  Vector3d zAxis = m_glwidget->camera()->backtransformedZAxis();
  m_glwidget->camera()->translate( center );
  m_glwidget->camera()->rotate( delta * ROTATION_SPEED, zAxis );
  m_glwidget->camera()->translate( -center );
}

QUndoCommand* NavigateTool::mousePress(GLWidget *widget, const QMouseEvent *event)
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

QUndoCommand* NavigateTool::mouseRelease(GLWidget *widget, const QMouseEvent *event)
{
  m_glwidget = widget;
  m_leftButtonPressed = false;
  m_midButtonPressed = false;
  m_rightButtonPressed = false;
  m_clickedAtom = 0;

  widget->update();
  return 0;
}

QUndoCommand* NavigateTool::mouseMove(GLWidget *widget, const QMouseEvent *event)
{
  m_glwidget = widget;
  if(!m_glwidget->molecule()) {
    return 0;
  }

  QPoint deltaDragging = event->pos() - m_lastDraggingPosition;

  // Mouse navigation has two modes - atom centred when an atom is clicked and scence if no
  // atom has been clicked.

  if( m_clickedAtom )
  {
    if ( event->buttons() & Qt::RightButton )
    {
      // Atom centred rotation 
      rotate( m_clickedAtom->pos(), deltaDragging.x(), deltaDragging.y() );
    }
    else if ( event->buttons() & Qt::MidButton )
    {
      // Perform the rotation
      tilt( m_clickedAtom->pos(), deltaDragging.x() );

      // Perform the zoom toward clicked atom
      zoom( m_clickedAtom->pos(), deltaDragging.y() );
    }
    else if ( event->buttons() & Qt::LeftButton )
    {
      // translate the molecule following mouse movement
      translate( m_clickedAtom->pos(), m_lastDraggingPosition, event->pos() );
    }
  }
  else // Nothing clicked on
  {
    if( event->buttons() & Qt::RightButton )
    {
      // rotation around the center of the molecule
      rotate( m_glwidget->center(), deltaDragging.x(), deltaDragging.y() );
    }
    else if ( event->buttons() & Qt::MidButton )
    {
      // Perform the rotation
      tilt( m_glwidget->center(), deltaDragging.x() );

      // Perform the zoom toward molecule center
      zoom( m_glwidget->center(), deltaDragging.y() );
    }
    else if ( event->buttons() & Qt::LeftButton )
    {
      // translate the molecule following mouse movement
      translate( m_glwidget->center(), m_lastDraggingPosition, event->pos() );
    }
  }

  m_lastDraggingPosition = event->pos();
  m_glwidget->update();

  return 0;
}

QUndoCommand* NavigateTool::wheel(GLWidget *widget, const QWheelEvent *event )
{
  m_glwidget = widget;
  computeClickedAtom(event->pos());
  if( m_clickedAtom )
  {
    // Perform the zoom toward clicked atom
    zoom( m_clickedAtom->pos(), - MOUSE_WHEEL_SPEED * event->delta() );
  }
  else
  {
    // Perform the zoom toward molecule center
    zoom( m_glwidget->center(), - MOUSE_WHEEL_SPEED * event->delta() );
  }
  m_glwidget->update();

  return 0;
}

bool NavigateTool::paint(GLWidget *widget)
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
    else
    {
      drawSphere(widget, widget->center(), 0.10, 1.0);
    }
  }
  return true;
}

void NavigateTool::drawSphere(GLWidget *widget,  const Eigen::Vector3d &position, double radius, float alpha )
{
  Color( 1.0, 1.0, 0.3, alpha ).applyAsMaterials();
  glEnable( GL_BLEND );
  widget->painter()->drawSphere(position, radius);
  glDisable( GL_BLEND );
}

#include "navigatetool.moc"

Q_EXPORT_PLUGIN2(navigatetool, NavigateToolFactory)
