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

using namespace OpenBabel;

namespace Avogadro {

  CutCommand::CutCommand(Molecule *molecule, QMimeData *copyData,
                         QList<Primitive*> selectedList) :
    m_molecule(molecule), m_originalMolecule(*molecule),
    m_copiedData(copyData), m_selectedList(selectedList)
  {
    if (selectedList.size() == 0)
      setText(QObject::tr("Cut Molecule"));
    else
      setText(QObject::tr("Cut Atoms"));
  }

  void CutCommand::redo()
  {
    QApplication::clipboard()->setMimeData(m_copiedData, QClipboard::Clipboard);
    if (m_selectedList.size() == 0) {
      m_molecule->Clear();
    }
    else {
      // Make sure any selection is an atom
      // FIXME: Do we need to do bonds or other primitives?
      foreach(Primitive* item, m_selectedList) {
        if (item->type() == Primitive::AtomType)
          m_molecule->DeleteAtom(static_cast<Atom*>(item));
      }
    }
    m_molecule->update();
  }

  void CutCommand::undo()
  {
    // restore the molecule
    *m_molecule = m_originalMolecule;
    m_molecule->update();
  }

  PasteCommand::PasteCommand(Molecule *molecule, Molecule pastedMolecule,
                             GLWidget *widget) :
    m_molecule(molecule),
    m_pastedMolecule(pastedMolecule),
    m_originalMolecule(*molecule),
    m_widget(widget)
  {
    setText(QObject::tr("Paste"));
  }

  void PasteCommand::redo()
  {
    m_widget->clearSelected();
    // save the current number of atoms -- we'll select all new ones
    unsigned int currentNumAtoms = m_molecule->NumAtoms();
    *m_molecule += m_pastedMolecule;

    QList<Primitive*> newSelection;
    FOR_ATOMS_OF_MOL(a, *m_molecule) {
      if (a->GetIdx() > currentNumAtoms)
        newSelection.append(static_cast<Atom *>(&*a));
    }
    m_widget->setSelected(newSelection, true);
    m_molecule->update();
  }

  void PasteCommand::undo()
  {
    // We can't easily save the previous selection, but it would be nice
    m_widget->clearSelected();
    *m_molecule = m_originalMolecule;
    m_molecule->update();
  }

  ClearCommand::ClearCommand(Molecule *molecule,
                             QList<Primitive*> selectedList):
    m_molecule(molecule),
    m_originalMolecule(*molecule),
    m_selectedList(selectedList)
  {
    if (selectedList.size() == 0)
      setText(QObject::tr("Clear Molecule"));
    else
      setText(QObject::tr("Clear Atoms"));
  }

  void ClearCommand::redo()
  {
    if (m_selectedList.size() == 0) {
      m_molecule->Clear();
    }
    else {
      // Make sure any selection is an atom
      // FIXME: Do we need to do bonds or other primitives?
      foreach(Primitive* item, m_selectedList) {
        if (item->type() == Primitive::AtomType)
          m_molecule->DeleteAtom(static_cast<Atom*>(item));
      }
    }
    m_molecule->update();
  }

  void ClearCommand::undo()
  {
    // we should restore the selectedPrimitives when we undo
    *m_molecule = m_originalMolecule;
    m_molecule->update();
  }

} // end namespace Avogadro
