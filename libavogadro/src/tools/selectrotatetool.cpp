/**********************************************************************
  SelectRotateTool - Selection and Rotation Tool for Avogadro

  Copyright (C) 2006 by Geoffrey R. Hutchison
  Some portions Copyright (C) 2006 by Donald E. Curtis

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

#include "selectrotatetool.h"
#include <avogadro/primitives.h>
#include <avogadro/color.h>
#include <avogadro/glwidget.h>
#include <avogadro/camera.h>

#include <openbabel/obiter.h>

#include <QtGui>

using namespace std;
using namespace OpenBabel;
using namespace Avogadro;
using namespace Eigen;

SelectRotateTool::SelectRotateTool(QObject *parent) : Tool(parent), _selectionDL(0)
{
  QAction *action = activateAction();
  action->setIcon(QIcon(QString::fromUtf8(":/select/select.png")));
}

SelectRotateTool::~SelectRotateTool()
{
  if(_selectionDL)
  {
    glDeleteLists(_selectionDL, 1);
  }
}

void SelectRotateTool::mousePress(GLWidget *widget, const QMouseEvent *event)
{

  _movedSinceButtonPressed = false;
  _lastDraggingPosition = event->pos();
  _initialDraggingPosition = event->pos();

  //! List of hits from a selection/pick
  _hits = widget->hits(event->pos().x()-2, event->pos().y()-2, 5, 5);

  if(!_hits.size())
  {
    selectionBox(_initialDraggingPosition.x(), _initialDraggingPosition.y(),
        _initialDraggingPosition.x(), _initialDraggingPosition.y());
    widget->addDL(_selectionDL);
  }
}

void SelectRotateTool::mouseRelease(GLWidget *widget, const QMouseEvent *event)
{
  Molecule *molecule = widget->molecule();
  if(!molecule) {
    return;
  }

  if(!_hits.size())
  {
    widget->removeDL(_selectionDL);
  }

  if(!_movedSinceButtonPressed && _hits.size())
  {
    for(int i=0; i < _hits.size(); i++) {
      if(_hits[i].type() == Primitive::AtomType)
      {
        Atom *atom = (Atom *) molecule->GetAtom(_hits[i].name());
        atom->toggleSelected();
        atom->update();
        break;
      }
      else if(_hits[i].type() == Primitive::BondType)
      {
        Bond *bond = (Bond *) molecule->GetBond(_hits[i].name());
        bond->toggleSelected();
        bond->update();
        break;
      }
    }
  }
  else if(_movedSinceButtonPressed && !_hits.size())
  {
    int sx = qMin(_initialDraggingPosition.x(), _lastDraggingPosition.x());
    int ex = qMax(_initialDraggingPosition.x(), _lastDraggingPosition.x());
    int sy = qMin(_initialDraggingPosition.y(), _lastDraggingPosition.y());
    int ey = qMax(_initialDraggingPosition.y(), _lastDraggingPosition.y());

    int w = ex-sx;
    int h = ey-sy;

    // (sx, sy) = Upper left most position.
    // (ex, ey) = Bottom right most position.
    QList<GLHit> hits = widget->hits(sx, sy, ex-sx, ey-sy);
    for(int i=0; i < hits.size(); i++) {
      int type = hits[i].type();
      if(type == Primitive::AtomType)
      {
        ((Atom *)molecule->GetAtom(hits[i].name()))->toggleSelected();
      }
      else if(type == Primitive::BondType)
      {
        ((Bond *)molecule->GetBond(hits[i].name()))->toggleSelected();
      }
    }
  }

  widget->updateGL();
}

void SelectRotateTool::mouseMove(GLWidget *widget, const QMouseEvent *event)
{

  QPoint deltaDragging = event->pos() - _lastDraggingPosition;

  _lastDraggingPosition = event->pos();

  if( ( event->pos() - _initialDraggingPosition ).manhattanLength() > 2 ) 
    _movedSinceButtonPressed = true;

  if( _hits.size() )
  {
    if( event->buttons() & Qt::LeftButton )
    {
      Matrix3d rotation = widget->camera().matrix().linearComponent();
      Vector3d XAxis = rotation.row(0);
      Vector3d YAxis = rotation.row(1);
      widget->camera().translate( widget->center() );
      widget->camera().rotate( deltaDragging.y() * ROTATION_SPEED, XAxis );
      widget->camera().rotate( deltaDragging.x() * ROTATION_SPEED, YAxis );
      widget->camera().translate( - widget->center() );
    }
    else if ( event->buttons() & Qt::RightButton )
    {
      //dc:       deltaDragging = _initialDraggingPosition - event->pos();
      widget->camera().pretranslate( Vector3d( deltaDragging.x() * TRANSLATION_SPEED,
            -deltaDragging.y() * TRANSLATION_SPEED,
            0.0 ) );
    }
    else if ( event->buttons() & Qt::MidButton )
    {
      //dc:       deltaDragging = _initialDraggingPosition - event->pos();
      //dc:       int xySum = deltaDragging.x() + deltaDragging.y();
      //dc: 
      //dc:       if (xySum < 0)
      //dc:         widget->setScale(deltaDragging.manhattanLength() / 5.0);
      //dc:       else if (xySum > 0)
      //dc:         widget->setScale(1.0 / deltaDragging.manhattanLength());
    }
  }
  else
  {
    // draw the selection box
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT,viewport);
    selectionBox(_initialDraggingPosition.x(), _initialDraggingPosition.y(),
        _lastDraggingPosition.x(), _lastDraggingPosition.y());
  }

  widget->updateGL();
}

void SelectRotateTool::wheel(GLWidget *widget, const QWheelEvent *event)
{
}

void SelectRotateTool::selectionBox(float sx, float sy, float ex, float ey)
{
  if(!_selectionDL)
  {
    _selectionDL = glGenLists(1);
  }

  glPushMatrix();
  glLoadIdentity();
  GLdouble projection[16];
  glGetDoublev(GL_PROJECTION_MATRIX,projection);
  GLdouble modelview[16];
  glGetDoublev(GL_MODELVIEW_MATRIX,modelview);
  GLint viewport[4];
  glGetIntegerv(GL_VIEWPORT,viewport);

  GLdouble startPos[3];
  GLdouble endPos[3];

  gluUnProject(float(sx), viewport[3] - float(sy), 0.1, modelview, projection, viewport, &startPos[0], &startPos[1], &startPos[2]);
  gluUnProject(float(ex), viewport[3] - float(ey), 0.1, modelview, projection, viewport, &endPos[0], &endPos[1], &endPos[2]);

  glNewList(_selectionDL, GL_COMPILE);
  glMatrixMode(GL_MODELVIEW);
  glPushAttrib(GL_ALL_ATTRIB_BITS);
  glPushMatrix();
  glLoadIdentity();
  glDisable(GL_CULL_FACE);
  glEnable(GL_BLEND);
  glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  Color(1.0, 1.0, 1.0, 0.2).applyAsMaterials();
  glBegin(GL_POLYGON);
  glVertex3f(startPos[0],startPos[1],startPos[2]);
  glVertex3f(startPos[0],endPos[1],startPos[2]);
  glVertex3f(endPos[0],endPos[1],startPos[2]);
  glVertex3f(endPos[0],startPos[1],startPos[2]);
  glEnd();
  startPos[2] += 0.0001;
  Color(1.0, 1.0, 1.0, 1.0).applyAsMaterials();
  glBegin(GL_LINE_LOOP);
  glVertex3f(startPos[0],startPos[1],startPos[2]);
  glVertex3f(startPos[0],endPos[1],startPos[2]);
  glVertex3f(endPos[0],endPos[1],startPos[2]);
  glVertex3f(endPos[0],startPos[1],startPos[2]);
  glEnd();
  glPopMatrix();
  glPopAttrib();
  glEndList();

  glPopMatrix();

}

#include "selectrotatetool.moc"

Q_EXPORT_PLUGIN2(selectrotatetool, SelectRotateToolFactory)
