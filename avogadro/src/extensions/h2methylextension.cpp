/**********************************************************************
  H2Methyl - Hydrogen to Methyl plugin for Avogadro

  Copyright (C) 2006 by Donald Ephraim Curtis
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

#include "h2methylextension.h"
#include <avogadro/primitive.h>
#include <avogadro/color.h>
#include <avogadro/glwidget.h>

#include <openbabel/obiter.h>

#include <QtGui>

using namespace std;
using namespace OpenBabel;

  namespace Avogadro {
    H2MethylExtension::H2MethylExtension(QObject *parent) : QObject(parent)
    {
      QAction *action = new QAction(this);
      action->setText(tr("Change H to Methyl"));
      m_actions.append(action);
    }

    H2MethylExtension::~H2MethylExtension()
    {
    }

    QList<QAction *> H2MethylExtension::actions() const
    {
      return m_actions;
    }

    QUndoCommand* H2MethylExtension::performAction(QAction *action, Molecule *molecule, GLWidget *widget, QTextEdit *messages)
    {
      QUndoCommand *undo = 0;
      undo = new H2MethylCommand(molecule, widget);
      return undo;
    }

    H2MethylCommand::H2MethylCommand(Molecule *molecule, GLWidget *widget):
      m_molecule(molecule), m_moleculeCopy(*molecule),
      m_SelectedList(widget->selectedPrimitives())
    {
      // save the selection from the current view widget
      // (i.e., only modify a few hydrogens)
      //      m_SelectedList = widget->selectedPrimitives;

      setText(QObject::tr("H to Methyl"));
    }

    void H2MethylCommand::redo()
    {
      if (m_SelectedList.size() == 0) {
        QList<Atom*> hydrogenList;
        FOR_ATOMS_OF_MOL(a, m_molecule) {
          if (a->IsHydrogen())
            hydrogenList.append(static_cast<Atom *>(&*a));
        }
        foreach(Atom *a, hydrogenList) {
          a->HtoMethyl();
        }
      }
      else { // user selected some atoms, only operate on those

        foreach(Primitive *a, m_SelectedList) {
          if (a->type() == Primitive::AtomType) {
            Atom *atom = static_cast<Atom *>(a);
            atom->HtoMethyl();
          }
        }
      } // end adding to selected atoms
      m_molecule->update();
    }

    void H2MethylCommand::undo()
    {
      *m_molecule = m_moleculeCopy;
      m_molecule->update();
    }

    bool H2MethylCommand::mergeWith ( const QUndoCommand * command )
    {
      // we received another call of the same action
      return true;
    }

    int H2MethylCommand::id() const
    {
      return 4706531;
    }

  } // end namespace Avogadro

#include "h2methylextension.moc"
Q_EXPORT_PLUGIN2(hydrogensextension, Avogadro::H2MethylExtensionFactory)
