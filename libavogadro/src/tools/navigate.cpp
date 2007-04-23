/**********************************************************************
  Navigate - Navigation Tool for Avogadro

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

#include "navigate.h"
#include <avogadro/primitives.h>
#include <avogadro/color.h>
#include <avogadro/glwidget.h>
#include <avogadro/camera.h>

#include <openbabel/obiter.h>
#include <openbabel/mol.h>

#include <QtGui>

using namespace std;
using namespace OpenBabel;
using namespace Avogadro;
using namespace Eigen;

Navigate::Navigate() : Tool(), ROTATION_SPEED(0.005), TRANSLATION_SPEED(0.02)
{

}

Navigate::~Navigate()
{

}

void Navigate::mousePress(GLWidget *widget, const QMouseEvent *event)
{

  _movedSinceButtonPressed = false;
  _lastDraggingPosition = event->pos();
  _initialDraggingPosition = event->pos();

  // Now we want to determine whether an atom is being clicked, and which one.
  _clickedAtom = false;

  // Perform a OpenGL selection and retrieve the list of hits.
  _hits = widget->hits(event->pos().x()-2, event->pos().y()-2, 5, 5);

  // Find the first atom (if any) in hits - this will be the closest
  for( int i = 0; i < _hits.size(); i++ )
  {
    if(_hits[0].type() == Primitive::AtomType)
    {
      _clickedAtom = true;
      OBAtom *a = widget->molecule()->GetAtom(_hits[0].name());
      _clickedAtomPos = static_cast<Atom *>(a)->pos();
      _clickedAtomRadius = etab.GetVdwRad(a->GetAtomicNum());
      break;
    }
  }
}

void Navigate::mouseRelease(GLWidget *widget, const QMouseEvent *event)
{

}

void Navigate::mouseMove(GLWidget *widget, const QMouseEvent *event)
{
  if(!widget->molecule()) return;

  // Mouse navigation has two modes - atom centred when an atom is clicked and scence if no
  // atom has been clicked.

  QPoint deltaDragging = event->pos() - _lastDraggingPosition;

  _lastDraggingPosition = event->pos();

  if( ( event->pos() - _initialDraggingPosition ).manhattanLength() > 2 ) 
    _movedSinceButtonPressed = true;

  // Get the camera rotation - used whether an atom is clicked or not
  Matrix3d cameraRotation = widget->camera().matrix().linearComponent();

  if( _clickedAtom )
  {
    if ( event->buttons() & Qt::LeftButton )
    {
      // Atom centred rotation 
      widget->camera().translate( _clickedAtomPos );
      widget->camera().rotate( deltaDragging.x() * ROTATION_SPEED, cameraRotation.row(1) );
      widget->camera().rotate( deltaDragging.y() * ROTATION_SPEED, cameraRotation.row(0) );
      widget->camera().translate( -_clickedAtomPos );
    }
    else if ( event->buttons() & Qt::MidButton )
    {
      // Perform the rotation
      widget->camera().translate( _clickedAtomPos );
      widget->camera().rotate( deltaDragging.x() * ROTATION_SPEED, cameraRotation.row(2) );
      widget->camera().translate( -_clickedAtomPos );

      // Perform the atom centred zoom
      Vector3d transformedClickedAtomCenter = widget->camera().matrix() * _clickedAtomPos;
      Vector3d goal = transformedClickedAtomCenter + Vector3d(0,0,1) * 8.0 * _clickedAtomRadius;
      double t = TRANSLATION_SPEED * deltaDragging.y();
      bool isTooClose = transformedClickedAtomCenter.norm() < 10.0 * _clickedAtomRadius;
      if( isTooClose && t < 0 ) t = 0;
      if( t > 0.5 ) t = 0.5;
      if( t < -0.5 ) t = -0.5;
      widget->camera().matrix().pretranslate( goal * t );
    }
    else if ( event->buttons() & Qt::RightButton )
    {
      widget->camera().pretranslate( Vector3d( deltaDragging.x() * TRANSLATION_SPEED,
                                               -deltaDragging.y() * TRANSLATION_SPEED,
                                               0.0 ) );
    }
  }
  else // Nothing clicked on
  {
    if( event->buttons() & Qt::LeftButton )
    {
      // rotation around the center of the molecule
      widget->camera().translate( widget->center() );
      widget->camera().rotate( deltaDragging.x() * ROTATION_SPEED, cameraRotation.row(1) );
      widget->camera().rotate( deltaDragging.y() * ROTATION_SPEED, cameraRotation.row(0) );
      widget->camera().translate( - widget->center() );
    }
    else if ( event->buttons() & Qt::MidButton )
    {
      widget->camera().pretranslate( Vector3d( 0.0,
                                               0.0,
                                               deltaDragging.y() * TRANSLATION_SPEED) );
    }
    else if ( event->buttons() & Qt::RightButton )
    {
      widget->camera().pretranslate( Vector3d( deltaDragging.x() * TRANSLATION_SPEED,
                                               -deltaDragging.y() * TRANSLATION_SPEED,
                                               0.0 ) );
    }
  }

  widget->updateGL();
}

void Navigate::wheel(GLWidget *widget, const QWheelEvent *event )
{
	widget->camera().pretranslate( event->delta() * TRANSLATION_SPEED * Vector3d(0, 0, 1) );
	widget->updateGL();
}

#include "navigate.moc"
Q_EXPORT_PLUGIN2(navigate, Navigate)
