/**********************************************************************
  CEUndoState - Undo state infomation
  CEUndoCommand - Undo command for crystal builder
  CEAddCellUndoCommand
  CERemoveCellUndoCommand

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

#ifndef CEUNDO_H
#define CEUNDO_H

#include "crystallographyextension.h"

#include <avogadro/molecule.h>

#include <openbabel/generic.h>

#include <Eigen/Core>

#include <QtGui/QUndoCommand>

#include <QtCore/QStringList>

namespace Avogadro
{
  class CEUndoState
  {
  public:
    CEUndoState(CrystallographyExtension *ext);
    virtual ~CEUndoState();
    void apply();
  private:
    CrystallographyExtension *m_ext;
    QStringList m_ids;
    QList<Eigen::Vector3d> m_coords;
    OpenBabel::OBUnitCell m_cell;
  };

  class CEUndoCommand : public QUndoCommand
  {
  public:
    CEUndoCommand(const CEUndoState &before,
                  const CEUndoState &after,
                  const QString &text);
    virtual ~CEUndoCommand();
    virtual void undo();
    virtual void redo();
  private:
    CEUndoState m_before, m_after;
    bool m_hasBeenUndone;
  };

  // Trivial: Cell addition: (note that this deletes the new cell and
  // passes mol a pointer to a new copy)
  class CEAddCellUndoCommand : public QUndoCommand
  {
  public:
    CEAddCellUndoCommand(Molecule *mol,
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
  class CERemoveCellUndoCommand : public QUndoCommand
  {
  public:
  CERemoveCellUndoCommand(Molecule *mol,
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
