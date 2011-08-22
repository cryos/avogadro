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

#ifndef MOVEATOMCOMMAND_H
#define MOVEATOMCOMMAND_H

#include <QUndoCommand>
#include <avogadro/molecule.h>

namespace Avogadro {

 class MoveAtomCommand : public QUndoCommand
  {
    public:
      explicit MoveAtomCommand(Molecule *molecule, QUndoCommand *parent = 0);
      MoveAtomCommand(Molecule *molecule, int type, QUndoCommand *parent = 0);

      void redo();
      void undo();
      bool mergeWith ( const QUndoCommand * command );
      int id() const;

    private:
      Molecule m_moleculeCopy;
      Molecule *m_molecule;
      int m_type;
      bool undone;
  };


} // end namespace Avogadro

#endif
