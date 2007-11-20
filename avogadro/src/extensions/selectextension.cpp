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
      ResidueTypeIndex,
      SMARTSIndex
    };

  SelectExtension::SelectExtension(QObject *parent) : QObject(parent)
  {
    QAction *action;

    action = new QAction(this);
    action->setText(tr("&Invert Selection"));
    action->setData(InvertIndex);
    m_actions.append(action);

    action = new QAction(this);
    action->setText(tr("Select SMARTS..."));
    action->setData(SMARTSIndex);
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

} // end namespace Avogadro

#include "selectextension.moc"
Q_EXPORT_PLUGIN2(selectextension, Avogadro::SelectExtensionFactory)
