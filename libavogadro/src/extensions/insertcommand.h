/**********************************************************************
  Insert Command - Undo command for inserting a new fragment

  Copyright (C) 2007 Donald Ephraim Curtis
  Copyright (C) 2008,2009 Tim Vandermeersch

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

#ifndef INSERTCOMMAND_H
#define INSERTCOMMAND_H

#include <QUndoCommand>

namespace Avogadro {

  class Molecule;
  class GLWidget;

  class InsertFragmentCommandPrivate;
  class InsertFragmentCommand : public QUndoCommand
  {
  public:
    InsertFragmentCommand(Molecule *molecule, 
                          const Molecule &generatedMolecule,
                          GLWidget *widget,
                          const QString commandName,
                          int startAtom = -1,
                          int endAtom = -1);
    ~InsertFragmentCommand();

    virtual void undo();
    virtual void redo();

  private:
    InsertFragmentCommandPrivate * const d;
  };


} // end namespace Avogadro

#endif
