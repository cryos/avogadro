/**********************************************************************
  CBUndoState - Undo state infomation
  CBUndoCommand - Undo command for crystal builder

  Copyright (C) 2011 by David C. Lonie

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.openmolecules.net/>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation version 2 of the License.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
 ***********************************************************************/

#include "cbundo.h"

#include "crystalbuilderdialog.h"

namespace Avogadro
{

  CBUndoState::CBUndoState(CrystalBuilderDialog *dialog)
    : m_dialog(dialog),
      m_ids(dialog->currentAtomicSymbols()),
      m_coords(dialog->currentCartesianCoords()),
      m_cell(*dialog->currentCell())
  {
  }

  CBUndoState::~CBUndoState()
  {
  }

  void CBUndoState::apply()
  {
    m_dialog->setCurrentCell(new OpenBabel::OBUnitCell (m_cell));
    m_dialog->setCurrentCartesianCoords(m_ids, m_coords);
  }

  CBUndoCommand::CBUndoCommand(const CBUndoState &before,
                               const CBUndoState &after,
                               const QString &text)
    : m_before(before),
      m_after(after),
      m_hasBeenUndone(false)
  {
    setText(text);
  }

  CBUndoCommand::~CBUndoCommand()
  {
  }

  void CBUndoCommand::undo()
  {
    m_before.apply();
    m_hasBeenUndone = true;
  }

  void CBUndoCommand::redo()
  {
    if (!m_hasBeenUndone) {
      // Don't do anything until this has been undone
      return;
    }
    m_after.apply();
  }

} // end namespace Avogadro
