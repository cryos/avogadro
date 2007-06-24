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

#ifndef __EDITCOMMANDS_H
#define __EDITCOMMANDS_H

#include <QUndoCommand>
#include <QStatusBar>
#include <QMimeData>

#include <avogadro/primitive.h>
#include <avogadro/glwidget.h>

namespace Avogadro {

  class CutCommand : public QUndoCommand
  {
  public:
    CutCommand(Molecule *molecule, QMimeData *copyData, 
               QList<Primitive*> selectedList);
    
    virtual void undo();
    virtual void redo();
    
  private:
    Molecule *m_molecule;         //!< parent (active molecule in widget)
    Molecule  m_originalMolecule; //!< original (unmodified molecule)
    QMimeData *m_copiedData;      //!< fragment to be copied to the clipboard
    QMimeData *m_savedData;       //!< original clipboard contents
    QList<Primitive*> m_selectedList; //!< any selected atoms
  };

  class CopyCommand : public QUndoCommand
  {
  public:
    CopyCommand(QMimeData *copyData);
    
    virtual void undo();
    virtual void redo();
    
  private:
    QMimeData *m_copiedMolecule;  //!< fragment to be copied to the clipboard
    QMimeData *m_savedData;       //!< original clipboard contents
  };

  class PasteCommand : public QUndoCommand
  {
  public:
    PasteCommand(Molecule *molecule, Molecule pasteData, GLWidget *widget);
    
    virtual void undo();
    virtual void redo();
    
  private:
    Molecule *m_molecule;
    Molecule m_pastedMolecule;   //!< pasted fragment from the clipboard
    Molecule m_originalMolecule;
    GLWidget *m_widget;
  };

 class ClearCommand : public QUndoCommand
  {
  public:
    ClearCommand(Molecule *molecule, QList<Primitive*> selectedList);
    
    virtual void undo();
    virtual void redo();
    
  private:
    Molecule *m_molecule;             //!< active widget molecule
    Molecule  m_originalMolecule;     //!< save original molecule
    QList<Primitive*> m_selectedList; //!< any selected atoms
  };

}

#endif
