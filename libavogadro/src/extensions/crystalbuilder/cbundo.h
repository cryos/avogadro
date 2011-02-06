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

#ifndef CBUNDO_H
#define CBUNDO_H

#include <openbabel/generic.h>

#include <Eigen/Core>

#include <QtGui/QUndoCommand>

#include <QtCore/QStringList>

namespace Avogadro
{
  class CrystalBuilderDialog;

  class CBUndoState
  {
  public:
    CBUndoState(CrystalBuilderDialog *dialog);
    virtual ~CBUndoState();
    void apply();
  private:
    CrystalBuilderDialog *m_dialog;
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

}

#endif
