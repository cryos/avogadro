/**********************************************************************
  DrawCommand - Set of command classes for drawing.

  Copyright (C) 2007 Donald Ephraim Curtis
  Copyright (C) 2008,2009 Tim Vandermeersch

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.openmolecules.net/>

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

#ifndef DRAWCOMMAND_H
#define DRAWCOMMAND_H

#include <QUndoCommand>
#include <Eigen/Core>

namespace Avogadro {

  class Molecule;
  class Atom;
  class Bond;

  struct AdjustHydrogens 
  {
    enum Option {
      NoOption = 0x0,
      RemoveOnRedo = 0x1,
      AddOnRedo = 0x2,
      RemoveOnUndo = 0x4,
      AddOnUndo = 0x8,
      
      Never = NoOption,
      Always = RemoveOnRedo | AddOnRedo | RemoveOnUndo | AddOnUndo,
      OnRedo = RemoveOnRedo | AddOnRedo,
      OnUndo = RemoveOnUndo | AddOnUndo  
    };
    // typedef QFlags<Option> Options;
    Q_DECLARE_FLAGS(Options, Option)
  };

  // declare global operator|() for AdjustHydrogens::Options
  Q_DECLARE_OPERATORS_FOR_FLAGS(AdjustHydrogens::Options)


  class AdjustHydrogensPreCommandPrivate;
  class AdjustHydrogensPreCommand : public QUndoCommand
  {
  public:
    explicit AdjustHydrogensPreCommand(Molecule *molecule, const QList<unsigned long> &atomIds = QList<unsigned long>());
    AdjustHydrogensPreCommand(Molecule *molecule, unsigned long atomId);
    ~AdjustHydrogensPreCommand();
 
    virtual void undo();
    virtual void redo();

  private:
    void constructor();
    AdjustHydrogensPreCommandPrivate * const d;
  };

  class AdjustHydrogensPostCommandPrivate;
  class AdjustHydrogensPostCommand : public QUndoCommand
  {
  public:
    explicit AdjustHydrogensPostCommand(Molecule *molecule, const QList<unsigned long> &atomIds = QList<unsigned long>());
    AdjustHydrogensPostCommand(Molecule *molecule, unsigned long atomId);
    ~AdjustHydrogensPostCommand();
 
    virtual void undo();
    virtual void redo();

  private:
    AdjustHydrogensPostCommandPrivate * const d;
  };

  class AddAtomDrawCommandPrivate;
  /**
   * @class AddAtomDrawCommand
   *
   * supported flags:
   * 
   * AdjustHydrogens::AddOnRedo
   *
   *   AdjustHydrogensPostCommand::redo() : Add hydrogens to the atom after 
   *   creating it and store the hydrogen atom and C-H bond ids for later calls.
   *
   * AdjustHydrogens::RemoveOnUndo
   *
   *   AdjustHydrogensPostCommand::undo() : Remove the hydrogens before deleting 
   *   the atom.
   */
  class AddAtomDrawCommand : public QUndoCommand
  {
  public:
    AddAtomDrawCommand(Molecule *molecule, const Eigen::Vector3d& pos, 
        unsigned int element, AdjustHydrogens::Options adjustHydrogens = AdjustHydrogens::Never);
    AddAtomDrawCommand(Molecule *molecule, Atom *atom, 
        AdjustHydrogens::Options adjustHydrogens = AdjustHydrogens::Never);
    ~AddAtomDrawCommand();

    virtual void undo();
    virtual void redo();

  private:
    AddAtomDrawCommandPrivate * const d;
  };

  class DeleteAtomDrawCommandPrivate;
  class DeleteAtomDrawCommand : public QUndoCommand
  {
  public:
    DeleteAtomDrawCommand(Molecule *molecule, int index, bool adjustValence);
    ~DeleteAtomDrawCommand();

    virtual void undo();
    virtual void redo();

  private:
    DeleteAtomDrawCommandPrivate * const d;
  };

  class AddBondDrawCommandPrivate;
  /**
   * @class AddBondDrawCommand
   *
   * supported flags:
   * 
   * AdjustHydrogens::RemoveOnRedo
   *
   *   AdjustHydrogensPreCommand::redo() : Remove the hydrogens before creating
   *   the bond.
   *
   * AdjustHydrogens::AddOnRedo
   *
   *   AdjustHydrogensPostCommand::redo() : Add hydrogens to the atom(s) after 
   *   creating the bond and store the hydrogen atom and C-H bond ids for later 
   *   calls.
   *
   * AdjustHydrogens::RemoveOnUndo
   *
   *   AdjustHydrogensPostCommand::undo() : Remove the hydrogens before 
   *   deleting the bond.
   *
   * AdjustHydrogens::AddOnUndo
   *
   *   AdjustHydrogensPreCommand::undo() : Add the hydrogens again to restore 
   *   the atoms to their original state.
   */
  class AddBondDrawCommand : public QUndoCommand
  {
  public:
    AddBondDrawCommand(Molecule *molecule, Atom *beginAtom, Atom *endAtom, unsigned int order,
        AdjustHydrogens::Options adjustHydrogensOnBeginAtom = AdjustHydrogens::Never,
        AdjustHydrogens::Options adjustHydrogensOnEndAtom = AdjustHydrogens::Never);
    AddBondDrawCommand(Molecule *molecule, Bond *bond,
        AdjustHydrogens::Options adjustHydrogensOnBeginAtom = AdjustHydrogens::Never,
        AdjustHydrogens::Options adjustHydrogensOnEndAtom = AdjustHydrogens::Never);
    ~AddBondDrawCommand();

    virtual void undo();
    virtual void redo();

  private:
    AddBondDrawCommandPrivate * const d;
  };

  class DeleteBondDrawCommandPrivate;
  class DeleteBondDrawCommand : public QUndoCommand
  {
  public:
    DeleteBondDrawCommand(Molecule *molecule, int index, bool adjustValence);
    ~DeleteBondDrawCommand();

    virtual void undo();
    virtual void redo();

  private:
    DeleteBondDrawCommandPrivate * const d;
  };

  class ChangeElementDrawCommandPrivate;
  class ChangeElementDrawCommand : public QUndoCommand
  {
  public:
    ChangeElementDrawCommand(Molecule *molecule, Atom *atom, 
                             unsigned int element, bool adjustValence);
    ~ChangeElementDrawCommand();

    void setAdjustHydrogens(bool adjustHydrogens);

    virtual void undo();
    virtual void redo();

  private:
    ChangeElementDrawCommandPrivate * const d;
  };

  class ChangeBondOrderDrawCommandPrivate;
  class ChangeBondOrderDrawCommand : public QUndoCommand
  {
  public:
    ChangeBondOrderDrawCommand(Molecule *molecule, Bond *bond,
                               unsigned int bondOrder, bool adjustValence);
    ~ChangeBondOrderDrawCommand();

    virtual void undo();
    virtual void redo();

  private:
    ChangeBondOrderDrawCommandPrivate * const d;
  };

  class InsertFragmentCommandPrivate;
  class InsertFragmentCommand : public QUndoCommand
  {
  public:
    InsertFragmentCommand(Molecule *molecule, Molecule &generatedMolecule);
    ~InsertFragmentCommand();

    virtual void undo();
    virtual void redo();

  private:
    InsertFragmentCommandPrivate * const d;
  };


} // end namespace Avogadro

#endif
