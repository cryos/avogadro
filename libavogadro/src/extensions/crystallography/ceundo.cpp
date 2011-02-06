/**********************************************************************
  CEUndoState - Undo state infomation
  CEUndoCommand - Undo command for crystal builder

  Copyright (C) 2011 by David C. Lonie

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.openmolecules.net/>

  This source code is released under the New BSD License, (the "License").

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
 ***********************************************************************/

#include "ceundo.h"

#include "crystallographyextension.h"

namespace Avogadro
{

  CEUndoState::CEUndoState(CrystallographyExtension *ext)
    : m_ext(ext),
      m_ids(ext->currentAtomicSymbols()),
      m_coords(ext->currentCartesianCoords()),
      m_cell(*ext->currentCell())
  {
  }

  CEUndoState::~CEUndoState()
  {
  }

  void CEUndoState::apply()
  {
    m_ext->setCurrentCell(new OpenBabel::OBUnitCell (m_cell));
    m_ext->setCurrentCartesianCoords(m_ids, m_coords);
  }

  CEUndoCommand::CEUndoCommand(const CEUndoState &before,
                               const CEUndoState &after,
                               const QString &text)
    : m_before(before),
      m_after(after),
      m_hasBeenUndone(false)
  {
    setText(text);
  }

  CEUndoCommand::~CEUndoCommand()
  {
  }

  void CEUndoCommand::undo()
  {
    m_before.apply();
    m_hasBeenUndone = true;
  }

  void CEUndoCommand::redo()
  {
    if (!m_hasBeenUndone) {
      // Don't do anything until this has been undone
      return;
    }
    m_after.apply();
  }

} // end namespace Avogadro
