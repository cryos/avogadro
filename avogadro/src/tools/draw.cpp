/**********************************************************************
  Draw - Draw Tool for Avogadro

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

#include "draw.moc"
#include <avogadro/primitives.h>
#include <avogadro/color.h>
#include <avogadro/glwidget.h>

#include <openbabel/obiter.h>

#include <QtGui>

using namespace std;
using namespace OpenBabel;
using namespace Avogadro;

#define _DRAW_DEFAULT_WIN_Z 0.96

Draw::Draw() : Tool()
{
}

void Draw::initialize()
{

}

void Draw::cleanup() 
{

};

void Draw::mousePress(GLWidget *widget, const QMouseEvent *event)
{
  _buttons = event->buttons();
  qDebug() << event->buttons();
}

void Draw::mouseRelease(GLWidget *widget, const QMouseEvent *event)
{
  Molecule *molecule = widget->getMolecule();

  qDebug() << event->buttons();
  if(_buttons & Qt::LeftButton)
  {
    glPushMatrix();
    //glLoadIdentity();
    GLdouble projection[16];
    glGetDoublev(GL_PROJECTION_MATRIX,projection);
    GLdouble modelview[16];
    glGetDoublev(GL_MODELVIEW_MATRIX,modelview);
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT,viewport);

    GLdouble relPos[3];

    gluUnProject(event->pos().x(), viewport[3] - event->pos().y(), _DRAW_DEFAULT_WIN_Z, modelview, projection, viewport, &relPos[0], &relPos[1], &relPos[2]);
  //dc:   qDebug("Matrix %f:(%f, %f, %f)\n", f, relPos[0], relPos[1], relPos[2]);

    glPopMatrix();

    Atom *atom = (Atom *)molecule->NewAtom();
    atom->SetVector(relPos[0], relPos[1], relPos[2]);
    atom->SetAtomicNum(1);
    atom->update();
  }
  else if(_buttons & Qt::RightButton)
  {
    QList<GLHit> hits;
    hits = widget->getHits(event->pos().x()-2, event->pos().y()-2, 5, 5);
    if(hits.size())
    {
      // get our top hit
      GLHit hit = hits[0];
      if(hit.type == Primitive::AtomType)
      {
        Atom *atom = (Atom *)molecule->GetAtom(hit.name);
        molecule->DeleteAtom(atom);
      }
    }

  }
}

void Draw::mouseMove(GLWidget *widget, const QMouseEvent *event)
{
}

Q_EXPORT_PLUGIN2(Draw, Draw)
