/**********************************************************************
  DrawTool - Tool for drawing molecules

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

#include "drawtool.h"
#include "drawcommand.h"
#include <avogadro/primitive.h>
#include <avogadro/color.h>
#include <avogadro/glwidget.h>
#include <avogadro/undosequence.h>

#include <openbabel/obiter.h>

#include <QtPlugin>

using namespace std;
using namespace OpenBabel;
using namespace Avogadro;

DrawTool::DrawTool(QObject *parent) : Tool(parent),
  m_beginAtomAdded(false),
  m_beginAtom(0),
  m_endAtom(0),
  m_element(6),
  m_bond(0),
  m_bondOrder(1),
  m_prevAtomElement(0),
  m_prevBond(0),
  m_prevBondOrder(0),
  //m_beginAtomDrawCommand(0), //m_bondCommand(0), m_endAtomDrawCommand(0),
  m_periodicTable(0),
  m_settingsWidget(0)
{
  QAction *action = activateAction();
  action->setIcon(QIcon(QString::fromUtf8(":/draw/draw.png")));
  action->setToolTip(tr("Draw Tool (F8)\n\n"
        "Left Mouse: \tClick and Drag to create Atoms and Bonds\n"
        "Right Mouse: Delete Atom"));
  action->setShortcut(Qt::Key_F8);
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

void DrawTool::elementChanged( int index )
{
  // The popup menu returns this signal
  // First case: we pick a common element from the popup
  if (index < m_elementsIndex.size() - 1) {
    setElement(m_elementsIndex[index]);
    if (m_periodicTable)
      m_periodicTable->setSelectedElement(m_elementsIndex[index]);
  }
  // Second case: we have a custom element "Other..."
  // Bring up the periodic table widget
  else {
    if (m_periodicTable) {
      m_periodicTable->setSelectedElement(m_element);
      m_periodicTable->show();
    }
  }
}

// TODO: Add additional entries to the element index (popup menu)
void DrawTool::customElementChanged( int index )
{
  setElement(index);
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

QUndoCommand* DrawTool::mousePress(GLWidget *widget, const QMouseEvent *event)
{
  Molecule *molecule = widget->molecule();
  if(!molecule) {
    return 0;
  }

  QUndoCommand *command = 0;

  _buttons = event->buttons();

  m_movedSinceButtonPressed = false;
  m_lastDraggingPosition = event->pos();
  m_initialDragginggPosition = event->pos();

  //! List of hits from a selection/pick
  m_hits = widget->hits(event->pos().x()-SEL_BOX_HALF_SIZE,
                        event->pos().y()-SEL_BOX_HALF_SIZE,
                        SEL_BOX_SIZE,
                        SEL_BOX_SIZE);

  if(_buttons & Qt::LeftButton)
  {
    if(m_hits.size() && (m_hits[0].type() == Primitive::AtomType))
    {
      // "alchemy" -- change this atom to a new element
      // Make sure we call BeginModify / EndModify (e.g., PR#1720879)
      widget->molecule()->BeginModify();
      m_beginAtom = (Atom *)molecule->GetAtom(m_hits[0].name());
      m_prevAtomElement = m_beginAtom->GetAtomicNum();
      m_beginAtom->SetAtomicNum(m_element);
      widget->molecule()->EndModify();
      m_beginAtom->update(); // Make sure to call for a repaint(#1741653).
      // FIXME: This should really be something we can undo
    }
    else
    {
      m_beginAtom = newAtom(widget, event->pos());
      m_beginAtomAdded = true;
      widget->updateGeometry();
      m_beginAtom->update();
    }
  }

  return command;
}

QUndoCommand* DrawTool::mouseMove(GLWidget *widget, const QMouseEvent *event)
{
  Molecule *molecule = widget->molecule();
  if(!molecule) {
    return 0;
  }

  if((_buttons & Qt::LeftButton) && m_beginAtom)
  {
    m_hits = widget->hits(event->pos().x()-SEL_BOX_HALF_SIZE,
                        event->pos().y()-SEL_BOX_HALF_SIZE,
                        SEL_BOX_SIZE,
                        SEL_BOX_SIZE);

    bool hitBeginAtom = false;
    Atom *existingAtom = 0;
    if(m_hits.size())
    {
      // parse our hits.  we want to know
      // if we hit another existingAtom that is not
      // the m_endAtom which we created
      for(int i=0; i < m_hits.size() && !hitBeginAtom; i++)
      {
        if(m_hits[i].type() == Primitive::AtomType)
        {
          // hit the same atom either moved here from somewhere else
          // or were already here.
          if(m_hits[i].name() == m_beginAtom->GetIdx())
          {
            hitBeginAtom = true;
          }
          else if(!m_endAtom)
          {
            existingAtom = (Atom *)molecule->GetAtom(m_hits[i].name());
          }
          else
          {
            if(m_hits[i].name() != m_endAtom->GetIdx())
            {
              existingAtom = (Atom *)molecule->GetAtom(m_hits[i].name());
            }
          }
        }
      }
    }
    if(hitBeginAtom)
    {
      if(m_endAtom)
      {
        molecule->DeleteAtom(m_endAtom);
        // widget->updateGeometry();
        m_bond = 0;
        m_endAtom = 0;
        m_prevAtomElement = m_beginAtom->GetAtomicNum();
        m_beginAtom->SetAtomicNum(m_element);
        // m_beginAtom->update();
      }
      else if(m_bond)
      {
        Atom *oldAtom = (Atom *)m_bond->GetEndAtom();
        oldAtom->DeleteBond(m_bond);
        molecule->DeleteBond(m_bond);
        m_bond=0;
        m_prevAtomElement = m_beginAtom->GetAtomicNum();
        m_beginAtom->SetAtomicNum(m_element);
        // m_beginAtom->update();
      }
    }
    else
    {
      if(m_prevAtomElement)
      {
        m_beginAtom->SetAtomicNum(m_prevAtomElement);
        m_prevAtomElement = 0;
      }

      // we hit an existing atom != m_endAtom
      if(existingAtom)
      {
        Bond *existingBond = (Bond *)molecule->GetBond(m_beginAtom, existingAtom);
        if(!existingBond) {
          if(m_prevBond)
          {
            m_prevBond->SetBondOrder(m_prevBondOrder);
            // m_prevBond->update();
            m_prevBond = 0;
            m_prevBondOrder = 0;
          }
          if(m_bond) {
            if(m_endAtom) {
              m_endAtom->DeleteBond(m_bond);
              molecule->DeleteAtom(m_endAtom);
              m_endAtom = 0;
            } else {
              Atom *oldAtom = (Atom *)m_bond->GetEndAtom();
              oldAtom->DeleteBond(m_bond);
            }
            m_bond->SetEnd(existingAtom);
            existingAtom->AddBond(m_bond);
            // m_bond->update();
          } else {
            m_bond = newBond(molecule, m_beginAtom, existingAtom);
            // m_bond->update();
          }
        }
        // (existingBond)
        else {
          if(m_prevBond && m_prevBond != existingBond) {
            m_prevBond->SetBondOrder(m_prevBondOrder);
            // m_prevBond->update();
            m_prevBond = 0;
            m_prevBondOrder = 0;
          }
          if(!m_prevBond) {
            m_prevBond = existingBond;
            m_prevBondOrder = existingBond->GetBO();
            existingBond->SetBondOrder(m_bondOrder);
            // existingBond->update();
          }

          if(m_bond && m_bond != existingBond) {
            if(m_endAtom) {
              // will delete bonds too (namely m_bond)
              molecule->DeleteAtom(m_endAtom);
              m_endAtom = 0;
            } else {
              molecule->DeleteBond(m_bond);
            }
            m_bond = 0;
          }
        }
      }
      // (!existingAtom && !hitBeginAtom)
      else if(!m_endAtom)
      {
        if(m_prevBond) {
          m_prevBond->SetBondOrder(m_prevBondOrder);
          // m_prevBond->update();
          m_prevBond = 0;
          m_prevBondOrder = 0;
        }
        m_endAtom = newAtom(widget, event->pos());
        if(!m_bond)
        {
          m_bond = newBond(molecule, m_beginAtom, m_endAtom);
        }
        else
        {
          Atom *oldAtom = (Atom *)m_bond->GetEndAtom();
          oldAtom->DeleteBond(m_bond);
          m_bond->SetEnd(m_endAtom);
          m_endAtom->AddBond(m_bond);
        }
        // m_bond->update();
        // m_endAtom->update();
        widget->updateGeometry();
      }
      else
      {
        moveAtom(widget, m_endAtom, event->pos());
        // widget->updateGeometry();
        // m_endAtom->update();
      }
    }
    molecule->update();
  }

  return 0;
}

QUndoCommand* DrawTool::mouseRelease(GLWidget *widget, const QMouseEvent *event)
{
  QUndoCommand *undo = 0;

#ifdef Q_WS_MAC
  if(_buttons & Qt::LeftButton && event->modifiers() == Qt::NoModifier)
#else
  if(_buttons & Qt::LeftButton)
#endif
  {
    // we can have a beginAtom w/out bond or endAtom
    // we can hava bond w/out endAtom
    // we cannot have endAtom w/out bond
    // i go through a lot of testing to make the text look prettier and save memory.
    if(m_beginAtomAdded || m_bond)
    {
      AddAtomDrawCommand *beginAtomDrawCommand = 0;
      if(m_beginAtomAdded) {
        beginAtomDrawCommand = new AddAtomDrawCommand(widget->molecule(), m_beginAtom);
        beginAtomDrawCommand->setText(tr("Draw Atom"));
      }

      AddAtomDrawCommand *endAtomDrawCommand = 0;
      if(m_endAtom) {
        endAtomDrawCommand = new AddAtomDrawCommand(widget->molecule(), m_endAtom);
        endAtomDrawCommand->setText(tr("Draw Atom"));
      }

      AddBondDrawCommand *bondCommand = 0;
      if(m_bond) {
        bondCommand = new AddBondDrawCommand(widget->molecule(), m_bond);
        bondCommand->setText(tr("Draw Bond"));
      }

      if(endAtomDrawCommand || (bondCommand && beginAtomDrawCommand))
      {
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
      else if(bondCommand)
      {
        undo = bondCommand;
      }
      else
      {
        undo = beginAtomDrawCommand;
      }
    }

    m_beginAtom=0;
    m_bond=0;
    m_endAtom=0;
    m_prevBond=0;
    m_prevBondOrder=0;
    m_prevAtomElement=0;
    m_beginAtomAdded=false;

    // create the undo action for creating endAtom and bond
    //  pass along atom idx, element, vector, bond idx, order, start/end
  }
#ifdef Q_WS_MAC
  // On the Mac, either use a three-button mouse
  // or hold down the Command key (ControlModifier in Qt notation)
  else if( (_buttons & Qt::RightButton) ||
           ((_buttons & Qt::LeftButton) && (event->modifiers() == Qt::ControlModifier)) )
#else
  // Every other platform, use a three-button mouse
  else if(_buttons & Qt::RightButton)
#endif
  {
    m_hits = widget->hits(event->pos().x()-SEL_BOX_HALF_SIZE,
                        event->pos().y()-SEL_BOX_HALF_SIZE,
                          SEL_BOX_SIZE,
                        SEL_BOX_SIZE);
    if(m_hits.size())
    {
      qDebug() << m_hits[0].name() << " -- " << m_hits[0].type();

      qDebug() << "bondtype -- " << Primitive::BondType;
      // get our top hit
      if(m_hits[0].type() == Primitive::AtomType)
      {
        undo = new DeleteAtomDrawCommand(widget->molecule(), m_hits[0].name());
//         molecule->DeleteAtom(atom);
//         widget->updateGeometry();
//         molecule->update();
      }
      if(m_hits[0].type() == Primitive::BondType)
      {
        qDebug() << m_hits[0].name();
        undo = new DeleteBondDrawCommand(widget->molecule(), m_hits[0].name());
//         molecule->DeleteAtom(atom);
//         widget->updateGeometry();
//         molecule->update();
      }
    }
  }

  return undo;
}

QUndoCommand* DrawTool::wheel(GLWidget *, const QWheelEvent *)
{
  return 0;
}

Atom *DrawTool::newAtom(GLWidget *widget, const QPoint& p)
{
  // GRH (for reasons I don't understand, calling Begin/EndModify here
  // causes crashes with multiple bond orders
  // (need to investigate, probable OB bug.

  widget->molecule()->BeginModify();
  Atom *atom = static_cast<Atom*>(widget->molecule()->NewAtom());
  moveAtom(widget, atom, p);
  atom->SetAtomicNum(element());
  widget->molecule()->EndModify();

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
  molecule->BeginModify();
  Bond *bond = (Bond *)molecule->NewBond();
  bond->SetBondOrder(bondOrder());
  bond->SetBegin(beginAtom);
  bond->SetEnd(endAtom);
  beginAtom->AddBond(bond);
  endAtom->AddBond(bond);
  molecule->EndModify();

  return bond;
}

QWidget *DrawTool::settingsWidget() {
  if(!m_settingsWidget) {
    m_settingsWidget = new QWidget;

    // Small popup with 10 most common elements for organic chemistry
    // (and extra for "other" to bring up periodic table window)
    m_comboElements = new QComboBox(m_settingsWidget);
    m_elementsIndex.reserve(11);
    m_comboElements->addItem("Hydrogen (1)");
    m_elementsIndex.append(1);
    m_comboElements->addItem("Boron (5)");
    m_elementsIndex.append(5);
    m_comboElements->addItem("Carbon (6)");
    m_elementsIndex.append(6);
    m_comboElements->addItem("Nitrogen (7)");
    m_elementsIndex.append(7);
    m_comboElements->addItem("Oxygen (8)");
    m_elementsIndex.append(8);
    m_comboElements->addItem("Fluorine (9)");
    m_elementsIndex.append(9);
    m_comboElements->addItem("Phosphorus (15)");
    m_elementsIndex.append(15);
    m_comboElements->addItem("Sulfur (16)");
    m_elementsIndex.append(16);
    m_comboElements->addItem("Chlorine (17)");
    m_elementsIndex.append(17);
    m_comboElements->addItem("Bromine (35)");
    m_elementsIndex.append(35);
    m_comboElements->addItem("Other...");
    m_elementsIndex.append(0);
    m_comboElements->setCurrentIndex(2);

    m_comboBondOrder = new QComboBox(m_settingsWidget);
    m_comboBondOrder->addItem("Single");
    m_comboBondOrder->addItem("Double");
    m_comboBondOrder->addItem("Triple");

    m_periodicTable = new PeriodicTableDialog(m_settingsWidget);
    connect(m_periodicTable, SIGNAL(elementChanged(int)),
            this, SLOT(customElementChanged(int)));

    m_layout = new QVBoxLayout();
    m_layout->addWidget(m_comboElements);
    m_layout->addWidget(m_comboBondOrder);
    m_settingsWidget->setLayout(m_layout);

    connect(m_comboElements, SIGNAL(currentIndexChanged(int)),
        this, SLOT(elementChanged(int)));

    connect(m_comboBondOrder, SIGNAL(currentIndexChanged(int)),
        this, SLOT(bondOrderChanged(int)));

    connect(m_settingsWidget, SIGNAL(destroyed()),
        this, SLOT(settingsWidgetDestroyed()));
  }

  return m_settingsWidget;
}

void DrawTool::settingsWidgetDestroyed() {
  m_settingsWidget = 0;
}


#include "drawtool.moc"

Q_EXPORT_PLUGIN2(drawtool, DrawToolFactory)
