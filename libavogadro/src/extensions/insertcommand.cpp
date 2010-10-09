/**********************************************************************
  Insert Command - Undo class for inserting a molecular fragment

  Copyright (C) 2007 Donald Ephraim Curtis
  Copyright (C) 2008,2009 Tim Vandermeersch
  Copyright (C) 2008-2010 Geoffrey Hutchison

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

#include "insertcommand.h"

#include <avogadro/molecule.h>
#include <avogadro/atom.h>
#include <avogadro/primitivelist.h>
#include <avogadro/glwidget.h>
#include <avogadro/toolgroup.h>

#include <openbabel/mol.h>
#include <openbabel/builder.h>

#include <QDebug>

namespace Avogadro {

  /////////////////////////////////////////////////////////////////////////////
  // Insert Fragment
  /////////////////////////////////////////////////////////////////////////////

  class InsertFragmentCommandPrivate {
    public:
      InsertFragmentCommandPrivate() : 
        molecule(0), 
        generatedMolecule(0), widget(0),
        startAtom(-1), endAtom(-1) {};
    
    Molecule *molecule;
    Molecule moleculeCopy, generatedMolecule;
    GLWidget *widget;
    int startAtom, endAtom; // if we're using OBBuilder::Connect()
  };

  InsertFragmentCommand::InsertFragmentCommand(Molecule *molecule, 
                                               const Molecule &generatedMolecule,
                                               GLWidget *widget,
                                               const QString commandName, int start, int end)
      : d(new InsertFragmentCommandPrivate)
  {
    setText(commandName);
    d->molecule = molecule;
    d->moleculeCopy = *molecule;
    d->generatedMolecule = generatedMolecule;
    d->widget = widget;
    d->startAtom = start;
    d->endAtom = end;
  }

  InsertFragmentCommand::~InsertFragmentCommand()
  {
    delete d;
  }

  void InsertFragmentCommand::undo()
  {
    *(d->molecule) = d->moleculeCopy;
    d->molecule->update();
  }

  void InsertFragmentCommand::redo()
  {
    unsigned int initialAtoms = d->molecule->numAtoms() - 1;
    bool emptyMol = (d->molecule->numAtoms() == 0);
    if (emptyMol)
      initialAtoms = 0;

    *(d->molecule) += d->generatedMolecule;

    // Do we need to connect the fragment to the original molecule?
    if (d->startAtom != -1 && !emptyMol) {
      // OK, first, we should see if this atom is a hydrogen
      Atom *startAtom = d->molecule->atomById(d->startAtom);
      if (startAtom->isHydrogen()) {
        // get the bonded non-hydrogen and remove this atom
        Atom *hydrogen = startAtom;
        if (hydrogen->neighbors().size())
          startAtom = d->molecule->atomById(hydrogen->neighbors()[0]); // the first bonded atom to this "H"
        d->molecule->removeAtom(hydrogen);
      } else { // heavy atom -- remove attached hydrogens
        d->molecule->removeHydrogens(startAtom);
      }

      if (d->endAtom == -1) { // connect to the first atom of the fragment
        d->endAtom = initialAtoms + 1;
      }
      Atom *endAtom = d->molecule->atomById(d->endAtom);
      d->molecule->removeHydrogens(endAtom); // make sure to adjust valence on this atom

      OpenBabel::OBMol mol = d->molecule->OBMol();
      // Open Babel indexes atoms from 1, not 0
      OpenBabel::OBBuilder::Connect(mol, startAtom->index() + 1, endAtom->index() + 1);
      d->molecule->setOBMol(&mol);
      d->molecule->addHydrogens();
    }

    // now tell the molecule to update
    d->molecule->update();

    if (d->widget && d->startAtom == -1) {
      QList<Primitive *> matchedAtoms;

      if (emptyMol) // we'll miss atom 0, so add it now
        matchedAtoms.append(d->molecule->atom(0));

      foreach (Atom *atom, d->molecule->atoms()) {
        if (atom->index() > initialAtoms)
          matchedAtoms.append(const_cast<Atom *>(atom));
      }

      d->widget->clearSelected();
      d->widget->setSelected(matchedAtoms, true);

      d->widget->toolGroup()->setActiveTool("Manipulate");
    }

    // in either case, update the widget
    if (d->widget)
      d->widget->update();
  }

} // end namespace Avogadro
