/**********************************************************************
  SelectRotateTool - Selection and Rotation Tool for Avogadro

  Copyright (C) 2007 Donald Ephraim Curtis
  Copyright (C) 2007,2008 by Marcus D. Hanwell
  Copyright (C) 2010 Konstantin Tokarev

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.openmolecules.net/>

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
#include <avogadro/atom.h>
#include <avogadro/bond.h>
#include <avogadro/molecule.h>
#include <avogadro/residue.h>
#include <avogadro/color.h>
#include <avogadro/glhit.h>
#include <avogadro/glwidget.h>
#include <avogadro/primitivelist.h>

#include <openbabel/mol.h>

#include <Eigen/Geometry>

#include <QtPlugin>
#include <QApplication>
#include <QLabel>
#include <QAction>
#include <QVBoxLayout>
#include <QComboBox>
#include <QDebug>
#include <QColorDialog>
#include <QInputDialog>
#include <QPushButton>

using namespace std;
using namespace OpenBabel;
using namespace Eigen;

namespace Avogadro {

  SelectRotateTool::SelectRotateTool(QObject *parent) : Tool(parent),
    m_selectionBox(false), m_widget(0), m_selectionMode(0), m_settingsWidget(0)
  {
    QAction *action = activateAction();
    action->setIcon(QIcon(QString::fromUtf8(":/select/select.png")));
    action->setToolTip(tr("Selection Tool (F11)\n\n"
          "Left Mouse: \tClick to pick individual atoms, residues, or fragments\n"
          "\tDrag to select a range of atoms\n"
          "Right Mouse: \tClick outside the molecule to clear selection\n"
          "Use Ctrl to toggle the selection and shift to add to the selection.\n"
          "Double-Click: \tSelect an entire fragment."));
    action->setShortcut(Qt::Key_F11);

    m_atomMenu = new QMenu;
    m_atomMenu->addAction("Change radius...", this, SLOT(changeAtomRadius()));
    m_atomMenu->addAction("Reset radius", this, SLOT(resetAtomRadius()));

    m_atomMenu->addSeparator();

    m_atomMenu->addAction("Change label...", this, SLOT(changeAtomLabel()));
    m_atomMenu->addAction("Reset label", this, SLOT(resetAtomLabel()));

    m_atomMenu->addSeparator();

    m_atomMenu->addAction("Change color...", this, SLOT(changeAtomColor()));
    m_atomMenu->addAction("Reset color", this, SLOT(resetAtomColor()));

    m_bondMenu = new QMenu;
    m_bondMenu->addAction("Change label...", this, SLOT(changeAtomLabel()));
    m_bondMenu->addAction("Reset label", this, SLOT(resetAtomLabel()));
  }

  SelectRotateTool::~SelectRotateTool()
  {
    if(m_settingsWidget)
      m_settingsWidget->deleteLater();

    m_atomMenu->deleteLater();
    m_bondMenu->deleteLater();
  }

  int SelectRotateTool::usefulness() const
  {
    return 500000;
  }

  QUndoCommand* SelectRotateTool::mousePressEvent(GLWidget *widget, QMouseEvent *event)
  {
    m_movedSinceButtonPressed = false;
    m_doubleClick = false; // set true if we get a doubleClick event
    m_lastDraggingPosition = event->pos();
    m_initialDraggingPosition = event->pos();

    m_widget = widget; // save for defining centroids

    //! List of hits from a selection/pick
    m_hits = widget->hits(event->pos().x()-SEL_BOX_HALF_SIZE,
        event->pos().y()-SEL_BOX_HALF_SIZE,
        SEL_BOX_SIZE, SEL_BOX_SIZE);

    if (event->buttons() & Qt::LeftButton && !m_hits.size()) {
      m_leftButtonPressed = true;
      event->accept();
      m_selectionBox = true;
    }
    else if (event->buttons() & Qt::LeftButton) {
      m_leftButtonPressed = true;
    } else
      m_leftButtonPressed = false;

    if (event->buttons() & Qt::RightButton) {
      m_rightButtonPressed = true;
    } else
      m_rightButtonPressed = false;

    if(!m_selectionBox) {
      widget->setCursor(Qt::CrossCursor);
    }

    return 0;
  }

  QUndoCommand* SelectRotateTool::mouseReleaseEvent(GLWidget *widget,
                                                    QMouseEvent *event)
  {
    m_widget = widget; // save for defining centroids

    // Reset the cursor
    widget->setCursor(Qt::ArrowCursor);

    // Double clicks are handled in that event, not here
    if (m_doubleClick) {
      m_doubleClick = false;
      return 0;
    }

    Molecule *molecule = widget->molecule();
    if(!molecule)
      return 0;

    if(!m_hits.size())
      m_selectionBox = false;

    QList<Primitive *> hitList;
    if (m_leftButtonPressed && !m_movedSinceButtonPressed && m_hits.size()) {
      event->accept();
      // user didn't move the mouse -- regular picking, not selection box
      // we'll assemble separate "hit lists" of selected atoms and residues
      // (e.g., if we're in residue selection mode, picking an atom
      // will select the whole residue

      foreach (const GLHit& hit, m_hits) {
        if(hit.type() == Primitive::AtomType) // Atom selection
        {
          Atom *atom = molecule->atom(hit.name());
          hitList.append(atom);
          break;
        }
        else if(hit.type() == Primitive::BondType) // Bond selection
        {
          Bond *bond = molecule->bond(hit.name());
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

              // Since the atom doesn't know to which residue it belongs,
              // we iterate over all residues and check if the atom is in
              // the current residue.
              foreach (Residue *residue, molecule->residues()) {
                QList<unsigned long> atoms = residue->atoms();
                if (atoms.contains(atom->id())) {
                  QList<Primitive *> neighborList;

                  // add the atoms
                  foreach (unsigned long id, atoms)
                    neighborList.append(molecule->atomById(id));

                  // add the bonds
                  foreach (unsigned long id, residue->bonds())
                    neighborList.append(molecule->bondById(id));

                  widget->setSelected(neighborList, select);
                }
              } // end for(residues)
            }
            else if (hit->type() == Primitive::BondType) {
              Bond *bond = static_cast<Bond *>(hit);
              // If the bond is unselected, select the whole residue
              bool select = !widget->isSelected(bond);

              // Since the bond doesn't know to which residue it belongs,
              // we iterate over all residues and check if the bond is in
              // the current residue.
              foreach (Residue *residue, molecule->residues()) {
                QList<unsigned long> bonds = residue->bonds();
                if (bonds.contains(bond->id())) {
                  QList<Primitive *> neighborList;
                  // add the atoms
                  foreach (unsigned long id, residue->atoms())
                    neighborList.append(molecule->atomById(id));
                  // add the bonds
                  foreach (unsigned long id, bonds)
                    neighborList.append(molecule->bondById(id));

                  widget->setSelected(neighborList, select);
                }
              } // end for(residues)
            }
          } // end for(hits)
          break;
        case 3: // molecule
          foreach(Primitive *hit, hitList) {
            if (hit->type() == Primitive::AtomType) {
              Atom *atom = static_cast<Atom *>(hit);
              // if this atom is unselected, select the whole fragment
              bool select = !widget->isSelected(atom);
              QList<Primitive *> neighborList;

              // We really want the "connected fragment" since a Molecule can contain
              // multiple user-visible molecule fragments
              // we can use either BFS or DFS interators -- look for the connected fragment
              OpenBabel::OBMol mol = molecule->OBMol();
              OpenBabel::OBMolAtomDFSIter iter(mol, atom->index() + 1);
              Atom *tmpNeighbor;
              do {
                tmpNeighbor = molecule->atom(iter->GetIdx() - 1);
                neighborList.append(tmpNeighbor);

                // we want to find all bonds on this site
                // (obviously all bonds will be in this fragment)

                FOR_BONDS_OF_ATOM(b, *iter)
                  neighborList.append(molecule->bond(b->GetIdx()));

              } while ((iter++).next()); // this returns false when we've gone looped through the fragment

              widget->setSelected(neighborList, select);
            }
            else if (hit->type() == Primitive::BondType) {
              Bond *bond = static_cast<Bond *>(hit);
              // if this atom is unselected, select the whole fragment
              bool select = !widget->isSelected(bond);
              QList<Primitive *> neighborList;

              // We really want the "connected fragment" since a Molecule can contain
              // multiple user-visible molecule fragments
              // we can use either BFS or DFS interators -- look for the connected fragment
              OpenBabel::OBMol mol = molecule->OBMol();
              OpenBabel::OBMolAtomDFSIter iter(mol, molecule->atomById(bond->beginAtomId())->index() + 1);
              Atom *tmpNeighbor;
              do {
                tmpNeighbor = molecule->atom(iter->GetIdx() - 1);
                neighborList.append(tmpNeighbor);

                // we want to find all bonds on this site
                // (obviously all bonds will be in this fragment)
                FOR_BONDS_OF_ATOM(b, *iter)
                  neighborList.append(molecule->bond(b->GetIdx()));
              } while ((iter++).next()); // this returns false when we've gone looped through the fragment

              widget->setSelected(neighborList, select);
            }

            // FIXME -- also need to handle other primitive hit types
            // (e.g., if we hit a residue, bond, etc.)
          }
          break;
        case 1: // atom/bond
        default:
          // If the Ctrl modifier is pressed toggle selection
          if(event->modifiers() & Qt::ControlModifier)
            widget->toggleSelected(hitList);
          else {
            // If the shift modifier key is not pressed clear previous selection
            if (!(event->modifiers() & Qt::ShiftModifier))
              widget->clearSelected();
            widget->setSelected(hitList, true);
          }
          break;
      }

    }
    else if(m_leftButtonPressed && m_movedSinceButtonPressed && !m_hits.size()) {
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
      foreach(const GLHit& hit, hits) {
        if(hit.type() == Primitive::AtomType) // Atom selection
        {
          Atom *atom = molecule->atom(hit.name());
          if(!hitList.contains(atom))
            hitList.append(atom);
        }
        if(hit.type() == Primitive::BondType) // Bond selection
        {
          Bond *bond = molecule->bond(hit.name());
          if(!hitList.contains(bond))
            hitList.append(bond);
        }
      }
      // If the modifier key is not pressed clear the previous selection
      if (!(event->modifiers() & Qt::ShiftModifier))
        widget->clearSelected();
      // Set the selection
      widget->setSelected(hitList, true);
    }
    else if(m_rightButtonPressed && !m_movedSinceButtonPressed) {
      if (m_hits.size() == 1 && m_hits.at(0).type() == Primitive::BondType) {
        // Bond selection
        Bond *bond = molecule->bond(m_hits.at(0).name());
        hitList.append(bond);
        widget->clearSelected();
        widget->toggleSelected(hitList);
        m_currentPrimitive = bond;
        m_bondMenu->exec(event->globalPos());
        widget->clearSelected();
      }
      else if (m_hits.size()) {
        foreach(const GLHit& hit, m_hits) {
          if(hit.type() == Primitive::AtomType) {// Atom selection
            Atom *atom = molecule->atom(hit.name());
            hitList.append(atom);
            widget->toggleSelected(hitList);
            m_currentPrimitive = atom;
            m_atomMenu->exec(event->globalPos());
            widget->clearSelected();
            break;
          }
        }
      }
      else {
        widget->clearSelected();
      }
      event->accept();
    }

    widget->update();

    return 0;
  }

  QUndoCommand* SelectRotateTool::mouseMoveEvent(GLWidget *widget, QMouseEvent *event)
  {
    if (m_leftButtonPressed && !m_hits.size()) {
      event->accept();
      QPoint deltaDragging = event->pos() - m_lastDraggingPosition;

      if( ( event->pos() - m_initialDraggingPosition ).manhattanLength() > 2 )
        m_movedSinceButtonPressed = true;

      m_lastDraggingPosition = event->pos();
      widget->update();
    }
    else /*if (m_leftButtonPressed)*/ {
      if((event->pos() - m_initialDraggingPosition).manhattanLength() > 2)
        m_movedSinceButtonPressed = true;
      else
        event->accept();
    }

    return 0;
  }

  QUndoCommand* SelectRotateTool::mouseDoubleClickEvent(GLWidget *widget, QMouseEvent *event)
  {
    m_doubleClick = true; // events are handled here, not in release event

    // Intuitively, select this connected component
    m_widget = widget; // save for defining centroids

    Molecule *molecule = widget->molecule();
    if(!molecule)
      return 0;

    // List of hits from a selection/pick
    m_hits = widget->hits(event->pos().x()-SEL_BOX_HALF_SIZE,
        event->pos().y()-SEL_BOX_HALF_SIZE,
        SEL_BOX_SIZE, SEL_BOX_SIZE);

    if (!m_hits.size()) {
      event->ignore();
      return 0; // ignore this
    }

    // OK, we have some hits
    event->accept();
    QList<Primitive *> hitList;
    foreach (const GLHit& hit, m_hits) {
      if(hit.type() == Primitive::AtomType) // Atom selection
        {
          Atom *atom = molecule->atom(hit.name());
          hitList.append(atom);
          break;
        }
      else if(hit.type() == Primitive::BondType) // Bond selection
        {
          Bond *bond = molecule->bond(hit.name());
          hitList.append(bond);
          break;
        }
    }


    // OK, now process the hitList to select the connected component
    foreach(Primitive *hit, hitList) {
      if (hit->type() == Primitive::AtomType) {
        Atom *atom = static_cast<Atom *>(hit);
        QList<Primitive *> neighborList;

        // We really want the "connected fragment" since a Molecule can contain
        // multiple user-visible molecule fragments
        // we can use either BFS or DFS interators -- look for the connected fragment
        OpenBabel::OBMol mol = molecule->OBMol();
        OpenBabel::OBMolAtomDFSIter iter(mol, atom->index() + 1);
        Atom *tmpNeighbor;
        do {
          tmpNeighbor = molecule->atom(iter->GetIdx() - 1);
          neighborList.append(tmpNeighbor);

          // we want to find all bonds on this site
          // (obviously all bonds will be in this fragment)

          FOR_BONDS_OF_ATOM(b, *iter)
            neighborList.append(molecule->bond(b->GetIdx()));

        } while ((iter++).next()); // this returns false when we've gone looped through the fragment

        widget->setSelected(neighborList, select);
      }
      else if (hit->type() == Primitive::BondType) {
        Bond *bond = static_cast<Bond *>(hit);
        QList<Primitive *> neighborList;

        // We really want the "connected fragment" since a Molecule can contain
        // multiple user-visible molecule fragments
        // we can use either BFS or DFS interators -- look for the connected fragment
        OpenBabel::OBMol mol = molecule->OBMol();
        OpenBabel::OBMolAtomDFSIter iter(mol, molecule->atomById(bond->beginAtomId())->index() + 1);
        Atom *tmpNeighbor;
        do {
          tmpNeighbor = molecule->atom(iter->GetIdx() - 1);
          neighborList.append(tmpNeighbor);

          // we want to find all bonds on this site
          // (obviously all bonds will be in this fragment)
          FOR_BONDS_OF_ATOM(b, *iter)
            neighborList.append(molecule->bond(b->GetIdx()));
        } while ((iter++).next()); // this returns false when we've gone looped through the fragment

        widget->setSelected(neighborList, select);
      } // end of handling bond primitives
    } // end of processing hits

    // Reset the cursor
    widget->setCursor(Qt::ArrowCursor);
    return 0;
  }

  QUndoCommand* SelectRotateTool::wheelEvent(GLWidget*, QWheelEvent*)
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

    gluUnProject(float(sx), viewport[3] - float(sy), 0.1, modelview,
                 projection, viewport, &startPos[0], &startPos[1], &startPos[2]);
    gluUnProject(float(ex), viewport[3] - float(ey), 0.1, modelview,
                 projection, viewport, &endPos[0], &endPos[1], &endPos[2]);

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

  void SelectRotateTool::defineCentroid(bool)
  {
    if (!m_widget)
      return;

    // loop through selected atoms
    QList<Primitive*> selectedAtoms = m_widget->selectedPrimitives().subList(Primitive::AtomType);
    if (selectedAtoms.isEmpty()) { // no selected atoms, we want the global center
      m_selectedPrimitivesCenter = m_widget->center();
    } else {
      // Calculate the centroid of the selection
      foreach(Primitive *item, selectedAtoms) {
        // Atom::pos() returns a pointer to the position
        m_selectedPrimitivesCenter += *(static_cast<Atom*>(item)->pos());
      }
      m_selectedPrimitivesCenter /= double(selectedAtoms.size());
    }

    // OK, now create a dummy atom at that point
    Atom *atom = m_widget->molecule()->addAtom();
    atom->setAtomicNumber(0);
    atom->setPos(m_selectedPrimitivesCenter);
    m_widget->update();
  }

  void SelectRotateTool::defineCenterOfMass(bool)
  {
    if (!m_widget)
      return;

    // loop through selected atoms
    QList<Primitive*> selectedAtoms = m_widget->selectedPrimitives().subList(Primitive::AtomType);
    if (selectedAtoms.isEmpty()) { // no selected atoms, we want the global center
      foreach(Atom *atom, m_widget->molecule()->atoms()) {
        selectedAtoms.append(atom);
      }
    }

    // Calculate the centroid of the selection
    Vector3d selectedCenter(0.0, 0.0, 0.0);
    double atomMass, totalMass = 0.0;
    Atom *atom;
    foreach(Primitive *item, selectedAtoms) {
      // Atom::pos() returns a pointer to the position
      atom = static_cast<Atom*>(item);
      atomMass = OpenBabel::etab.GetMass(atom->atomicNumber()); // TODO: Does not consider isotopes
      selectedCenter += *(static_cast<Atom*>(item)->pos()) * atomMass;
      totalMass += atomMass;
    }
    selectedCenter /= totalMass;

    // OK, now create a dummy atom at that point
    atom = m_widget->molecule()->addAtom();
    atom->setAtomicNumber(0);
    atom->setPos(selectedCenter);
    m_widget->update();
  }

  QWidget *SelectRotateTool::settingsWidget()
  {
    if(!m_settingsWidget) {
      m_settingsWidget = new QWidget;

      QLabel *labelMode = new QLabel(tr("Selection Mode:"), m_settingsWidget);
      labelMode->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
      labelMode->setMaximumHeight(15);

      m_comboSelectionMode = new QComboBox(m_settingsWidget);
      m_comboSelectionMode->addItem(tr("Atom/Bond"));
      m_comboSelectionMode->addItem(tr("Residue"));
      m_comboSelectionMode->addItem(tr("Molecule"));

      QPushButton *centroidButton = new QPushButton(tr("Add Center of Atoms"), m_settingsWidget);
      QPushButton *centerOfMassButton = new QPushButton(tr("Add Center of Mass"), m_settingsWidget);

      QHBoxLayout* tmp = new QHBoxLayout;
      tmp->addWidget(labelMode);
      tmp->addWidget(m_comboSelectionMode);
      tmp->addStretch(1);
      m_layout = new QVBoxLayout();
      m_layout->addLayout(tmp);
      m_layout->addWidget(centroidButton);
      m_layout->addWidget(centerOfMassButton);
      m_layout->addStretch(1);
      m_settingsWidget->setLayout(m_layout);

      connect(m_comboSelectionMode, SIGNAL(currentIndexChanged(int)),
              this, SLOT(selectionModeChanged(int)));
      connect(centroidButton, SIGNAL(clicked(bool)),
              this, SLOT(defineCentroid(bool)));
      connect(centerOfMassButton, SIGNAL(clicked(bool)),
              this, SLOT(defineCenterOfMass(bool)));

      connect(m_settingsWidget, SIGNAL(destroyed()),
              this, SLOT(settingsWidgetDestroyed()));
    }

    return m_settingsWidget;
  }

  void SelectRotateTool::settingsWidgetDestroyed()
  {
    m_settingsWidget = 0;
  }

  bool SelectRotateTool::paint(GLWidget *)
  {
    if(m_selectionBox) {
      selectionBox(m_initialDraggingPosition.x(), m_initialDraggingPosition.y(),
                   m_lastDraggingPosition.x(), m_lastDraggingPosition.y());
    }
    return true;
  }

  void SelectRotateTool::changeAtomColor()
  {
    QColor color;
    QColor oldColor;
    if(m_currentPrimitive->type() == Primitive::AtomType) {
      Atom *a = static_cast<Atom*>(m_currentPrimitive);
      if (!a)
        return;
      oldColor.setNamedColor(a->customColorName());
      if(!oldColor.isValid()) {
        Color *map = GLWidget::current()->colorMap(); // fall back to global color map
        map->setFromPrimitive(a);
        oldColor.setRgb(map->color().rgb());
      }
      color = QColorDialog::getColor(oldColor, 0, tr("Change color of the atom"));
      if (color.isValid() && color != oldColor)
        a->setCustomColorName(color.name());
    }
  }

  void SelectRotateTool::resetAtomColor()
  {
    if(m_currentPrimitive->type() == Primitive::AtomType) {
      Atom *a = static_cast<Atom*>(m_currentPrimitive);
      if (!a)
        return;
      a->setCustomColorName("");
    }
  }

  void SelectRotateTool::changeAtomLabel()
  {
     bool ok;
     QString label;
     if(m_currentPrimitive->type() == Primitive::AtomType) {
       Atom *a = static_cast<Atom*>(m_currentPrimitive);
       if (!a)
         return;
       label = QInputDialog::getText(0, tr("Change label of the atom"),
         tr("New Label:"), QLineEdit::Normal,a->customLabel(), &ok);
       if (ok && !label.isEmpty())
         a->setCustomLabel(label);
     }
     else if(m_currentPrimitive->type() == Primitive::BondType) {
       Bond *b = static_cast<Bond*>(m_currentPrimitive);
       if(!b)
         return;
       label = QInputDialog::getText(0, tr("Change label of the bond"),
         tr("New Label:"), QLineEdit::Normal,b->customLabel(), &ok);
       if (ok && !label.isEmpty())
         b->setCustomLabel(label);
     }
  }

  void SelectRotateTool::resetAtomLabel()
  {
     if(m_currentPrimitive->type() == Primitive::AtomType) {
       Atom *a = static_cast<Atom*>(m_currentPrimitive);
       if (!a)
         return;
       a->setCustomLabel("");
     }
     else if(m_currentPrimitive->type() == Primitive::BondType) {
       Bond *b = static_cast<Bond*>(m_currentPrimitive);
       if(!b)
         return;
       b->setCustomLabel("");
     }
  }

  void SelectRotateTool::changeAtomRadius()
  {
     bool ok;
     QString radius_str;
     double radius;
     if(m_currentPrimitive->type() == Primitive::AtomType) {
       Atom *a = static_cast<Atom*>(m_currentPrimitive);
       if (!a)
         return;
       radius_str = QInputDialog::getText(0, tr("Change radius of the atom"),
         tr("New Radius, %1:", "in Angstrom").arg("(\xC5)"),
         QLineEdit::Normal,QString::number(a->customRadius()), &ok);
       if (!ok && radius_str.isEmpty())
         return;
       radius = radius_str.toDouble();
       if (radius)
         a->setCustomRadius(radius);
     }
  }

  void SelectRotateTool::resetAtomRadius()
  {
    if(m_currentPrimitive->type() == Primitive::AtomType) {
      Atom *a = static_cast<Atom*>(m_currentPrimitive);
      if (!a)
        return;
      a->setCustomRadius(0);
    }
  }

}

Q_EXPORT_PLUGIN2(selectrotatetool, Avogadro::SelectRotateToolFactory)
