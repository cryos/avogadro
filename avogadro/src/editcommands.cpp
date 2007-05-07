/**********************************************************************
  EditCommands - Commands for undo/redo support for cut, copy, paste, etc.

  Copyright (C) 2007 Geoffrey R. Hutchison

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

#include "editcommands.h"

#include <QApplication>
#include <QClipboard>

namespace Avogadro {

  CopyCommand::CopyCommand(QMimeData *copyData):
    m_copiedMolecule(copyData)
  {
    m_savedData = const_cast<QMimeData *>(QApplication::clipboard()->mimeData());
    setText(QObject::tr("Copy Molecule"));
  }

  void CopyCommand::redo()
  {
    QApplication::clipboard()->setMimeData(m_copiedMolecule);
  }

  void CopyCommand::undo()
  {
    QApplication::clipboard()->setMimeData(m_savedData);
  }

  PasteCommand::PasteCommand(Molecule *molecule, Molecule pastedMolecule) :
    m_molecule(molecule),
    m_pastedMolecule(pastedMolecule),
    m_originalMolecule(*molecule)
  {
    setText(QObject::tr("Paste Molecule"));
  }

  void PasteCommand::redo()
  {
    *m_molecule += m_pastedMolecule;
    m_molecule->update();
  }

  void PasteCommand::undo()
  {
    *m_molecule = m_originalMolecule;
    m_molecule->update();
  }

} // end namespace Avogadro
