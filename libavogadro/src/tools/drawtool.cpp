/**********************************************************************
  DrawTool - Tool for drawing molecules

  Copyright (C) 2007,2008 Donald Ephraim Curtis
  Copyright (C) 2007-2008 Marcus D. Hanwell
  Copyright (C) 2008 Tim Vandermeersch
  Some Portions Copyright (C) 2007-2008 Geoffrey Hutchison

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

#include "drawtool.h"
#include "drawcommand.h"

#include <avogadro/navigate.h>
#include <avogadro/primitive.h>
#include <avogadro/atom.h>
#include <avogadro/bond.h>
#include <avogadro/molecule.h>
#include <avogadro/color.h>
#include <avogadro/glwidget.h>
#include <avogadro/undosequence.h>
#include <avogadro/elementtranslate.h>

#include <Eigen/Core>

#include <openbabel/obiter.h>
#include <openbabel/obconversion.h>

#include <QtPlugin>
#include <QLabel>
#include <QDir>
#include <QDebug>

using namespace std;
using namespace Eigen;
using namespace OpenBabel;

namespace Avogadro {

  DrawTool::DrawTool(QObject *parent) : Tool(parent),
                                        m_beginAtomAdded(false),
                                        m_endAtomAdded(false),
                                        m_beginAtom(0),
                                        m_endAtom(0),
                                        m_element(6),
                                        m_bond(0),
                                        m_bondOrder(1),
                                        m_prevAtomElement(0),
                                        m_prevBond(0),
                                        m_prevBondOrder(0),
                                        m_addHydrogens(2),
                                        m_comboElements(0),
                                        m_addHydrogensCheck(0),
                                        m_periodicTable(0),
                                        m_fragmentDialog(0),
                                        m_settingsWidget(0)
  {
    QAction *action = activateAction();
    action->setIcon(QIcon(QString::fromUtf8(":/draw/draw.png")));
    action->setToolTip(tr("Draw Tool (F8)\n\n"
                          "Left Mouse: \tClick and Drag to create Atoms and Bonds\n"
                          "Right Mouse: Delete Atom"));
    action->setShortcut(Qt::Key_F8);

    m_insertFragmentMode = false;
    m_forceField = OBForceField::FindForceField("MMFF94");
  }

  DrawTool::~DrawTool()
  {
    if(m_settingsWidget) {
      m_settingsWidget->deleteLater();
    }
  }

  int DrawTool::usefulness() const
  {
    return 5000000;
  }

  // Handle a user click
  QUndoCommand* DrawTool::mousePress(GLWidget *widget, const QMouseEvent *event)
  {
    Molecule *molecule = widget->molecule();
    if(!molecule) {
      return 0;
    }

    QUndoCommand *undo = 0;

    _buttons = event->buttons();

    m_movedSinceButtonPressed = false;
    m_lastDraggingPosition = event->pos();
    m_initialDraggingPosition = event->pos();

    //! List of hits from a selection/pick
    m_hits = widget->hits(event->pos().x()-SEL_BOX_HALF_SIZE,
                          event->pos().y()-SEL_BOX_HALF_SIZE,
                          SEL_BOX_SIZE,
                          SEL_BOX_SIZE);

    if(_buttons & Qt::LeftButton) {
      // The user clicked on an atom
      if(m_hits.size() && (m_hits[0].type() == Primitive::AtomType)) {
        // "alchemy" -- change this atom to a new element
        m_beginAtom = molecule->atom(m_hits[0].name());

        if(m_beginAtom && (m_beginAtom->atomicNumber() != m_element)) {
          m_prevAtomElement = m_beginAtom->atomicNumber();
          m_beginAtom->setAtomicNumber(m_element);
          m_beginAtom->update();
        }
      }
      else if(m_hits.size() && (m_hits[0].type() == Primitive::BondType)) {
        // "alchemy" -- change the bond order of this bond
        Bond *bond = molecule->bond(m_hits[0].name());
        if (bond) { // if we can't find the bond, we can't do anything here

          // do not try to change X-H bond order when adjust hydrogens is on
          if(m_addHydrogens) {
            if (molecule->atomById(bond->beginAtomId())->isHydrogen() ||
                molecule->atomById(bond->endAtomId())->isHydrogen())
              return 0;
          }

          unsigned int bondOrder, oldBondOrder;
          oldBondOrder = bond->order();

          switch (oldBondOrder) {
          case 1:
            bondOrder = 2;
            break;
          case 2:
            bondOrder = 3;
            break;
          case 3:
            bondOrder = 1;
            break;
          default:
            bondOrder = 1;
          }
          bond->setOrder(bondOrder);

          undo = new ChangeBondOrderDrawCommand(widget->molecule(), bond,
                                                oldBondOrder, m_addHydrogens);
        }
      }
      else { // a genuine click in new space == create a new atom or fragment
        if (m_insertFragmentMode) { // insert a new fragment
          Eigen::Vector3d refPoint;
          if (m_beginAtom) {
            refPoint = m_beginAtom->pos();
          } else {
            refPoint = widget->center();
          }
          Eigen::Vector3d newMolPos = widget->camera()->unProject(event->pos(), refPoint);
          Molecule m_generatedMolecule = *m_fragmentDialog->fragment();
          m_generatedMolecule.center();
          m_generatedMolecule.translate(Vector3d(newMolPos.x(),
                                                 newMolPos.y(),
                                                 newMolPos.z()));
          undo = new InsertFragmentCommand(widget->molecule(), m_generatedMolecule);
        } // end insert fragment mode
        else { // create a new atom
          m_beginAtom = newAtom(widget, event->pos());
          m_beginAtomAdded = true;
          m_forceField->SetIgnoreAtom(m_beginAtom->index());
          m_beginAtom->update();
        } // place atoms
      } // hits
    } // left button

    return undo;
  }

  QUndoCommand* DrawTool::mouseMove(GLWidget *widget, const QMouseEvent *event)
  {
    Molecule *molecule = widget->molecule();
    if(!molecule) {
      return 0;
    }

    if((_buttons & Qt::LeftButton) && m_beginAtom) {
      m_hits = widget->hits(event->pos().x()-SEL_BOX_HALF_SIZE,
                            event->pos().y()-SEL_BOX_HALF_SIZE,
                            SEL_BOX_SIZE,
                            SEL_BOX_SIZE);

      bool hitBeginAtom = false;
      Atom *existingAtom = 0;
      if(m_hits.size()) {
        // parse our hits.  we want to know
        // if we hit another existing atom that is not
        // the m_endAtom which we created
        for(int i=0; i < m_hits.size() && !hitBeginAtom; i++) {
          if(m_hits[i].type() == Primitive::AtomType) {
            // hit the beginning atom: either moved here from somewhere else
            // or were already here.
            if(m_hits[i].name() == m_beginAtom->index()) {
              hitBeginAtom = true;
            }
            else if(!m_endAtom) {
              // we don't yet have an end atom but
              // hit another atom on screen -- bond to this
              existingAtom = molecule->atom(m_hits[i].name());
            }
            else if(m_hits[i].name() != m_endAtom->index()) {
              // hit a new atom which isn't our end atom
              existingAtom = molecule->atom(m_hits[i].name());
            }
          } // end hits.type == AtomType
        }
      } // end parsing hits

      if(hitBeginAtom) { // we came back to our original atom -- undo the bond
        if(m_endAtom) {
          molecule->deleteAtom(m_endAtom); // this also deletes bonds
          m_endAtomAdded = false;
          m_bond = 0;
          m_endAtom = 0;
          m_prevAtomElement = m_beginAtom->atomicNumber();
          m_beginAtom->setAtomicNumber(m_element);
          m_forceField->UnsetIgnoreAtom();
        }
        else if(m_bond) {
          //          Atom *oldAtom = (Atom *)m_bond->GetEndAtom();
          //          oldAtom->DeleteBond(m_bond);
          molecule->deleteBond(m_bond);
          m_bond=0;
          m_prevAtomElement = m_beginAtom->atomicNumber();
          m_beginAtom->setAtomicNumber(m_element);
        }
      }
      else {
        if(m_prevAtomElement) {
          m_beginAtom->setAtomicNumber(m_prevAtomElement);
          m_prevAtomElement = 0;
        }

        // we hit an existing atom != m_endAtom
        if(existingAtom) {
          m_forceField->UnsetIgnoreAtom();
          m_forceField->SetFixAtom(existingAtom->index());
          Bond *existingBond = molecule->bond(m_beginAtom, existingAtom);
          if(!existingBond) {
            if(m_prevBond) {
              m_prevBond->setOrder(m_prevBondOrder);
              m_prevBond = 0;
              m_prevBondOrder = 0;
            }

            if(m_bond) {
              if(m_endAtom) {
                m_endAtom->deleteBond(m_bond);
                molecule->deleteAtom(m_endAtom);
                m_endAtomAdded = false;
                m_endAtom = 0;
              } else {
                Atom *oldAtom = molecule->atomById(m_bond->endAtomId());
                oldAtom->deleteBond(m_bond);
              }
              m_bond->setEnd(existingAtom);
              existingAtom->addBond(m_bond);
            }
            else {
              m_bond = newBond(molecule, m_beginAtom, existingAtom);
            }
          } // end no existing bond
          else {
            if(m_prevBond && m_prevBond != existingBond) {
              m_prevBond->setOrder(m_prevBondOrder);
              m_prevBond = 0;
              m_prevBondOrder = 0;
            }
            if(!m_prevBond) {
              m_prevBond = existingBond;
              m_prevBondOrder = existingBond->order();
              existingBond->setOrder(m_bondOrder);
            }

            if(m_bond && m_bond != existingBond) {
              if(m_endAtom) {
                // will delete bonds too (namely m_bond)
                molecule->deleteAtom(m_endAtom);
                m_endAtomAdded = false;
                m_endAtom = 0;
              } else {
                molecule->deleteBond(m_bond);
              }
              m_bond = 0;
            }
          } // existing bond
        } // end existing atom

        // (!existingAtom && !hitBeginAtom)
        else if(!m_endAtom) {

          if(m_prevBond) {
            m_prevBond->setOrder(m_prevBondOrder);
            m_prevBond = 0;
            m_prevBondOrder = 0;
          }
          m_endAtom = newAtom(widget, event->pos());
          m_endAtomAdded = true;
          m_forceField->SetIgnoreAtom(m_endAtom->index());

          if(!m_bond) {
            m_bond = newBond(molecule, m_beginAtom, m_endAtom);
          }
          else {
            Atom *oldAtom = molecule->atomById(m_bond->endAtomId());
            oldAtom->deleteBond(m_bond);
            m_bond->setEnd(m_endAtom);
            m_endAtom->addBond(m_bond);
          }
        }
        else { // we're moving -- stretch a bond
          moveAtom(widget, m_endAtom, event->pos());
        }
      }
      molecule->update();
    }

    return 0;
  }

  QUndoCommand* DrawTool::mouseRelease(GLWidget *widget, const QMouseEvent *event)
  {
    QUndoCommand *undo = 0;
    Molecule *molecule = widget->molecule();

    if(_buttons & Qt::LeftButton && (event->modifiers() == Qt::NoModifier)) {

      if(m_beginAtomAdded || m_bond) {

        // only add hydrogens to the atoms if it's the only thing
        // we've drawn.  else addbonds will adjust hydrogens.
        int atomAddHydrogens = 0;
        if(m_addHydrogens)
        {
          // if no bond then add on undo and redo
          if(!m_bond) {
            atomAddHydrogens = 1;
          }
          // if bond then only remove on undo, rest is handled by bond
          else
          {
            atomAddHydrogens = 2;
          }
        }

        // if we add a bond then we don't need
        // we added At least the beginAtom or we created a bond to
        // an existing atom or to endAtom that we also created
        AddAtomDrawCommand *beginAtomDrawCommand = 0;
        if(m_beginAtomAdded) {
          beginAtomDrawCommand = new AddAtomDrawCommand(widget->molecule(), m_beginAtom, atomAddHydrogens);
          beginAtomDrawCommand->setText(tr("Draw Atom"));
        }

        AddAtomDrawCommand *endAtomDrawCommand = 0;
        if(m_endAtomAdded) {
          endAtomDrawCommand = new AddAtomDrawCommand(widget->molecule(), m_endAtom, atomAddHydrogens);
          endAtomDrawCommand->setText(tr("Draw Atom"));
        }

        AddBondDrawCommand *bondCommand = 0;
        if(m_bond) {
          bondCommand = new AddBondDrawCommand(widget->molecule(), m_bond, m_addHydrogens);
          bondCommand->setText(tr("Draw Bond"));
        }

        // Set the actual undo command -- combining sequence if possible
        // we can have a beginAtom w/out bond or endAtom
        // we can have bond w/out endAtom (i.e., to an existing atom)
        // we cannot have endAtom w/out bond
        if(endAtomDrawCommand || (bondCommand && beginAtomDrawCommand)) {
          UndoSequence *seq = new UndoSequence();
          seq->setText(tr("Draw"));

          if(beginAtomDrawCommand) {
            seq->append(beginAtomDrawCommand);
          }
          if(endAtomDrawCommand) {
            seq->append(endAtomDrawCommand);
          }
          seq->append(bondCommand);

          undo = seq;
        }
        else if(bondCommand) {
          undo = bondCommand;
        }
        else {
          undo = beginAtomDrawCommand;
        }
      } else if (m_prevBond) {
        // bug #1898118
        // both beginAtom, endAtom and bond exist, but the bond order has changed
        if (m_prevBond->order() != m_prevBondOrder) {
          // do not try to change X-H bond order when adjust hydrogens is on
          if(m_addHydrogens) {
            if (molecule->atomById(m_prevBond->beginAtomId())->isHydrogen() ||
                molecule->atomById(m_prevBond->endAtomId())->isHydrogen()) {
              m_prevBond->setOrder(1); // restore
              return 0;
            }
          }

          undo = new ChangeBondOrderDrawCommand(widget->molecule(), m_prevBond,
                                                m_prevBondOrder, m_addHydrogens);
          undo->setText(tr("Change Bond Order"));
        }
      } else if (m_beginAtom) {
        // beginAtom exists, but we have no bond, we change the element
        if (m_beginAtom->atomicNumber() != m_prevAtomElement) {
          undo = new ChangeElementDrawCommand(widget->molecule(),
                                              m_beginAtom,
                                              m_prevAtomElement,
                                              m_addHydrogens);
        }
      }

      // clean up after drawing
      m_beginAtom=0;
      m_bond=0;
      m_endAtom=0;
      m_prevBond=0;
      m_prevBondOrder=0;
      m_prevAtomElement=0;
      m_beginAtomAdded=false;
      m_endAtomAdded=false;

      m_forceField->UnsetIgnoreAtom();
      m_forceField->UnsetFixAtom();

      widget->molecule()->update();
      return undo;
    }

    // Either use a three-button mouse
    // or hold down the Command key (ControlModifier in Qt notation)
    // or the Control key (MetaModifier in Qt notation)
    else if( (_buttons & Qt::RightButton) ||
             ((_buttons & Qt::LeftButton) && (event->modifiers() == Qt::ControlModifier || event->modifiers() == Qt::MetaModifier)) )
    {
      m_hits = widget->hits(event->pos().x()-SEL_BOX_HALF_SIZE,
                            event->pos().y()-SEL_BOX_HALF_SIZE,
                            SEL_BOX_SIZE,
                            SEL_BOX_SIZE);
      if(m_hits.size()) {
        // We did a right-click on an atom or bond -- delete it!
        if(m_hits[0].type() == Primitive::AtomType) {
          // don't delete H-? atom when adjust hydrogens is on
          Atom *atom = widget->molecule()->atom(m_hits[0].name());
          if (m_addHydrogens && atom->isHydrogen() && atom->valence())
            return undo;
          undo = new DeleteAtomDrawCommand(widget->molecule(), m_hits[0].name(),
                                           m_addHydrogens);
        }
        if(m_hits[0].type() == Primitive::BondType) {
          // don't delete ?-H bonds when adjust hydrogens is on
          Bond *bond = widget->molecule()->bond(m_hits[0].name());
          if (m_addHydrogens)
            if (molecule->atomById(bond->beginAtomId())->isHydrogen() ||
                molecule->atomById(bond->endAtomId())->isHydrogen())
              return undo;
          undo = new DeleteBondDrawCommand(widget->molecule(), m_hits[0].name(),
                                           m_addHydrogens);
        }
      }
      widget->molecule()->update();
    }

    return undo;
  }

  // Zoom the camera
  QUndoCommand* DrawTool::wheel(GLWidget *widget, const QWheelEvent *event)
  {
    // let's set the reference to be the center of the visible
    // part of the molecule.
    Eigen::Vector3d atomsBarycenter(0., 0., 0.);
    double sumOfWeights = 0.;
    if(widget->molecule()->numAtoms()) {
      QList<Atom*> atoms = widget->molecule()->atoms();
      foreach(Atom* atom, atoms) {
        Eigen::Vector3d transformedAtomPos = widget->camera()->modelview() * atom->pos();
        double atomDistance = transformedAtomPos.norm();
        double dot = transformedAtomPos.z() / atomDistance;
        double weight = exp(-30. * (1. + dot));
        sumOfWeights += weight;
        atomsBarycenter += weight * atom->pos();
      }
      atomsBarycenter /= sumOfWeights;
    }

    Navigate::zoom(widget, atomsBarycenter, -MOUSE_WHEEL_SPEED*event->delta());
    widget->update();

    return NULL;
  }

  Atom *DrawTool::newAtom(GLWidget *widget, const QPoint& p)
  {
    Atom *atom = widget->molecule()->newAtom();
    moveAtom(widget, atom, p);
    atom->setAtomicNumber(element());
    return atom;
  }

  void DrawTool::moveAtom(GLWidget *widget, Atom *atom, const QPoint& p)
  {
    Eigen::Vector3d refPoint;
    if(m_beginAtom) {
      refPoint = m_beginAtom->pos();
    } else {
      refPoint = widget->center();
    }
    Eigen::Vector3d newAtomPos = widget->camera()->unProject(p, refPoint);

    atom->setPos(newAtomPos);
  }


  Bond *DrawTool::newBond(Molecule *molecule, Atom *beginAtom, Atom *endAtom)
  {
    Bond *bond = molecule->newBond();
    bond->setOrder(bondOrder());
    bond->setBegin(beginAtom);
    bond->setEnd(endAtom);
    beginAtom->addBond(bond);
    endAtom->addBond(bond);
    return bond;
  }

  //
  // Settings widget
  // and associated signals/slots
  //

  void DrawTool::elementChanged( int index )
  {
    // The popup menu returns this signal
    // First case: we pick a common element from the popup
    if (index < m_elementsIndex.size() - 1) {
      setElement(m_elementsIndex[index]);
    }
    // Second case: we have a custom element "Other..."
    // Bring up the periodic table widget
    else {
      if (m_periodicTable) {
        m_periodicTable->show();
      }
    }
  }

  void DrawTool::customElementChanged( int index )
  {
    // Set the element so we can draw with it
    setElement(index);

    // Check to see if we already have this in the comboBox list
    // If not, we get back -1 and need to create a new item
    int comboItem = m_elementsIndex.indexOf(index);
    if (comboItem != -1) {
      m_comboElements->setCurrentIndex(comboItem);
      return; // we found it in the list, so we're done
    }

    // Find where we should put the new entry
    // (i.e., in order by atomic number)
    int position = 0;
    foreach(int entry, m_elementsIndex) {
      // Two cases: entry > index -- insert the new element before this one
      // Or... we hit the "Other" menu choice -- also insert here
      if (entry > index || entry == 0)
        break;

      ++position;
    }

    // And now we set up a new entry into the combo list
    QString entryName(elementTranslator.name(index)); // (e.g., "Hydrogen")
    entryName += " (" + QString::number(index) + ')';

    m_elementsIndex.insert(position, index);
    m_comboElements->insertItem(position, entryName);
    m_comboElements->setCurrentIndex(position);
  }

  void DrawTool::setElement( int index )
  {
    m_element = index;
  }

  int DrawTool::element() const
  {
    return m_element;
  }

  void DrawTool::bondOrderChanged( int index )
  {
    setBondOrder(index + 1);
  }

  void DrawTool::setBondOrder( int index )
  {
    m_bondOrder = index;
  }

  int DrawTool::bondOrder() const
  {
    return m_bondOrder;
  }

  void DrawTool::setAddHydrogens( int state )
  {
    m_addHydrogens = state;
  }

  int DrawTool::addHydrogens() const
  {
    return m_addHydrogens;
  }

  void DrawTool::setInsertFragmentMode( bool mode )
  {
    m_insertFragmentMode = mode;
  }

  QWidget *DrawTool::settingsWidget() {
    if(!m_settingsWidget) {
      m_settingsWidget = new QWidget;

      QLabel *labelElement = new QLabel(tr("Element:"));
      labelElement->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
      labelElement->setMaximumHeight(15);

      // Small popup with 10 most common elements for organic chemistry
      // (and extra for "other" to bring up periodic table window)
      m_comboElements = new QComboBox(m_settingsWidget);
      m_comboElements->addItem(elementTranslator.name(1) + " (1)");
      m_elementsIndex.append(1);
      m_comboElements->addItem(elementTranslator.name(5) + " (5)");
      m_elementsIndex.append(5);
      m_comboElements->addItem(elementTranslator.name(6) + " (6)");
      m_elementsIndex.append(6);
      m_comboElements->addItem(elementTranslator.name(7) + " (7)");
      m_elementsIndex.append(7);
      m_comboElements->addItem(elementTranslator.name(8) + " (8)");
      m_elementsIndex.append(8);
      m_comboElements->addItem(elementTranslator.name(9) + " (9)");
      m_elementsIndex.append(9);
      m_comboElements->addItem(elementTranslator.name(15) + " (15)");
      m_elementsIndex.append(15);
      m_comboElements->addItem(elementTranslator.name(16) + " (16)");
      m_elementsIndex.append(16);
      m_comboElements->addItem(elementTranslator.name(17) + " (17)");
      m_elementsIndex.append(17);
      m_comboElements->addItem(elementTranslator.name(35) + " (35)");
      m_elementsIndex.append(35);
      m_comboElements->addItem(tr("Other..."));
      m_elementsIndex.append(0);
      m_comboElements->setCurrentIndex(2);

      QLabel *labelBO = new QLabel(tr("Bond Order:"));
      labelBO->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
      labelBO->setMaximumHeight(15);

      m_comboBondOrder = new QComboBox(m_settingsWidget);
      m_comboBondOrder->addItem(tr("Single"));
      m_comboBondOrder->addItem(tr("Double"));
      m_comboBondOrder->addItem(tr("Triple"));

      // Improve the layout of the widgets
      QHBoxLayout* tmp = new QHBoxLayout;
      tmp->addWidget(m_comboElements);
      tmp->addStretch(1);
      QHBoxLayout* tmp2 = new QHBoxLayout;
      tmp2->addWidget(m_comboBondOrder);
      tmp2->addStretch(1);
      QGridLayout* grid = new QGridLayout;
      grid->addWidget(labelElement, 0, 0, Qt::AlignRight);
      grid->addLayout(tmp, 0, 1);
      grid->addWidget(labelBO, 1, 0, Qt::AlignRight);
      grid->addLayout(tmp2, 1, 1);

      m_addHydrogensCheck = new QCheckBox(tr("Adjust Hydrogens"), m_settingsWidget);
      m_addHydrogensCheck->setCheckState((Qt::CheckState)m_addHydrogens);

      m_fragmentButton = new QPushButton(m_settingsWidget);
      m_fragmentButton->setText(tr("Fragment Library..."));
      QHBoxLayout* fragmentLayout = new QHBoxLayout;
      fragmentLayout->addStretch(1);
      fragmentLayout->addWidget(m_fragmentButton);
      fragmentLayout->addStretch(1);
      connect(m_fragmentButton, SIGNAL(clicked(bool)),
              this, SLOT(showFragmentDialog(bool)));

      m_fragmentDialog = new InsertFragmentDialog(m_settingsWidget);
      connect(m_fragmentDialog, SIGNAL(setInsertMode(bool)),
              this, SLOT(setInsertFragmentMode(bool)));

      m_periodicTable = new PeriodicTableView(m_settingsWidget);
      connect(m_periodicTable, SIGNAL(elementChanged(int)),
              this, SLOT(customElementChanged(int)));

      m_layout = new QVBoxLayout();
      m_layout->addLayout(grid);
      m_layout->addWidget(m_addHydrogensCheck);
      m_layout->addLayout(fragmentLayout);
      m_layout->addStretch(1);
      m_settingsWidget->setLayout(m_layout);

      connect(m_comboElements, SIGNAL(currentIndexChanged(int)),
              this, SLOT(elementChanged(int)));

      connect(m_comboBondOrder, SIGNAL(currentIndexChanged(int)),
              this, SLOT(bondOrderChanged(int)));

      connect(m_addHydrogensCheck, SIGNAL(stateChanged(int)),
              this, SLOT(setAddHydrogens(int)));

      connect(m_settingsWidget, SIGNAL(destroyed()),
              this, SLOT(settingsWidgetDestroyed()));
    }

    return m_settingsWidget;
  }

  void DrawTool::settingsWidgetDestroyed() {
    m_settingsWidget = 0;
  }

  void DrawTool::showFragmentDialog(bool) {
    if (m_fragmentDialog->isVisible()) {
      m_fragmentDialog->hide();
      m_insertFragmentMode = false;
    } else {
      m_fragmentDialog->show();
    }
  }

  void DrawTool::writeSettings(QSettings &settings) const
  {
    Tool::writeSettings(settings);
    settings.setValue("currentElement", element());
    settings.setValue("addHydrogens", m_addHydrogens);
    if (m_fragmentDialog) {
      settings.setValue("smiles", m_fragmentDialog->smilesString());
      settings.setValue("fragmentPath", m_fragmentDialog->directoryList().join("\n"));
    }
  }

  void DrawTool::readSettings(QSettings &settings)
  {
    Tool::readSettings(settings);
    setAddHydrogens(settings.value("addHydrogens", 2).toInt());
    setElement(settings.value("currentElement", 6).toInt());
    if (m_comboElements)
    {
      int index = 0;
      for (int i = 0; i < m_elementsIndex.size() - 1; ++i)
        if (m_elementsIndex.at(i) == element()) index = i;
      m_comboElements->setCurrentIndex(index);
    }
    if(m_addHydrogensCheck)
      m_addHydrogensCheck->setCheckState((Qt::CheckState)m_addHydrogens);
    if(m_fragmentDialog) {
      m_fragmentDialog->setSmilesString(settings.value("smiles").toString());
      if (settings.contains("fragmentPath")) {
        QString directoryList = settings.value("fragmentPath").toString();
        m_fragmentDialog->setDirectoryList(directoryList.split('\n'));
      }
    }
  }
}

#include "drawtool.moc"

Q_EXPORT_PLUGIN2(drawtool, Avogadro::DrawToolFactory)
