/**********************************************************************
  Insert Command - Undo class for inserting a molecular fragment

  Copyright (C) 2007 Donald Ephraim Curtis
  Copyright (C) 2008,2009 Tim Vandermeersch
  Copyright (C) 2008 Geoffrey Hutchison

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


namespace Avogadro {

  /////////////////////////////////////////////////////////////////////////////
  // Insert Fragment
  /////////////////////////////////////////////////////////////////////////////

  class InsertFragmentCommandPrivate {
    public:
      InsertFragmentCommandPrivate() : 
        molecule(0), 
        generatedMolecule(0), widget(0) {};
    
    Molecule *molecule;
    Molecule moleculeCopy, generatedMolecule;
    GLWidget *widget;
  };

  InsertFragmentCommand::InsertFragmentCommand(Molecule *molecule, 
                                               const Molecule &generatedMolecule,
                                               GLWidget *widget,
                                               const QString commandName)
      : d(new InsertFragmentCommandPrivate)
  {
    setText(commandName);
    d->molecule = molecule;
    d->moleculeCopy = *molecule;
    d->generatedMolecule = generatedMolecule;
    d->widget = widget;
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
    d->molecule->update();

    if (d->widget) {
      QList<Primitive *> matchedAtoms;

      if (emptyMol) // we'll miss atom 0, so add it now
        matchedAtoms.append(d->molecule->atom(0));

      foreach (Atom *atom, d->molecule->atoms()) {
        if (atom->index() > initialAtoms)
          matchedAtoms.append(const_cast<Atom *>(atom));
      }

      d->widget->clearSelected();
      d->widget->setSelected(matchedAtoms, true);
      d->widget->update();

      d->widget->toolGroup()->setActiveTool("Manipulate");
    }
  }

} // end namespace Avogadro
