/**********************************************************************
  SelectRotateTool - Selection and Rotation Tool for Avogadro

  Copyright (C) 2007 Donald Ephraim Curtis

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
                                                      m_selectionDL(0),
                                                      m_settingsWidget(0)
{
  QAction *action = activateAction();
  action->setIcon(QIcon(QString::fromUtf8(":/select/select.png")));
  action->setToolTip(tr("Selection Tool (F11)"));
  action->setShortcut(Qt::Key_F11);
}

SelectRotateTool::~SelectRotateTool()
{
  if(m_selectionDL)  {
    glDeleteLists(m_selectionDL, 1);
  }

  if(m_settingsWidget) {
    m_settingsWidget->deleteLater();
  }
}

int SelectRotateTool::usefulness() const
{
  return 500000;
}

QUndoCommand* SelectRotateTool::mousePress(GLWidget *widget, const QMouseEvent *event)
{

  m_movedSinceButtonPressed = false;
  m_lastDraggingPosition = event->pos();
  m_initialDraggingPosition = event->pos();
//  if (QApplication::keyboardModifiers() & Qt::ShiftModifier) {
//      m_manipulateMode = true;
//  } else {
    m_manipulateMode = false;
//  }

  //! List of hits from a selection/pick
  m_hits = widget->hits(event->pos().x()-SEL_BOX_HALF_SIZE,
                       event->pos().y()-SEL_BOX_HALF_SIZE,
                       SEL_BOX_SIZE, SEL_BOX_SIZE);

  if(!m_hits.size())
  {
    selectionBox(m_initialDraggingPosition.x(), m_initialDraggingPosition.y(),
        m_initialDraggingPosition.x(), m_initialDraggingPosition.y());
    widget->addDL(m_selectionDL);
  }

  return 0;
}

QUndoCommand* SelectRotateTool::mouseRelease(GLWidget *widget, const QMouseEvent *event)
{
  Molecule *molecule = widget->molecule();
  if(!molecule) {
    return 0;
  }

  if(!m_hits.size()) {
    widget->removeDL(m_selectionDL);
  }

  QList<Primitive *> hitList;
  QList<Residue *>   residueList;
  if(!m_movedSinceButtonPressed && m_hits.size()) {
    // user didn't move the mouse -- regular picking, not selection box

    // we'll assemble separate "hit lists" of selected atoms and residues
    // (e.g., if we're in residue selection mode, picking an atom
    // will select the whole residue

    foreach (GLHit hit, m_hits) {
      if(hit.type() == Primitive::AtomType) {
        Atom *atom = static_cast<Atom *>(molecule->GetAtom(hit.name()));
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
    switch (m_selectionMode) {
    case 2: // residue
      foreach(Primitive *hit, hitList) {
        Atom *atom = static_cast<Atom *>(hit);
        // If the atom is unselected, select the whole residue
        bool select = !widget->selectedItem(atom);
        Residue *residue = static_cast<Residue *>(atom->GetResidue());
        QList<Primitive *> neighborList;
        OBMolAtomDFSIter iter(molecule, atom->GetIdx());
        FOR_ATOMS_OF_RESIDUE(a, residue) {
          neighborList.append(static_cast<Atom *>(&*a));
        }
        widget->setSelection(neighborList, select);
//        if (residue && !residueList.contains(residue))
//          residueList.append(residue);
      } // end for(hits)
      break;
    case 3: // molecule
      foreach(Primitive *hit, hitList) {
        Atom *atom = static_cast<Atom *>(hit);
        // if this atom is unselected, select the whole fragment
        bool select = !widget->selectedItem(atom);
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
//    foreach(Residue *residue, residueList) {
//      hitList.clear();
//      bool select = !residue->isSelected();
//      FOR_ATOMS_OF_RESIDUE(a, residue) {
//        hitList.append(static_cast<Atom *>(&*a));
//      }
//      widget->setSelection(hitList, select);
//      residue->toggleSelected();
//    }

  }
  else if(m_movedSinceButtonPressed && !m_hits.size())
  {
    // Selection box picking - need to figure out which atoms were in the box
    int sx = qMin(m_initialDraggingPosition.x(), m_lastDraggingPosition.x());
    int ex = qMax(m_initialDraggingPosition.x(), m_lastDraggingPosition.x());
    int sy = qMin(m_initialDraggingPosition.y(), m_lastDraggingPosition.y());
    int ey = qMax(m_initialDraggingPosition.y(), m_lastDraggingPosition.y());

    int w = ex-sx;
    int h = ey-sy;

    // (sx, sy) = Upper left most position.
    // (ex, ey) = Bottom right most position.
    QList<GLHit> hits = widget->hits(sx, sy, ex-sx, ey-sy);
    // Iterate over the hits
    foreach(GLHit hit, hits)
    {
      if(hit.type() == Primitive::AtomType)
      {
        Atom *atom = static_cast<Atom *>(molecule->GetAtom(hit.name()));
        hitList.append(atom);
      }
      //else if(hit.type() == Primitive::ResidueType)
      //{
      //  ((Residue *)molecule->GetResidue(hit.name()))->toggleSelected();
      //}
      //              else if(hit.type() == Primitive::BondType) {
      //        ((Bond *)molecule->GetBond(hit.name()))->toggleSelected();
      //      }
    }
    // Toggle the selection
    widget->toggleSelection(hitList);
  }

/*  if (_hits.size()) {
    m_selectionCenter.loadZero();
    foreach(Primitive *hit, widget->selectedItems()) {
      Atom *atom = static_cast<Atom *>(hit);
      m_selectionCenter += atom->pos();
    }
    m_selectionCenter /= widget->selectedItems().size();
  } */

  widget->update();

  return 0;
}

QUndoCommand* SelectRotateTool::mouseMove(GLWidget *widget, const QMouseEvent *event)
{
  Molecule *molecule = widget->molecule();
  QPoint deltaDragging = event->pos() - m_lastDraggingPosition;

  m_lastDraggingPosition = event->pos();

  if( ( event->pos() - m_initialDraggingPosition ).manhattanLength() > 2 ) 
    m_movedSinceButtonPressed = true;

  if( m_hits.size() )
  {
    if( event->buttons() & Qt::LeftButton )
    {
      // rotate
      Vector3d xAxis = widget->camera()->backTransformedXAxis();
      Vector3d yAxis = widget->camera()->backTransformedYAxis();

      if (!m_manipulateMode) {
        widget->camera()->translate( widget->center() );
        widget->camera()->rotate( deltaDragging.y() * ROTATION_SPEED, xAxis );
        widget->camera()->rotate( deltaDragging.x() * ROTATION_SPEED, yAxis );
        widget->camera()->translate( - widget->center() );
      } 
/*      else if (molecule) { 
        // rotate only selected primitives
        MatrixP3d fragmentRotation;
        fragmentRotation.loadTranslation(m_selectionCenter);
        fragmentRotation.rotate3(deltaDragging.y() * ROTATION_SPEED, XAxis );
        fragmentRotation.rotate3(deltaDragging.x() * ROTATION_SPEED, YAxis );
        fragmentRotation.translate(-m_selectionCenter);

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
      if (!m_manipulateMode) {
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
    selectionBox(m_initialDraggingPosition.x(), m_initialDraggingPosition.y(),
        m_lastDraggingPosition.x(), m_lastDraggingPosition.y());
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
  if(!m_selectionDL)
  {
    m_selectionDL = glGenLists(1);
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

  glNewList(m_selectionDL, GL_COMPILE);
  glPushAttrib(GL_ALL_ATTRIB_BITS);
  glPushMatrix();
  glLoadIdentity();
  glEnable(GL_BLEND);
  glDisable(GL_LIGHTING);
  glDisable(GL_CULL_FACE);
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
  m_selectionMode = i;
}

int SelectRotateTool::selectionMode() const
{
  return m_selectionMode;
}

void SelectRotateTool::selectionModeChanged( int index )
{
  setSelectionMode(index + 1);
}

QWidget *SelectRotateTool::settingsWidget() {
  if(!m_settingsWidget) {
    m_settingsWidget = new QWidget;

    m_comboSelectionMode = new QComboBox(m_settingsWidget);
    m_comboSelectionMode->addItem("Atom");
    m_comboSelectionMode->addItem("Residue");
    m_comboSelectionMode->addItem("Molecule");

    m_layout = new QVBoxLayout();
    m_layout->addWidget(m_comboSelectionMode);
    m_settingsWidget->setLayout(m_layout);

    connect(m_comboSelectionMode, SIGNAL(currentIndexChanged(int)),
        this, SLOT(selectionModeChanged(int)));

    connect(m_settingsWidget, SIGNAL(destroyed()),
        this, SLOT(settingsWidgetDestroyed()));
  }

  return m_settingsWidget;
}

void SelectRotateTool::settingsWidgetDestroyed() {
  m_settingsWidget = 0;
}


#include "selectrotatetool.moc"

Q_EXPORT_PLUGIN2(selectrotatetool, SelectRotateToolFactory)
