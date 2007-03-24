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

#include "draw.h"
#include <avogadro/primitives.h>
#include <avogadro/color.h>
#include <avogadro/glwidget.h>

#include <openbabel/obiter.h>

#include <QtGui>

using namespace std;
using namespace OpenBabel;
using namespace Avogadro;

Draw::Draw() : Tool(), _beginAtom(NULL), _endAtom(NULL), _bond(NULL), m_element(6), m_bondOrder(1)
{
  m_selectAction->setIcon(QIcon(QString::fromUtf8(":/draw/draw.png")));

  m_comboElements = new QComboBox(m_propertiesWidget);
  m_comboElements->addItem("Hydrogen (1)");
  m_comboElements->addItem("Helium (2)");
  m_comboElements->addItem("Lithium (3)");
  m_comboElements->addItem("Beryllium (4)");
  m_comboElements->addItem("Boron (5)");
  m_comboElements->addItem("Carbon (6)");
  m_comboElements->addItem("Nitrogen (7)");
  m_comboElements->addItem("Oxygen (8)");
  m_comboElements->addItem("Fluorine (9)");
  m_comboElements->addItem("Neon (10)");
  m_comboElements->addItem("Sodium (11)");
  m_comboElements->addItem("Magnesium (12)");
  m_comboElements->addItem("Aluminum (13)");
  m_comboElements->addItem("Silicon (14)");
  m_comboElements->addItem("Phosphorus (15)");
  m_comboElements->addItem("Sulfur (16)");
  m_comboElements->addItem("Chlorine (17)");
  m_comboElements->addItem("Argon (18)");
  m_comboElements->setCurrentIndex(5);

  m_comboBondOrder = new QComboBox(m_propertiesWidget);
  m_comboBondOrder->addItem("Single");
  m_comboBondOrder->addItem("Double");
  m_comboBondOrder->addItem("Triple");

  m_layout = new QVBoxLayout();
  m_layout->addWidget(m_comboElements);
  m_layout->addWidget(m_comboBondOrder);
  m_propertiesWidget->setLayout(m_layout);

  connect(m_comboElements, SIGNAL(currentIndexChanged(int)),
      this, SLOT(elementChanged(int)));

  connect(m_comboBondOrder, SIGNAL(currentIndexChanged(int)),
      this, SLOT(bondOrderChanged(int)));
}

Draw::~Draw()
{
  delete m_selectAction;
}

void Draw::initialize()
{

}

void Draw::cleanup() 
{

};

void Draw::elementChanged( int index )
{
  setElement(index + 1);
}

void Draw::setElement( int index )
{
  m_element = index;
}

int Draw::element() const
{
  return m_element;
}

void Draw::bondOrderChanged( int index )
{
  setBondOrder(index + 1);
}

void Draw::setBondOrder( int index )
{
  m_bondOrder = index;
}

int Draw::bondOrder() const
{
  return m_bondOrder;
}

void Draw::mousePress(Molecule *molecule, GLWidget *widget, const QMouseEvent *event)
{
//   Molecule *molecule = widget->getMolecule();
  _buttons = event->buttons();

  _movedSinceButtonPressed = false;
  _lastDraggingPosition = event->pos();
  _initialDraggingPosition = event->pos();

  //! List of hits from a selection/pick
  _hits = widget->hits(event->pos().x()-2, event->pos().y()-2, 5, 5);

  if(_buttons & Qt::LeftButton)
  {
    if(_hits.size())
    {
      if(_hits[0].type() == Primitive::AtomType)
      {
        _beginAtom = (Atom *)molecule->GetAtom(_hits[0].name());
      }
    }
    else
    {
      _beginAtom = newAtom(widget, event->pos().x(), event->pos().y());
      widget->updateGeometry();
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
    hits = widget->hits(event->pos().x()-2, event->pos().y()-2, 5, 5);

    bool hitBeginAtom = false;
    Atom *existingAtom = NULL;
    if(hits.size())
    {
      // parser our hits.  we want to know
      // if we hit another existingAtom that is not
      // the _endAtom which we created
      for(int i=0; i < hits.size() && !hitBeginAtom; i++)
      {
        if(hits[i].type() == Primitive::AtomType)
        {
          // hit the same atom either moved here from somewhere else
          // or were already here.
          if(hits[i].name() == _beginAtom->GetIdx())
          {
            hitBeginAtom = true;
          }
          else if(!_endAtom)
          {
            existingAtom = (Atom *)molecule->GetAtom(hits[i].name());
          }
          else
          {
            if(hits[i].name() != _endAtom->GetIdx())
            {
              existingAtom = (Atom *)molecule->GetAtom(hits[i].name());
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
        widget->updateGeometry();
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
        widget->updateGeometry();
        _endAtom = NULL;

        if(existingBond) {
          _bond = NULL;
        }
      }
      else if(!_bond && !existingBond)
      {
        _bond = newBond(molecule, _beginAtom, existingAtom);
        _bond->update();
      }
    }
    else // if(!existingAtom && !hitBeginAtom)
    {
      if(!_endAtom)
      {
        _endAtom = newAtom(widget, event->pos().x(), event->pos().y());
        widget->updateGeometry();
        if(!_bond)
        {
          _bond = newBond(molecule, _beginAtom, _endAtom);
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
        _endAtom->setVector3d(unProject(widget, event->pos().x(), event->pos().y()));
//         moveAtom(_endAtom, widget->molGeomInfo(), event->pos().x(), event->pos().y());
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

    // create the undo action for creating endAtom and bond
    //  pass along atom idx, element, vector, bond idx, order, start/end
  }
  else if(_buttons & Qt::RightButton)
  {
    QList<GLHit> hits;
    hits = widget->hits(event->pos().x()-2, event->pos().y()-2, 5, 5);
    if(hits.size())
    {
      // get our top hit
      if(hits[0].type() == Primitive::AtomType)
      {
        Atom *atom = (Atom *)molecule->GetAtom(hits[0].name());
        molecule->DeleteAtom(atom);
        widget->updateGeometry();
      }
    }
  }
}

void Draw::wheel(Molecule *molecule, GLWidget *widget, const QWheelEvent *event)
{
}

Eigen::Vector3d Draw::unProject(GLWidget *widget, int x, int y)
{
  GLdouble projection[16];
  glGetDoublev(GL_PROJECTION_MATRIX,projection);
  GLdouble modelview[16];
  glGetDoublev(GL_MODELVIEW_MATRIX,modelview);
  GLint viewport[4];
  glGetIntegerv(GL_VIEWPORT,viewport);

  // retrieve the 3D coords of the center of the molecule
  Eigen::Vector3d center = widget->center();

  // project center
  Eigen::Vector3d projectedCenter;
  gluProject(center.x(), center.y(), center.z(), modelview, projection, viewport, &projectedCenter.x(), &projectedCenter.y(), &projectedCenter.z());

  // now projectedCenter.z() gives us the Z-index of the center of the molecule.
  // this is all what we need to know - we don't care about the x() and y() coords of
  // projectedCenter.

  // Now unproject the pixel of coordinates (x,height-y) into a 3D point having the same Z-index
  // as the molecule's center.
  Eigen::Vector3d pos;
  gluUnProject(x, viewport[3] - y, projectedCenter.z(), modelview, projection, viewport, &pos.x(), &pos.y(), &pos.z());

  return pos;
}

// void Draw::moveAtom(Atom *atom, const MolGeomInfo & molGeomInfo, int x, int y)
// {
//   GLdouble projection[16];
//   glGetDoublev(GL_PROJECTION_MATRIX,projection);
//   GLdouble modelview[16];
//   glGetDoublev(GL_MODELVIEW_MATRIX,modelview);
//   GLint viewport[4];
//   glGetIntegerv(GL_VIEWPORT,viewport);
// 
//   Eigen::Vector3d molCenter = molGeomInfo.center();
//   Eigen::Vector3d molCenterWinCoords;
//   Eigen::Vector3d atomNewPos;
// 
//   gluProject(molCenter.x(), molCenter.y(), molCenter.z(), modelview, projection, viewport, &molCenterWinCoords.x(), &molCenterWinCoords.y(), &molCenterWinCoords.z());
// 
//   gluUnProject(x, viewport[3] - y, molCenterWinCoords.z(), modelview, projection, viewport, &atomNewPos.x(), &atomNewPos.y(), &atomNewPos.z());
//   //dc:   qDebug("Matrix %f:(%f, %f, %f)\n", f, relPos[0], relPos[1], relPos[2]);
//   atom->setVector3d(atomNewPos);
// }

Atom *Draw::newAtom(GLWidget *widget, int x, int y)
{
  // GRH (for reasons I don't understand, calling Begin/EndModify here
  // causes crashes with multiple bond orders
  // (need to investigate, probable OB bug.

  widget->molecule()->BeginModify();
  Atom *atom = static_cast<Atom*>(widget->molecule()->NewAtom());
//   moveAtom(atom, molGeomInfo, x, y);
  atom->setVector3d(unProject(widget, x, y));
  atom->SetAtomicNum(element());
  widget->molecule()->EndModify();
  
  return atom;
}

Bond *Draw::newBond(Molecule *molecule, Atom *beginAtom, Atom *endAtom)
{
  molecule->BeginModify();
  Bond *bond = (Bond *)molecule->NewBond();
  bond->SetBO(bondOrder());
  bond->SetBegin(beginAtom);
  bond->SetEnd(endAtom);
  beginAtom->AddBond(bond);
  endAtom->AddBond(bond);
  molecule->EndModify();

  return bond;
}

#include "draw.moc"
Q_EXPORT_PLUGIN2(draw, Draw)
