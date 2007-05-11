/**********************************************************************
  SelectRotateTool - Selection and Rotation Tool for Avogadro

  Copyright (C) 2006-2007 by Geoffrey R. Hutchison
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
#include <avogadro/primitive.h>
#include <avogadro/color.h>
#include <avogadro/glwidget.h>
#include <avogadro/camera.h>

#include <openbabel/obiter.h>
#include <openbabel/math/matrix3x3.h>

#include <eigen/projective.h>

#include <QtPlugin>
#include <QApplication>

using namespace std;
using namespace OpenBabel;
using namespace Avogadro;
using namespace Eigen;

SelectRotateTool::SelectRotateTool(QObject *parent) : Tool(parent),
                                                      _selectionDL(0),
                                                      _settingsWidget(0)
{
  QAction *action = activateAction();
  action->setIcon(QIcon(QString::fromUtf8(":/select/select.png")));
}

SelectRotateTool::~SelectRotateTool()
{
  if(_selectionDL)  {
    glDeleteLists(_selectionDL, 1);
  }

  if(_settingsWidget) {
    _settingsWidget->deleteLater();
  }
}

int SelectRotateTool::usefulness() const
{
  return 500000;
}

QUndoCommand* SelectRotateTool::mousePress(GLWidget *widget, const QMouseEvent *event)
{

  _movedSinceButtonPressed = false;
  _lastDraggingPosition = event->pos();
  _initialDraggingPosition = event->pos();
//  if (QApplication::keyboardModifiers() & Qt::ShiftModifier) {
//      _manipulateMode = true;
//  } else {
    _manipulateMode = false;
//  }

  //! List of hits from a selection/pick
  _hits = widget->hits(event->pos().x()-SEL_BOX_HALF_SIZE,
                       event->pos().y()-SEL_BOX_HALF_SIZE,
                       SEL_BOX_SIZE, SEL_BOX_SIZE);

  if(!_hits.size())
  {
    selectionBox(_initialDraggingPosition.x(), _initialDraggingPosition.y(),
        _initialDraggingPosition.x(), _initialDraggingPosition.y());
    widget->addDL(_selectionDL);
  }

  return 0;
}

QUndoCommand* SelectRotateTool::mouseRelease(GLWidget *widget, const QMouseEvent *event)
{
  Molecule *molecule = widget->molecule();
  if(!molecule) {
    return 0;
  }

  if(!_hits.size()) {
    widget->removeDL(_selectionDL);
  }

  QList<Primitive *> hitList;
  QList<Residue *>   residueList;
  if(!_movedSinceButtonPressed && _hits.size()) {
    // user didn't move the mouse -- regular picking, not selection box

    // we'll assemble separate "hit lists" of selected atoms and residues
    // (e.g., if we're in residue selection mode, picking an atom
    // will select the whole residue

    foreach (GLHit hit, _hits) {
      if(hit.type() == Primitive::AtomType) {
        Atom *atom = (Atom *) molecule->GetAtom(hit.name());
        hitList.append(atom);
        break;
      }
      else if(hit.type() == Primitive::ResidueType) {
        Residue *res = static_cast<Residue *>(molecule->GetResidue(hit.name()));
        residueList.append(res);
        break;
      }      
      // Currently only atom or residue selections are supported
//       else if(hit.type() == Primitive::BondType) {
//         Bond *bond = (Bond *) molecule->GetBond(hit.name());
//         hitList.append(bond);
//         break;
//       }
    }

    bool isSelected;
    switch (_selectionMode) {
    case 2: // residue
      foreach(Primitive *hit, hitList) {
        Atom *atom = static_cast<Atom *>(hit);
        Residue *residue = static_cast<Residue *>(atom->GetResidue());
        if (residue && !residueList.contains(residue))
          residueList.append(residue);
      } // end for(hits)
      break;
    case 3: // molecule
      foreach(Primitive *hit, hitList) {
        Atom *atom = static_cast<Atom *>(hit);
        // if this atom is unselected, select the whole fragment
        bool select = !atom->isSelected();
        QList<Primitive *> neighborList;
        OBMolAtomDFSIter iter(molecule, atom->GetIdx());
        do {
          neighborList.append(static_cast<Atom*>(&*iter));
        } while ((iter++).next());
        widget->setSelection(neighborList, select);
      }
      break;
    case 1: // atom
    default:
      widget->toggleSelection(hitList);
      break;
    }

    // if we have any residues to handle
    // we set all atoms to match the selection of the residue
    // and update the selection list of atoms (BUT NOT RESIDUES)
    foreach(Residue *residue, residueList) {
      hitList.clear();
      bool select = !residue->isSelected();
      FOR_ATOMS_OF_RESIDUE(a, residue) {
        hitList.append(static_cast<Atom*>(&*a));
      }
      widget->setSelection(hitList, select);
      residue->toggleSelected();
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
    foreach(GLHit hit, hits) {
      if(hit.type() == Primitive::AtomType) {
        ((Atom *)molecule->GetAtom(hit.name()))->toggleSelected();
      }
      else if(hit.type() == Primitive::ResidueType) {
        ((Residue *)molecule->GetResidue(hit.name()))->toggleSelected();
      }
      //              else if(hit.type() == Primitive::BondType) {
      //        ((Bond *)molecule->GetBond(hit.name()))->toggleSelected();
      //      }
    }
  }

/*  if (_hits.size()) {
    _selectionCenter.loadZero();
    foreach(Primitive *hit, widget->selectedItems()) {
      Atom *atom = static_cast<Atom *>(hit);
      _selectionCenter += atom->pos();
    }
    _selectionCenter /= widget->selectedItems().size();
  } */

  widget->update();

  return 0;
}

QUndoCommand* SelectRotateTool::mouseMove(GLWidget *widget, const QMouseEvent *event)
{
  Molecule *molecule = widget->molecule();
  QPoint deltaDragging = event->pos() - _lastDraggingPosition;

  _lastDraggingPosition = event->pos();

  if( ( event->pos() - _initialDraggingPosition ).manhattanLength() > 2 ) 
    _movedSinceButtonPressed = true;

  if( _hits.size() )
  {
    if( event->buttons() & Qt::LeftButton )
    {
      // rotate
      Matrix3d rotation = widget->camera()->modelview().linearComponent();
      Vector3d XAxis = rotation.row(0);
      Vector3d YAxis = rotation.row(1);

      if (!_manipulateMode) {
        widget->camera()->translate( widget->center() );
        widget->camera()->rotate( deltaDragging.y() * ROTATION_SPEED, XAxis );
        widget->camera()->rotate( deltaDragging.x() * ROTATION_SPEED, YAxis );
        widget->camera()->translate( - widget->center() );
      } 
/*      else if (molecule) { 
        // rotate only selected primitives
        MatrixP3d fragmentRotation;
        fragmentRotation.loadTranslation(_selectionCenter);
        fragmentRotation.rotate3(deltaDragging.y() * ROTATION_SPEED, XAxis );
        fragmentRotation.rotate3(deltaDragging.x() * ROTATION_SPEED, YAxis );
        fragmentRotation.translate(-_selectionCenter);

        FOR_ATOMS_OF_MOL(a, molecule) {
          Atom *atom = static_cast<Atom *>(&*a);
          if (atom->isSelected()) {
            atom->setPos(fragmentRotation * atom->pos());
          }
        }
      } */
    }
    else if ( event->buttons() & Qt::RightButton )
    {
      // translate
      if (!_manipulateMode) {
      widget->camera()->pretranslate( Vector3d( deltaDragging.x() * ROTATION_SPEED,
            deltaDragging.y() * ROTATION_SPEED,
            0.0 ) );
      } 
/*      else if (molecule) { 
        // translate only selected primitives
        // now only works for atoms
        OpenBabel::vector3 translation( deltaDragging.x() * ZOOM_SPEED,
                  -deltaDragging.y() * ZOOM_SPEED,
                  0.0 );
        FOR_ATOMS_OF_MOL(a, molecule)
          if (static_cast<Atom *>(&*a)->isSelected()) {
            a->SetVector(a->GetVector() + translation);
          }
      } */
    }
    else if ( event->buttons() & Qt::MidButton )
    {
      // should be some sort of zoom / scaling
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

  widget->update();

  return 0;
}

QUndoCommand* SelectRotateTool::wheel(GLWidget *widget, const QWheelEvent *event)
{
  return 0;
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
  glPushAttrib(GL_ALL_ATTRIB_BITS);
  glPushMatrix();
  glLoadIdentity();
  glEnable(GL_BLEND);
  glDisable(GL_LIGHTING);
  glColor4f(1.0, 1.0, 1.0, 0.2);
  glBegin(GL_POLYGON);
  glVertex3f(startPos[0],startPos[1],startPos[2]);
  glVertex3f(startPos[0],endPos[1],startPos[2]);
  glVertex3f(endPos[0],endPos[1],startPos[2]);
  glVertex3f(endPos[0],startPos[1],startPos[2]);
  glEnd();
  startPos[2] += 0.0001;
  glDisable(GL_BLEND);
  glColor3f(1.0, 1.0, 1.0);
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

void SelectRotateTool::setSelectionMode(int i)
{
  _selectionMode = i;
}

int SelectRotateTool::selectionMode() const
{
  return _selectionMode;
}

void SelectRotateTool::selectionModeChanged( int index )
{
  setSelectionMode(index + 1);
}

QWidget *SelectRotateTool::settingsWidget() {
  if(!_settingsWidget) {
    _settingsWidget = new QWidget;

    _comboSelectionMode = new QComboBox(_settingsWidget);
    _comboSelectionMode->addItem("Atom");
    _comboSelectionMode->addItem("Residue");
    _comboSelectionMode->addItem("Molecule");

    _layout = new QVBoxLayout();
    _layout->addWidget(_comboSelectionMode);
    _settingsWidget->setLayout(_layout);

    connect(_comboSelectionMode, SIGNAL(currentIndexChanged(int)),
        this, SLOT(selectionModeChanged(int)));

    connect(_settingsWidget, SIGNAL(destroyed()),
        this, SLOT(settingsWidgetDestroyed()));
  }

  return _settingsWidget;
}

void SelectRotateTool::settingsWidgetDestroyed() {
  _settingsWidget = 0;
}


#include "selectrotatetool.moc"

Q_EXPORT_PLUGIN2(selectrotatetool, SelectRotateToolFactory)
