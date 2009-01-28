/**********************************************************************
  Selection - Various selection options for Avogadro

  Copyright (C) 2006-2007 by Donald Ephraim Curtis
  Copyright (C) 2006-2007 by Geoffrey R. Hutchison
  Copyright (C) 2008 by Tim Vandermeersch

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

#include "selectextension.h"

#include <avogadro/molecule.h>
#include <avogadro/atom.h>
#include <avogadro/bond.h>
#include <avogadro/residue.h>
#include <avogadro/color.h>

#include <openbabel/mol.h>
#include <openbabel/parsmart.h>

#include <QLineEdit>
#include <QInputDialog>
#include <QMessageBox>
#include <QAction>
#include <QDockWidget>
#include <QDebug>

using namespace std;
using namespace OpenBabel;

namespace Avogadro {

  enum SelectionExtensionIndex
    {
      InvertIndex = 0,
      ElementIndex,
      ResidueIndex,
      SolventIndex,
      SMARTSIndex,
      AddNamedIndex,
      SeparatorIndex
    };

  SelectExtension::SelectExtension(QObject *parent) : Extension(parent)
  {
    m_periodicTable = new PeriodicTableView;
    connect( m_periodicTable, SIGNAL( elementChanged(int) ),
        this, SLOT( selectElement(int) ));
    QAction *action;

    action = new QAction(this);
    action->setText(tr("&Invert Selection"));
    action->setData(InvertIndex);
    m_actions.append(action);

    action = new QAction(this);
    action->setText(tr("Select SMARTS..."));
    action->setData(SMARTSIndex);
    m_actions.append(action);

    action = new QAction(this);
    action->setText(tr("Select by Element..."));
    action->setData(ElementIndex);
    m_actions.append(action);

    action = new QAction(this);
    action->setText(tr("Select by Residue..."));
    action->setData(ResidueIndex);
    m_actions.append(action);

    action = new QAction(this);
    action->setText(tr("Select Solvent"));
    action->setData(SolventIndex);
    m_actions.append(action);

    action = new QAction( this );
    action->setSeparator(true);
    action->setData(SeparatorIndex);
    m_actions.append( action );

    action = new QAction(this);
    action->setText(tr("Add Named Selection..."));
    action->setData(AddNamedIndex);
    m_actions.append(action);
  }

  SelectExtension::~SelectExtension()
  {
  }

  QList<QAction *> SelectExtension::actions() const
  {
    return m_actions;
  }

  QString SelectExtension::menuPath(QAction *) const
  {
    return tr("&Select");
  }

  void SelectExtension::setMolecule(Molecule *molecule)
  {
    m_molecule = molecule;
  }

  QUndoCommand* SelectExtension::performAction(QAction *action, GLWidget *widget)
  {
    int i = action->data().toInt();

    // dispatch to the appropriate method for that selection command
    switch (i) {
    case InvertIndex:
      invertSelection(widget);
      break;
    case SMARTSIndex:
      selectSMARTS(widget);
      break;
    case ElementIndex:
      m_widget = widget;
      m_periodicTable->show();
      break;
    case ResidueIndex:
      selectResidue(widget);
      break;
    case SolventIndex:
      selectSolvent(widget);
      break;
    case AddNamedIndex:
      addNamedSelection(widget);
      break;
    default:
      break;
    }

    // Selections are per-view and as such are not saved or undo-able
    return NULL;
  }

  // Helper function -- invert selection
  // Called by performAction()
  void SelectExtension::invertSelection(GLWidget *widget)
  {
    widget->toggleSelected();
    widget->update(); // make sure to call for a redraw or you won't see it
    return;
  }

  // Helper function -- handle SMARTS selections
  // Called by performAction()
  void SelectExtension::selectSMARTS(GLWidget *widget)
  {
    bool ok;
    QString pattern = QInputDialog::getText(qobject_cast<QWidget*>(parent()),
        tr("SMARTS Selection"),
        tr("SMARTS pattern to select"),
        QLineEdit::Normal,
        "", &ok);
    if (ok && !pattern.isEmpty()) {
      OBSmartsPattern smarts;
      smarts.Init(pattern.toStdString());
      OpenBabel::OBMol obmol = m_molecule->OBMol();
      smarts.Match(obmol);

      // if we have matches, select them
      if(smarts.NumMatches() != 0) {
        QList<Primitive *> matchedAtoms;

        vector< vector <int> > mapList = smarts.GetUMapList();
        vector< vector <int> >::iterator i; // a set of matching atoms
        vector<int>::iterator j; // atom ids in each match
        for (i = mapList.begin(); i != mapList.end(); ++i) {
          for (j = i->begin(); j != i->end(); ++j) {
            matchedAtoms.append(m_molecule->atom(obmol.GetAtom(*j)->GetIdx()-1));
          }
        }

        widget->clearSelected();
        widget->setSelected(matchedAtoms, true);
        widget->update();
      } // end matches
    }
    return;
  }

  // Helper function -- handle element selections
  // Connected to signal from PeriodicTableView
  void SelectExtension::selectElement(int element)
  {
    if(m_widget)
    {
      QList<Primitive *> selectedAtoms;

      QList<Atom *> atoms = m_molecule->atoms();
      foreach (Atom *atom, atoms) {
        if (atom->atomicNumber() == element) {
          selectedAtoms.append(atom);
        }
      }

      m_widget->clearSelected();
      m_widget->setSelected(selectedAtoms, true);
      m_widget->update();
    }
  }


  // Helper function -- handle residue selections
  // Called by performAction()
  void SelectExtension::selectResidue(GLWidget *widget)
  {
    QList<Primitive *> selectedAtoms;
    bool ok;
    QString resname = QInputDialog::getText(qobject_cast<QWidget*>(parent()),
        tr("Select by residue"), tr("Residue name"), QLineEdit::Normal, "", &ok);

    QList<Residue*> residues = m_molecule->residues();
    foreach (Residue *res, residues) {
      if (res->name() == resname) {
        QList<unsigned long int> atoms = res->atoms();
        foreach(unsigned long int atom, atoms) {
          selectedAtoms.push_back(m_molecule->atomById(atom));
        }
        QList<unsigned long int> bonds = res->bonds();
        foreach(unsigned long int bond, bonds) {
          selectedAtoms.push_back(m_molecule->bondById(bond));
        }
      }
    }
    widget->clearSelected();
    widget->setSelected(selectedAtoms, true);
    widget->update();
  }

  // Helper function -- handle solvent selections
  // Called by performAction()
  void SelectExtension::selectSolvent(GLWidget *widget)
  {
    QList<Primitive *> selectedAtoms;

    QList<Residue*> residues = m_molecule->residues();
    foreach (Residue *res, residues) {
      if (res->name() == "HOH") {
        QList<unsigned long int> atoms = res->atoms();
        foreach(unsigned long int atom, atoms) {
          selectedAtoms.push_back(m_molecule->atomById(atom));
        }
        QList<unsigned long int> bonds = res->bonds();
        foreach(unsigned long int bond, bonds) {
          selectedAtoms.push_back(m_molecule->bondById(bond));
        }
      }
    }
    widget->clearSelected();
    widget->setSelected(selectedAtoms, true);
    widget->update();
  }

  void SelectExtension::addNamedSelection(GLWidget *widget)
  {
    PrimitiveList primitives = widget->selectedPrimitives();

    if (primitives.isEmpty()) {
      QMessageBox::warning( widget, tr( "Avogadro" ),
        tr( "There is no current selection." ));
      return;
    }

    bool ok;
    QString name = QInputDialog::getText(qobject_cast<QWidget*>(parent()),
        tr("Add Named Selection"), tr("name"), QLineEdit::Normal, "", &ok);

    if (!ok) return;

    if (name.isEmpty()) {
      QMessageBox::warning( widget, tr( "Avogadro" ),
        tr( "Name cannot be empty." ));
      return;
    }

    if (!widget->addNamedSelection(name, primitives)) {
       QMessageBox::warning( widget, tr( "Avogadro" ),
        tr( "There is already a selection with this name." ));
    }
  }

} // end namespace Avogadro

#include "selectextension.moc"
Q_EXPORT_PLUGIN2(selectextension, Avogadro::SelectExtensionFactory)
