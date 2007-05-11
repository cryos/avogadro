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

  CutCommand::CutCommand(Molecule *molecule, QMimeData *copyData) :
    m_molecule(molecule), m_originalMolecule(*molecule),
    m_copiedData(copyData)
  {
    setText(QObject::tr("Cut"));
  }

  void CutCommand::redo()
  {
    QApplication::clipboard()->setMimeData(m_copiedData, QClipboard::Clipboard);
    QApplication::clipboard()->setMimeData(m_copiedData, QClipboard::Selection);
    m_molecule->Clear();
    m_molecule->update();
  }

  void CutCommand::undo()
  {
    // restore the clipboard and molecule
    QApplication::clipboard()->setMimeData(m_savedData, QClipboard::Clipboard);
    *m_molecule = m_originalMolecule;
    m_molecule->update();
  }

  CopyCommand::CopyCommand(QMimeData *copyData):
    m_copiedMolecule(copyData)
  {
    m_savedData = const_cast<QMimeData *>(QApplication::clipboard()->mimeData());
    setText(QObject::tr("Copy"));
  }

  void CopyCommand::redo()
  {
    QApplication::clipboard()->setMimeData(m_copiedMolecule, QClipboard::Clipboard);
    QApplication::clipboard()->setMimeData(m_copiedMolecule, QClipboard::Selection);
  }

  void CopyCommand::undo()
  {
    QApplication::clipboard()->setMimeData(m_savedData, QClipboard::Clipboard);
  }

  PasteCommand::PasteCommand(Molecule *molecule, Molecule pastedMolecule) :
    m_molecule(molecule),
    m_pastedMolecule(pastedMolecule),
    m_originalMolecule(*molecule)
  {
    setText(QObject::tr("Paste"));
  }

  void PasteCommand::redo()
  {
    // we should clear selection before pasting
    *m_molecule += m_pastedMolecule;
    m_molecule->update();
  }

  void PasteCommand::undo()
  {
    // we should restore the selection when we undo
    *m_molecule = m_originalMolecule;
    m_molecule->update();
  }

  ClearCommand::ClearCommand(Molecule *molecule):
    m_molecule(molecule),
    m_originalMolecule(*molecule)
  {
    setText(QObject::tr("Clear Molecule"));
  }

  void ClearCommand::redo()
  {
    m_molecule->Clear();
    m_molecule->update();
  }

  void ClearCommand::undo()
  {
    // we should restore the selection when we undo
    *m_molecule = m_originalMolecule;
    m_molecule->update();
  }

} // end namespace Avogadro
