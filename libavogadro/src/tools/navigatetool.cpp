/**********************************************************************
  NavigateTool - Navigation Tool for Avogadro

  Copyright (C) 2007 by Marcus D. Hanwell
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

namespace Avogadro {
  class NavigateToolPrivate {
    public:
      NavigateToolPrivate() : clickedAtom(0), leftButtonPressed(false), midButtonPressed(false), rightButtonPressed(false) {}

      GLWidget *          glwidget;
      bool                leftButtonPressed;  // rotation
      bool                rightButtonPressed; // translation
      bool                midButtonPressed;   // scale / zoom
      Atom *              clickedAtom;

      Sphere              sphere;

      //! Temporary var for adding selection box
      GLuint              selectionDL;

      QPoint              lastDraggingPosition;
  };
}

NavigateTool::NavigateTool(QObject *parent) : Tool(parent), d_ptr(new NavigateToolPrivate)
{
  QAction *action = activateAction();
  action->setIcon(QIcon(QString::fromUtf8(":/navigate/navigate.png")));
}

NavigateTool::~NavigateTool()
{
  delete d_ptr;
}

int NavigateTool::usefulness() const
{
  return 1000000;
}

void NavigateTool::computeClickedAtom(const QPoint& p)
{
  QList<GLHit> hits;
  d_ptr->clickedAtom = 0;

  // Perform a OpenGL selection and retrieve the list of hits.
  hits = d_ptr->glwidget->hits(p.x()-SEL_BOX_HALF_SIZE,
      p.y()-SEL_BOX_HALF_SIZE,
      SEL_BOX_SIZE, SEL_BOX_SIZE);

  // Find the first atom (if any) in hits - this will be the closest
  foreach( GLHit hit, hits )
  {
    if(hit.type() == Primitive::AtomType)
    {
      d_ptr->clickedAtom = static_cast<Atom *>( d_ptr->glwidget->molecule()->GetAtom(hit.name()) );
      return;
    }
  }
}

void NavigateTool::zoom( const Eigen::Vector3d &goal, double delta ) const
{
  Vector3d transformedGoal = d_ptr->glwidget->camera().matrix() * goal;
  double distanceToGoal = transformedGoal.norm();

  double t = ZOOM_SPEED * delta;
  const double minDistanceToGoal = 2.0 * CAMERA_NEAR_DISTANCE;
  double u = minDistanceToGoal / distanceToGoal - 1.0;

  if( t < u ) {
    t = u;
  }

  d_ptr->glwidget->camera().matrix().pretranslate( transformedGoal * t );
}

void NavigateTool::translate( const Eigen::Vector3d &what, const QPoint &from, const QPoint &to ) const
{
  Vector3d fromPos = d_ptr->glwidget->unProject(from, what);
  Vector3d toPos = d_ptr->glwidget->unProject(to, what);
  d_ptr->glwidget->camera().translate( toPos - fromPos );
}

void NavigateTool::rotate( const Eigen::Vector3d &center, double deltaX, double deltaY ) const
{
  const MatrixP3d & m = d_ptr->glwidget->camera().matrix();
  Vector3d xAxis( m(0, 0), m(0, 1), m(0, 2) );
  Vector3d yAxis( m(1, 0), m(1, 1), m(1, 2) );
  d_ptr->glwidget->camera().translate( center );
  d_ptr->glwidget->camera().rotate( deltaX * ROTATION_SPEED, yAxis );
  d_ptr->glwidget->camera().rotate( deltaY * ROTATION_SPEED, xAxis );
  d_ptr->glwidget->camera().translate( -center );
}

void NavigateTool::tilt( const Eigen::Vector3d &center, double delta ) const
{
  const MatrixP3d & m = d_ptr->glwidget->camera().matrix();
  Vector3d zAxis( m(2, 0), m(2, 1), m(2, 2) );
  d_ptr->glwidget->camera().translate( center );
  d_ptr->glwidget->camera().rotate( delta * ROTATION_SPEED, zAxis );
  d_ptr->glwidget->camera().translate( -center );
}

QUndoCommand* NavigateTool::mousePress(GLWidget *widget, const QMouseEvent *event)
{
  d_ptr->glwidget = widget;
  d_ptr->lastDraggingPosition = event->pos();
  d_ptr->leftButtonPressed = ( event->buttons() & Qt::LeftButton );
  d_ptr->midButtonPressed = ( event->buttons() & Qt::MidButton );
  d_ptr->rightButtonPressed = ( event->buttons() & Qt::RightButton );
  computeClickedAtom(event->pos());

  widget->update();
  return 0;
}

QUndoCommand* NavigateTool::mouseRelease(GLWidget *widget, const QMouseEvent *event)
{
  d_ptr->glwidget = widget;
  d_ptr->leftButtonPressed = false;
  d_ptr->midButtonPressed = false;
  d_ptr->rightButtonPressed = false;
  d_ptr->clickedAtom = 0;

  widget->update();
  return 0;
}

QUndoCommand* NavigateTool::mouseMove(GLWidget *widget, const QMouseEvent *event)
{
  d_ptr->glwidget = widget;
  if(!d_ptr->glwidget->molecule()) {
    return 0;
  }

  QPoint deltaDragging = event->pos() - d_ptr->lastDraggingPosition;

  // Mouse navigation has two modes - atom centred when an atom is clicked and scence if no
  // atom has been clicked.

  if( d_ptr->clickedAtom )
  {
    if ( event->buttons() & Qt::RightButton )
    {
      // Atom centred rotation 
      rotate( d_ptr->clickedAtom->pos(), deltaDragging.x(), deltaDragging.y() );
    }
    else if ( event->buttons() & Qt::MidButton )
    {
      // Perform the rotation
      tilt( d_ptr->clickedAtom->pos(), deltaDragging.x() );

      // Perform the zoom toward clicked atom
      zoom( d_ptr->clickedAtom->pos(), deltaDragging.y() );
    }
    else if ( event->buttons() & Qt::LeftButton )
    {
      // translate the molecule following mouse movement
      translate( d_ptr->clickedAtom->pos(), d_ptr->lastDraggingPosition, event->pos() );
    }
  }
  else // Nothing clicked on
  {
    if( event->buttons() & Qt::RightButton )
    {
      // rotation around the center of the molecule
      rotate( d_ptr->glwidget->center(), deltaDragging.x(), deltaDragging.y() );
    }
    else if ( event->buttons() & Qt::MidButton )
    {
      // Perform the rotation
      tilt( d_ptr->glwidget->center(), deltaDragging.x() );

      // Perform the zoom toward molecule center
      zoom( d_ptr->glwidget->center(), deltaDragging.y() );
    }
    else if ( event->buttons() & Qt::LeftButton )
    {
      // translate the molecule following mouse movement
      translate( d_ptr->glwidget->center(), d_ptr->lastDraggingPosition, event->pos() );
    }
  }

  d_ptr->lastDraggingPosition = event->pos();
  d_ptr->glwidget->update();

  return 0;
}

QUndoCommand* NavigateTool::wheel(GLWidget *widget, const QWheelEvent *event )
{
  d_ptr->glwidget = widget;
  computeClickedAtom(event->pos());
  if( d_ptr->clickedAtom )
  {
    // Perform the zoom toward clicked atom
    zoom( d_ptr->clickedAtom->pos(), - MOUSE_WHEEL_SPEED * event->delta() );
  }
  else
  {
    // Perform the zoom toward molecule center
    zoom( d_ptr->glwidget->center(), - MOUSE_WHEEL_SPEED * event->delta() );
  }
  d_ptr->glwidget->update();

  return 0;
}

bool NavigateTool::paint(GLWidget *widget)
{
  if(d_ptr->leftButtonPressed || d_ptr->midButtonPressed || d_ptr->rightButtonPressed) {
    if(d_ptr->clickedAtom) {
      double renderRadius = 0.0;
      foreach(Engine *engine, widget->engines())
      {
        if(engine->isEnabled())
        {
          double engineRadius = engine->radius(d_ptr->clickedAtom);
          if(engineRadius > renderRadius) {
            renderRadius = engineRadius;
          }
        }
      }
      renderRadius += 0.10;
      drawSphere(widget, d_ptr->clickedAtom->GetVector().AsArray(), renderRadius, 0.7);
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

  d_ptr->sphere.setup(6);

  Color( 1.0, 1.0, 0.3, alpha ).applyAsMaterials();
  glEnable( GL_BLEND );
  d_ptr->sphere.draw(position, radius);
  glDisable( GL_BLEND );
}

#include "navigatetool.moc"

Q_EXPORT_PLUGIN2(navigatetool, NavigateToolFactory)
