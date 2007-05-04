/**********************************************************************
  NavigateTool - Navigation Tool for Avogadro

  Copyright (C) 2007 by Marcus D. Hanwel
  Copyright (C) 2006,2007 by Benoit Jacob

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

NavigateTool::NavigateTool(QObject *parent) : Tool(parent), _clickedAtom(0), _leftButtonPressed(false), _rightButtonPressed(false)
{
  QAction *action = activateAction();
  action->setIcon(QIcon(QString::fromUtf8(":/navigate/navigate.png")));
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
  _clickedAtom = 0;

  // Perform a OpenGL selection and retrieve the list of hits.
  hits = _glwidget->hits(p.x()-SEL_BOX_HALF_SIZE,
      p.y()-SEL_BOX_HALF_SIZE,
      SEL_BOX_SIZE, SEL_BOX_SIZE);

  // Find the first atom (if any) in hits - this will be the closest
  foreach( GLHit hit, hits )
  {
    if(hit.type() == Primitive::AtomType)
    {
      _clickedAtom = static_cast<Atom *>( _glwidget->molecule()->GetAtom(hit.name()) );
      return;
    }
  }
}

void NavigateTool::zoom( const Eigen::Vector3d &goal, double delta ) const
{
  Vector3d transformedGoal = _glwidget->camera().matrix() * goal;
  double distanceToGoal = transformedGoal.norm();

  double t = ZOOM_SPEED * delta;
  const double minDistanceToGoal = 2.0 * CAMERA_NEAR_DISTANCE;
  double u = minDistanceToGoal / distanceToGoal - 1.0;

  if( t < u ) {
    t = u;
  }

  _glwidget->camera().matrix().pretranslate( transformedGoal * t );
}

void NavigateTool::translate( const Eigen::Vector3d &what, const QPoint &from, const QPoint &to ) const
{
  Vector3d fromPos = _glwidget->unProject(from, what);
  Vector3d toPos = _glwidget->unProject(to, what);
  _glwidget->camera().translate( toPos - fromPos );
}

void NavigateTool::rotate( const Eigen::Vector3d &center, double deltaX, double deltaY ) const
{
  const MatrixP3d & m = _glwidget->camera().matrix();
  Vector3d xAxis( m(0, 0), m(0, 1), m(0, 2) );
  Vector3d yAxis( m(1, 0), m(1, 1), m(1, 2) );
  _glwidget->camera().translate( center );
  _glwidget->camera().rotate( deltaX * ROTATION_SPEED, yAxis );
  _glwidget->camera().rotate( deltaY * ROTATION_SPEED, xAxis );
  _glwidget->camera().translate( -center );
}

void NavigateTool::tilt( const Eigen::Vector3d &center, double delta ) const
{
  const MatrixP3d & m = _glwidget->camera().matrix();
  Vector3d zAxis( m(2, 0), m(2, 1), m(2, 2) );
  _glwidget->camera().translate( center );
  _glwidget->camera().rotate( delta * ROTATION_SPEED, zAxis );
  _glwidget->camera().translate( -center );
}

QUndoCommand* NavigateTool::mousePress(GLWidget *widget, const QMouseEvent *event)
{
  _glwidget = widget;
  _lastDraggingPosition = event->pos();
  _leftButtonPressed = ( event->buttons() & Qt::LeftButton );
  _rightButtonPressed = ( event->buttons() & Qt::RightButton );
  computeClickedAtom(event->pos());

  widget->update();
  return 0;
}

QUndoCommand* NavigateTool::mouseRelease(GLWidget *widget, const QMouseEvent *event)
{
  _glwidget = widget;
  _leftButtonPressed = false;
  _rightButtonPressed = false;

  widget->update();
  return 0;
}

QUndoCommand* NavigateTool::mouseMove(GLWidget *widget, const QMouseEvent *event)
{
  _glwidget = widget;
  if(!_glwidget->molecule()) {
    return 0;
  }

  QPoint deltaDragging = event->pos() - _lastDraggingPosition;

  // Mouse navigation has two modes - atom centred when an atom is clicked and scence if no
  // atom has been clicked.

  if( _clickedAtom )
  {
    if ( event->buttons() & Qt::RightButton )
    {
      // Atom centred rotation 
      rotate( _clickedAtom->pos(), deltaDragging.x(), deltaDragging.y() );
    }
    else if ( event->buttons() & Qt::MidButton )
    {
      // Perform the rotation
      tilt( _clickedAtom->pos(), deltaDragging.x() );

      // Perform the zoom toward clicked atom
      zoom( _clickedAtom->pos(), deltaDragging.y() );
    }
    else if ( event->buttons() & Qt::LeftButton )
    {
      // translate the molecule following mouse movement
      translate( _clickedAtom->pos(), _lastDraggingPosition, event->pos() );
    }
  }
  else // Nothing clicked on
  {
    if( event->buttons() & Qt::RightButton )
    {
      // rotation around the center of the molecule
      rotate( _glwidget->center(), deltaDragging.x(), deltaDragging.y() );
    }
    else if ( event->buttons() & Qt::MidButton )
    {
      // Perform the rotation
      tilt( _glwidget->center(), deltaDragging.x() );

      // Perform the zoom toward molecule center
      zoom( _glwidget->center(), deltaDragging.y() );
    }
    else if ( event->buttons() & Qt::LeftButton )
    {
      // translate the molecule following mouse movement
      translate( _glwidget->center(), _lastDraggingPosition, event->pos() );
    }
  }

  _lastDraggingPosition = event->pos();
  _glwidget->update();

  return 0;
}

QUndoCommand* NavigateTool::wheel(GLWidget *widget, const QWheelEvent *event )
{
  _glwidget = widget;
  computeClickedAtom(event->pos());
  if( _clickedAtom )
  {
    // Perform the zoom toward clicked atom
    zoom( _clickedAtom->pos(), - MOUSE_WHEEL_SPEED * event->delta() );
  }
  else
  {
    // Perform the zoom toward molecule center
    zoom( _glwidget->center(), - MOUSE_WHEEL_SPEED * event->delta() );
  }
  _glwidget->update();

  return 0;
}

bool NavigateTool::paint(GLWidget *widget)
{
  if(_rightButtonPressed) {
    if(_clickedAtom) {
      double renderRadius = 0.0;
      foreach(Engine *engine, widget->engines())
      {
        if(engine->isEnabled())
        {
          double engineRadius = engine->radius(_clickedAtom);
          if(engineRadius > renderRadius) {
            renderRadius = engineRadius;
          }
        }
      }
      renderRadius += 0.10;
      drawSphere(widget, _clickedAtom->GetVector().AsArray(), renderRadius, 0.7);
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

  _sphere.setup(6);

  Color( 1.0, 1.0, 0.3, alpha ).applyAsMaterials();
  glEnable( GL_BLEND );
  _sphere.draw(position, radius);
  glDisable( GL_BLEND );
}

#include "navigatetool.moc"

Q_EXPORT_PLUGIN2(navigatetool, NavigateToolFactory)
