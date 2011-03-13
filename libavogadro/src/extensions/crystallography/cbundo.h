/**********************************************************************
  CBUndoState - Undo state infomation
  CBUndoCommand - Undo command for crystal builder
  CBAddCellUndoCommand
  CBRemoveCellUndoCommand

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

#ifndef CBUNDO_H
#define CBUNDO_H

#include "crystallographyextension.h"

#include <avogadro/molecule.h>

#include <openbabel/generic.h>

#include <Eigen/Core>

#include <QtGui/QUndoCommand>

#include <QtCore/QStringList>

namespace Avogadro
{
  class CBUndoState
  {
  public:
    CBUndoState(CrystallographyExtension *ext);
    virtual ~CBUndoState();
    void apply();
  private:
    CrystallographyExtension *m_ext;
    QStringList m_ids;
    QList<Eigen::Vector3d> m_coords;
    OpenBabel::OBUnitCell m_cell;
  };

  class CBUndoCommand : public QUndoCommand
  {
  public:
    CBUndoCommand(const CBUndoState &before,
                  const CBUndoState &after,
                  const QString &text);
    virtual ~CBUndoCommand();
    virtual void undo();
    virtual void redo();
  private:
    CBUndoState m_before, m_after;
    bool m_hasBeenUndone;
  };

  // Trivial: Cell addition: (note that this deletes the new cell and
  // passes mol a pointer to a new copy)
  class CBAddCellUndoCommand : public QUndoCommand
  {
  public:
    CBAddCellUndoCommand(Molecule *mol,
                         OpenBabel::OBUnitCell *cell,
                         CrystallographyExtension *ext)
      : m_ext(ext), m_mol(mol), m_cell(*cell) {delete cell;}
    void redo() {m_mol->setOBUnitCell(new OpenBabel::OBUnitCell(m_cell));
      m_ext->setMolecule(m_mol);}
    void undo() {m_mol->setOBUnitCell(0);
      m_ext->setMolecule(m_mol);}
  private:
    CrystallographyExtension *m_ext;
    Avogadro::Molecule *m_mol;
    OpenBabel::OBUnitCell m_cell;
  };

  // Trivial: Cell deletion:
  class CBRemoveCellUndoCommand : public QUndoCommand
  {
  public:
  CBRemoveCellUndoCommand(Molecule *mol,
                          CrystallographyExtension *ext)
    : m_ext(ext), m_mol(mol), m_cell(*mol->OBUnitCell()) {}
    void redo() {m_mol->setOBUnitCell(0);
      m_ext->setMolecule(m_mol);}
    void undo() {m_mol->setOBUnitCell(new OpenBabel::OBUnitCell(m_cell));
      m_ext->setMolecule(m_mol);}
  private:
    CrystallographyExtension *m_ext;
    Avogadro::Molecule *m_mol;
    OpenBabel::OBUnitCell m_cell;
  };

}

#endif
