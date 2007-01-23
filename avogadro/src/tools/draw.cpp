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

Draw::Draw() : Tool(), _beginAtom(NULL), _endAtom(NULL), _bond(NULL)
{
  m_action->setText(name());
  m_action->setToolTip(description());
   m_action->setIcon(QIcon(QString::fromUtf8(":/draw/draw.png")));
}

Draw::~Draw()
{
  delete m_action;
}

void Draw::initialize()
{

}

void Draw::cleanup() 
{

};

void Draw::mousePress(Molecule *molecule, GLWidget *widget, const QMouseEvent *event)
{
//   Molecule *molecule = widget->getMolecule();
  _buttons = event->buttons();

  _movedSinceButtonPressed = false;
  _lastDraggingPosition = event->pos();
  _initialDraggingPosition = event->pos();

  //! List of hits from a selection/pick
  _hits = widget->getHits(event->pos().x()-2, event->pos().y()-2, 5, 5);

  if(_buttons & Qt::LeftButton)
  {
    if(_hits.size())
    {
      GLHit hit = _hits[0];
      if(hit.type == Primitive::AtomType)
      {
        _beginAtom = (Atom *)molecule->GetAtom(hit.name);
      }
    }
    else
    {
      _beginAtom = newAtom(molecule, event->pos().x(), event->pos().y());
      _beginAtom->update();
    }
  }

}

void Draw::mouseMove(Molecule *molecule, GLWidget *widget, const QMouseEvent *event)
{
//   Molecule *molecule = widget->getMolecule();

  if((_buttons & Qt::LeftButton) && _beginAtom)
  {
    QList<GLHit> hits;
    hits = widget->getHits(event->pos().x()-2, event->pos().y()-2, 5, 5);

    bool hitBeginAtom = false;
    Atom *existingAtom = NULL;
    if(hits.size())
    {
      // parser our hits.  we want to know
      // if we hit another existingAtom that is not
      // the _endAtom which we created
      for(int i=0; i < hits.size() & !hitBeginAtom; i++)
      {
        GLHit hit = hits[i];
        if(hit.type == Primitive::AtomType)
        {
          // hit the same atom either moved here from somewhere else
          // or were already here.
          if(hit.name == _beginAtom->GetIdx())
          {
            hitBeginAtom = true;
          }
          else if(!_endAtom)
          {
            existingAtom = (Atom *)molecule->GetAtom(hit.name);
          }
          else
          {
            if(hit.name != _endAtom->GetIdx())
            {
              existingAtom = (Atom *)molecule->GetAtom(hit.name);
            }
          }
        }
      }
    }
    if(hitBeginAtom)
    {
      if(_endAtom)
      {
        molecule->DeleteAtom(_endAtom);
        _bond = NULL;
        _endAtom = NULL;
      }
      else if(_bond)
      {
        Atom *oldAtom = (Atom *)_bond->GetEndAtom();
        oldAtom->DeleteBond(_bond);
        molecule->DeleteBond(_bond);
        _bond=NULL;
      }
    }
    else if(existingAtom)
    {
      Bond *existingBond = (Bond *)molecule->GetBond(_beginAtom, existingAtom);
      if(_endAtom && _bond)
      {
        if(!existingBond) {
          _endAtom->DeleteBond(_bond);
          _bond->SetEnd(existingAtom);
          existingAtom->AddBond(_bond);
          _bond->update();
        }

        molecule->DeleteAtom(_endAtom);
        _endAtom = NULL;

        if(existingBond) {
          _bond = NULL;
        }
      }
      else if(!_bond && !existingBond)
      {
        _bond = newBond(molecule);
        _bond->SetBegin(_beginAtom);
        _bond->SetEnd(existingAtom);
        _beginAtom->AddBond(_bond);
        existingAtom->AddBond(_bond);
        _bond->update();
      }
    }
    else // if(!existingAtom && !hitBeginAtom)
    {
      if(!_endAtom)
      {
        _endAtom = newAtom(molecule, event->pos().x(), event->pos().y());
        if(!_bond)
        {
          _bond = newBond(molecule);
          _bond->SetBegin(_beginAtom);
          _bond->SetEnd(_endAtom);
          _beginAtom->AddBond(_bond);
          _endAtom->AddBond(_bond);
        }
        else
        {
          Atom *oldAtom = (Atom *)_bond->GetEndAtom();
          oldAtom->DeleteBond(_bond);
          _bond->SetEnd(_endAtom);
          _endAtom->AddBond(_bond);
        }
        _bond->update();
        _endAtom->update();
      }
      else
      {
        moveAtom(_endAtom, event->pos().x(), event->pos().y());
        _endAtom->update();
//dc:         _endAtom->update();
      }
    }
  }

}

void Draw::mouseRelease(Molecule *molecule, GLWidget *widget, const QMouseEvent *event)
{
  if(_buttons & Qt::LeftButton)
  {
    _beginAtom=NULL;
    _bond=NULL;
    _endAtom=NULL;
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

void Draw::moveAtom(Atom *atom, int x, int y)
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

    gluUnProject(x, viewport[3] - y, _DRAW_DEFAULT_WIN_Z, modelview, projection, viewport, &relPos[0], &relPos[1], &relPos[2]);
  //dc:   qDebug("Matrix %f:(%f, %f, %f)\n", f, relPos[0], relPos[1], relPos[2]);
    glPopMatrix();

    atom->SetVector(relPos[0], relPos[1], relPos[2]);
}

Atom *Draw::newAtom(Molecule *molecule, int x, int y)
{
  molecule->BeginModify();
    Atom *atom = (Atom *)molecule->NewAtom();
    moveAtom(atom, x, y);
    atom->SetAtomicNum(1);
    molecule->EndModify();

    return atom;
}

Bond *Draw::newBond(Molecule *molecule)
{
  Bond *bond = (Bond *)molecule->NewBond();
  bond->SetBO(1);

  return bond;
}

Q_EXPORT_PLUGIN2(Draw, Draw)
