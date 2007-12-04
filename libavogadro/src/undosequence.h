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

#ifndef __UNDOSEQUENCE_H
#define __UNDOSEQUENCE_H

#include <QUndoCommand>

#include <avogadro/global.h>

namespace Avogadro {

  /**
   * @class UndoSequence
   * @brief Provides a sequence of Undo/Redo commands in a single command
   * @author Donald Ephraim Curtis
   *
   * This class simply acts as grouping of Undo/Redo commands.  As commands
   * are added, that is how they will be executed in the Redo stage.  In the
   * Undo stage they are executed in reverse order.  However, they are not
   * merged, they are left as they are but under a single command.
   */
  class UndoSequencePrivate;
  class A_EXPORT UndoSequence : public QUndoCommand
  {
    public:
      UndoSequence();
      ~UndoSequence();

      /**
       * @param command Command to add to the sequence
       */
      void append(QUndoCommand *command);

      virtual void undo();
      virtual void redo();

    private:
      UndoSequencePrivate * const d;
  };

} // end namespace Avogadro

#endif
