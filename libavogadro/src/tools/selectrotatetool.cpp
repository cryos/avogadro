/**********************************************************************
  SelectRotateTool - Selection and Rotation Tool for Avogadro

  Copyright (C) 2007 Donald Ephraim Curtis
  Copyright (C) 2007 by Marcus D. Hanwell

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
#include "navigate.h"
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
                                   m_selectionBox(false),
                                   m_settingsWidget(0)
{
  QAction *action = activateAction();
  action->setIcon(QIcon(QString::fromUtf8(":/select/select.png")));
  action->setToolTip(tr("Selection Tool (F11)\n"
                        "Click to pick individual atoms, residues, or fragments\n"
                        "Drag to select a range of atoms"));
  action->setShortcut(Qt::Key_F11);
}

SelectRotateTool::~SelectRotateTool()
{
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

  //! List of hits from a selection/pick
  m_hits = widget->hits(event->pos().x()-SEL_BOX_HALF_SIZE,
                       event->pos().y()-SEL_BOX_HALF_SIZE,
                       SEL_BOX_SIZE, SEL_BOX_SIZE);

  if(!m_hits.size())
  {
    m_selectionBox = true;
  }

  return 0;
}

QUndoCommand* SelectRotateTool::mouseRelease(GLWidget *widget, const QMouseEvent*)
{
  Molecule *molecule = widget->molecule();
  if(!molecule) {
    return 0;
  }

  if(!m_hits.size()) {
    m_selectionBox = false;
  }

  QList<Primitive *> hitList;
  if(!m_movedSinceButtonPressed && m_hits.size())
  {
    // user didn't move the mouse -- regular picking, not selection box

    // we'll assemble separate "hit lists" of selected atoms and residues
    // (e.g., if we're in residue selection mode, picking an atom
    // will select the whole residue

    foreach (GLHit hit, m_hits) {
      if(hit.type() == Primitive::AtomType) // Atom selection
      {
        Atom *atom = static_cast<Atom *>(molecule->GetAtom(hit.name()));
        hitList.append(atom);
        break;
      }
      else if(hit.type() == Primitive::BondType) // Bond selection
      {
        Bond *bond = static_cast<Bond *>(molecule->GetBond(hit.name()));
        hitList.append(bond);
        break;
      }
      //      else if(hit.type() == Primitive::ResidueType) {
      //        Residue *res = static_cast<Residue *>(molecule->GetResidue(hit.name()));
      //        hitList.append(res);
      //        break;
    }

    switch (m_selectionMode) {
    case 2: // residue
      foreach(Primitive *hit, hitList) {
        if (hit->type() == Primitive::AtomType) {
          Atom *atom = static_cast<Atom *>(hit);
          // If the atom is unselected, select the whole residue
          bool select = !widget->isSelected(atom);
          Residue *residue = static_cast<Residue *>(atom->GetResidue());
          QList<Primitive *> neighborList;
          FOR_ATOMS_OF_RESIDUE(a, residue) {
            neighborList.append(static_cast<Atom *>(&*a));
          }
          widget->setSelected(neighborList, select);
        }
        // FIXME -- also need to handle other primitive hit types
        // (e.g., if we hit a residue, bond, etc.)

      } // end for(hits)
      break;
    case 3: // molecule
      foreach(Primitive *hit, hitList) {
        if (hit->type() == Primitive::AtomType) {
          Atom *atom = static_cast<Atom *>(hit);
          // if this atom is unselected, select the whole fragment
          bool select = !widget->isSelected(atom);
          QList<Primitive *> neighborList;
          OBMolAtomDFSIter iter(molecule, atom->GetIdx());
          do {
            neighborList.append(static_cast<Atom*>(&*iter));
          } while ((iter++).next());
          widget->setSelected(neighborList, select);
        }
        // FIXME -- also need to handle other primitive hit types
        // (e.g., if we hit a residue, bond, etc.)
      }
      break;
    case 1: // atom
    default:
      widget->toggleSelected(hitList);
      break;
    }
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
    QList<GLHit> hits = widget->hits(sx, sy, w, h);
    // Iterate over the hits
    foreach(GLHit hit, hits)
    {
      if(hit.type() == Primitive::AtomType) // Atom selection
      {
        Atom *atom = static_cast<Atom *>(molecule->GetAtom(hit.name()));
        if(!hitList.contains(atom))
        {
          hitList.append(atom);
        }
      }
      if(hit.type() == Primitive::BondType) // Bond selection
      {
        Bond *bond = static_cast<Bond *>(molecule->GetBond(hit.name()));
        if(!hitList.contains(bond)) {
          hitList.append(bond);
        }
      }
    }
    // Toggle the selection
    widget->toggleSelected(hitList);
  }

  widget->update();

  return 0;
}

QUndoCommand* SelectRotateTool::mouseMove(GLWidget *widget, const QMouseEvent *event)
{
  QPoint deltaDragging = event->pos() - m_lastDraggingPosition;

  if( ( event->pos() - m_initialDraggingPosition ).manhattanLength() > 2 )
    m_movedSinceButtonPressed = true;

  if( m_hits.size() )
  {
    if( event->buttons() & Qt::LeftButton )
    {
      // Rotation about the centre of the molecule
      Navigate::rotate(widget, widget->center(), deltaDragging.x(), deltaDragging.y());
    }
    else if ( event->buttons() & Qt::RightButton )
    {
      // Translation about the centre of the molecule in the x and y axes
      Navigate::translate(widget, widget->center(), m_lastDraggingPosition, event->pos());
    }
    else if ( event->buttons() & Qt::MidButton )
    {
      // Rotation about the centre of the molecule in the z axis
      Navigate::tilt(widget, widget->center(), deltaDragging.x());

      // Zoom toward the centre of the molecule
      Navigate::zoom(widget, widget->center(), deltaDragging.y());
    }
  }

  m_lastDraggingPosition = event->pos();
  widget->update();

  return 0;
}

QUndoCommand* SelectRotateTool::wheel(GLWidget*, const QWheelEvent*)
{
  return 0;
}

void SelectRotateTool::selectionBox(float sx, float sy, float ex, float ey)
{
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

bool SelectRotateTool::paint(GLWidget *)
{
  if(m_selectionBox)
  {
    selectionBox(m_initialDraggingPosition.x(), m_initialDraggingPosition.y(),
                  m_lastDraggingPosition.x(), m_lastDraggingPosition.y());
  }

  return true;
}

#include "selectrotatetool.moc"

Q_EXPORT_PLUGIN2(selectrotatetool, SelectRotateToolFactory)
