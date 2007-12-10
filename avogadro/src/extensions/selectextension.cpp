/**********************************************************************
  Selection - Various selection options for Avogadro

  Copyright (C) 2006-2007 by Donald Ephraim Curtis
  Copyright (C) 2006-2007 by Geoffrey R. Hutchison

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
#include <avogadro/primitive.h>
#include <avogadro/color.h>

#include <openbabel/mol.h>
#include <openbabel/parsmart.h>

#include <QtGui>

using namespace std;
using namespace OpenBabel;

namespace Avogadro {

  enum SelectionExtensionIndex
    {
      InvertIndex = 0,
      ElementIndex,
      ResidueIndex,
      SolventIndex,
      SMARTSIndex
    };

  SelectExtension::SelectExtension(QObject *parent) : QObject(parent)
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
    action->setText(tr("Select by residue..."));
    action->setData(ResidueIndex);
    m_actions.append(action);
    
    action = new QAction(this);
    action->setText(tr("Select solvent"));
    action->setData(SolventIndex);
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

  QUndoCommand* SelectExtension::performAction(QAction *action,
      Molecule *molecule, GLWidget *widget, QTextEdit *)
  {
    int i = action->data().toInt();

    // dispatch to the appropriate method for that selection command
    switch (i) {
    case InvertIndex:
      invertSelection(widget);
      break;
    case SMARTSIndex:
      selectSMARTS(molecule, widget);
      break;
    case ElementIndex:
      m_molecule = molecule;
      m_widget = widget;
      m_periodicTable->show();
      break;
    case ResidueIndex:
      selectResidue(molecule, widget);
      break;
    case SolventIndex:
      selectSolvent(molecule, widget);
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
    widget->toggleSelected(widget->primitives().list());
    widget->update(); // make sure to call for a redraw or you won't see it
    return;
  }

  // Helper function -- handle SMARTS selections
  // Called by performAction()
  void SelectExtension::selectSMARTS(Molecule *molecule, GLWidget *widget)
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
      smarts.Match(*molecule);

      // if we have matches, select them
      if(smarts.NumMatches() != 0) {
        QList<Primitive *> matchedAtoms;

        vector< vector <int> > mapList = smarts.GetUMapList();
        vector< vector <int> >::iterator i; // a set of matching atoms
        vector<int>::iterator j; // atom ids in each match
        for (i = mapList.begin(); i != mapList.end(); ++i)
          for (j = i->begin(); j != i->end(); ++j) {
            matchedAtoms.append(static_cast<Atom*>(molecule->GetAtom(*j)));
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
    QList<Primitive *> selectedAtoms;
    /*
    bool ok;
    int element = QInputDialog::getInteger(qobject_cast<QWidget*>(parent()), 
        tr("Select by element"), tr("Element number"), 6, 1, 103, 1, &ok);
    
    if (!ok)
      return;
    */
    FOR_ATOMS_OF_MOL (atom, m_molecule) {
      if (atom->GetAtomicNum() == element)
        selectedAtoms.append(static_cast<Atom*>(m_molecule->GetAtom(atom->GetIdx())));
    }

    m_widget->clearSelected();
    m_widget->setSelected(selectedAtoms, true);
    m_widget->update();
  }

 
  // Helper function -- handle residue selections
  // Called by performAction()
  void SelectExtension::selectResidue(Molecule *molecule, GLWidget *widget)
  {
    QList<Primitive *> selectedAtoms;
    bool ok;
    QString resname = QInputDialog::getText(qobject_cast<QWidget*>(parent()),
        tr("Select by residue"), tr("Residue name"), QLineEdit::Normal, tr(""), &ok);
    
    FOR_ATOMS_OF_MOL (atom, molecule) {
      if (atom->GetResidue()->GetName() == resname.toStdString())
        selectedAtoms.append(static_cast<Atom*>(molecule->GetAtom(atom->GetIdx())));
    }

    widget->clearSelected();
    widget->setSelected(selectedAtoms, true);
    widget->update();
  }

  // Helper function -- handle solvent selections
  // Called by performAction()
  void SelectExtension::selectSolvent(Molecule *molecule, GLWidget *widget)
  {
    QList<Primitive *> selectedAtoms;
    
    FOR_ATOMS_OF_MOL (atom, molecule) {
      if (atom->GetResidue()->GetName() == "HOH")
        selectedAtoms.append(static_cast<Atom*>(molecule->GetAtom(atom->GetIdx())));
    }

    widget->clearSelected();
    widget->setSelected(selectedAtoms, true);
    widget->update();
  }


} // end namespace Avogadro

#include "selectextension.moc"
Q_EXPORT_PLUGIN2(selectextension, Avogadro::SelectExtensionFactory)
