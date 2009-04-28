/**********************************************************************
  H2Methyl - Hydrogen to Methyl plugin for Avogadro

  Copyright (C) 2006 by Donald Ephraim Curtis
  Copyright (C) 2006-2007 by Geoffrey R. Hutchison

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.openmolecules.net/>

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
#include <avogadro/glwidget.h>
#include <avogadro/molecule.h>
#include <avogadro/atom.h>
#include <avogadro/primitivelist.h>

#include <openbabel/mol.h>
#include <openbabel/obiter.h>

#include <QAction>

using namespace std;
using namespace OpenBabel;

namespace Avogadro {

  H2MethylExtension::H2MethylExtension(QObject *parent) : Extension(parent),
    m_molecule(0)
  {
    QAction *action = new QAction(this);
    action->setText(tr("Change H to Methyl"));
    m_actions.append(action);

    action = new QAction( this );
    action->setSeparator(true);
    m_actions.append( action );
  }

  H2MethylExtension::~H2MethylExtension()
  {
  }

  QList<QAction *> H2MethylExtension::actions() const
  {
    return m_actions;
  }

  QString H2MethylExtension::menuPath(QAction *) const
  {
    return tr("&Build");
  }

  void H2MethylExtension::setMolecule(Molecule *molecule)
  {
    m_molecule = molecule;
  }

  QUndoCommand* H2MethylExtension::performAction( QAction *, GLWidget *widget )
  {
    QUndoCommand *undo = 0;
    undo = new H2MethylCommand(m_molecule, widget);
    return undo;
  }

  H2MethylCommand::H2MethylCommand(Molecule *molecule, GLWidget *widget):
    m_molecule(molecule), m_moleculeCopy(new Molecule(*molecule)),
    m_SelectedList(widget->selectedPrimitives())
  {
    // save the selection from the current view widget
    // (i.e., only modify a few hydrogens)
    //      m_SelectedList = widget->selectedPrimitives;

    setText(QObject::tr("H to Methyl"));
  }

  H2MethylCommand::~H2MethylCommand()
  {
    delete m_moleculeCopy;
  }

  void H2MethylCommand::redo()
  {
    if (m_SelectedList.size() == 0) {
      QList<Atom*> hydrogenList, atoms;
      atoms = m_molecule->atoms();
      foreach(Atom *a, atoms) {
        if (a->isHydrogen()) {
          hydrogenList.append(a);
        }
      }
      foreach(Atom *a, hydrogenList) {
        a->setAtomicNumber(6);
        m_molecule->addHydrogens(a);
      }
    }
    else { // user selected some atoms, only operate on those

      foreach(Primitive *a, m_SelectedList.subList(Primitive::AtomType)) {
        Atom *atom = static_cast<Atom *>(a);
        atom->setAtomicNumber(6);
        m_molecule->addHydrogens(atom);
      }
    } // end adding to selected atoms
    m_molecule->update();
  }

  void H2MethylCommand::undo()
  {
    *m_molecule = m_moleculeCopy;
    m_molecule->update();
  }

  bool H2MethylCommand::mergeWith ( const QUndoCommand * )
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
Q_EXPORT_PLUGIN2(h2methylextension, Avogadro::H2MethylExtensionFactory)
