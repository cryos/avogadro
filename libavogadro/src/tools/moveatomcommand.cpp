/**********************************************************************
  MoveAtomCommand - Command class for moving atoms or fragments

  Copyright (C) 2007 by Marcus D. Hanwell
  Copyright (C) 2007,2011 by Geoffrey R. Hutchison
  Copyright (C) 2007 by Benoit Jacob

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

#include "moveatomcommand.h"

namespace Avogadro {

  MoveAtomCommand::MoveAtomCommand(Molecule *molecule, QUndoCommand *parent) : QUndoCommand(parent), m_molecule(0)
  {
    // Store the molecule - this call won't actually move an atom
    setText(QObject::tr("Manipulate Atom"));
    m_moleculeCopy = *molecule;
    m_molecule = molecule;
    undone = false;
  }

  MoveAtomCommand::MoveAtomCommand(Molecule *molecule, int type, QUndoCommand *parent) : QUndoCommand(parent), m_molecule(0)
  {
    // Store the original molecule before any modifications are made
    setText(QObject::tr("Manipulate Atom"));
    m_moleculeCopy = *molecule;
    m_molecule = molecule;
    m_type =type;
    undone = false;
  }

  void MoveAtomCommand::redo()
  {
    // Move the specified atom to the location given
    if (undone)
    {
      Molecule newMolecule = *m_molecule;
      *m_molecule = m_moleculeCopy;
      m_moleculeCopy = newMolecule;
    }
    QUndoCommand::redo();
  }

  void MoveAtomCommand::undo()
  {
    // Restore our original molecule
    Molecule newMolecule = *m_molecule;
    *m_molecule = m_moleculeCopy;
    m_moleculeCopy = newMolecule;
    undone = true;
  }

  bool MoveAtomCommand::mergeWith (const QUndoCommand *)
  {
    // Just return true to repeated calls - we have stored the original molecule
    return true;
  }

  int MoveAtomCommand::id() const
  {
    return 26011980;
  }

} // end namespace Avogadro

