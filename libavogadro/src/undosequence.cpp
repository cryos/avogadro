/**********************************************************************
  UndoSequence - Provides an sequence of Undo/Redo in a single command

  Copyright (C) 2007 Donald Ephraim Curtis

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.sourceforge.net/>

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

#include <config.h>
#include <avogadro/undosequence.h>

namespace Avogadro {

  class UndoSequencePrivate {
    public:
      UndoSequencePrivate()  {};

      QList<QUndoCommand *> commands;
  };

  UndoSequence::UndoSequence() : d(new UndoSequencePrivate)
  {
  }

  UndoSequence::~UndoSequence()
  {
    while(!d->commands.isEmpty()) {
      delete d->commands.takeFirst();
    }
    delete d;
  }

  void UndoSequence::undo()
  {
    // last in first to undo
    for(int i=d->commands.count()-1; i >= 0; i--)
    {
      d->commands.at(i)->undo();
    }
  }

  void UndoSequence::redo()
  {
    foreach(QUndoCommand *command, d->commands)
    {
      command->redo();
    }
  }

  void UndoSequence::append(QUndoCommand *command)
  {
    d->commands.append(command);
  }

} // end namespace Avogadro
