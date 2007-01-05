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
}

void Draw::mouseRelease(GLWidget *widget, const QMouseEvent *event)
{
//dc:   glPushMatrix();
//dc:   glLoadIdentity();
//dc:   GLdouble projection[16];
//dc:   glGetDoublev(GL_PROJECTION_MATRIX,projection);
//dc:   GLdouble modelview[16];
//dc:   glGetDoublev(GL_MODELVIEW_MATRIX,modelview);
//dc:   GLint viewport[4];
//dc:   glGetIntegerv(GL_VIEWPORT,viewport);
//dc: 
//dc:   GLdouble relPos[3];
//dc: 
//dc:   gluUnProject(float(event->pos().x()), viewport[3] - float(event->pos().y()), 0.1, modelview, projection, viewport, &relPos[0], &relPos[1], &relPos[2]);
//dc: 
//dc:   glPopMatrix();
//dc: 
//dc:   Molecule *mol = widget->getMolecule();
//dc:   Atom *atom = mol->NewAtom();
//dc: 
//dc:   atom->SetVector(relPos[0], relPos[1], relPos[2]);
}

void Draw::mouseMove(GLWidget *widget, const QMouseEvent *event)
{
}

Q_EXPORT_PLUGIN2(Draw, Draw)
