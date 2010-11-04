/**********************************************************************
  DrawCommand - Set of command classes for drawing.

  Copyright (C) 2007 Donald Ephraim Curtis
  Copyright (C) 2008,2009 Tim Vandermeersch
  Copyright (C) 2008 Geoffrey Hutchison

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

#include "drawcommand.h"
#include <avogadro/primitive.h>
#include <avogadro/atom.h>
#include <avogadro/bond.h>
#include <avogadro/molecule.h>

#include <QDebug>

// use this for creating drawcommand unit tests
//#define DEBUG_COMMANDS 1

using namespace OpenBabel;

namespace Avogadro {

  /////////////////////////////////////////////////////////////////////////////
  // Adjust Hydrogens PreCommand
  /////////////////////////////////////////////////////////////////////////////

  class AdjustHydrogensPreCommandPrivate {
    public:
      AdjustHydrogensPreCommandPrivate() : molecule(0) {};

      Molecule *molecule;
      QList<unsigned long> atomIds;
      QHash<unsigned long, QList<unsigned long> > hydrogenIds;
      QHash<unsigned long, QList<unsigned long> > bondIds;
  };

  AdjustHydrogensPreCommand::AdjustHydrogensPreCommand(Molecule *molecule, const QList<unsigned long> &atomIds)
      : d(new AdjustHydrogensPreCommandPrivate)
  {
#ifdef DEBUG_COMMANDS
    QString ids = "[ ";
    foreach (unsigned long id, atomIds)
      ids += QString::number(id) + " ";
    ids += "]";

    qDebug() << "AdjustHydrogensPreCommand(atomIds = " + ids + ")";
#endif
    d->molecule = molecule;
    d->atomIds = atomIds;
    constructor();
  }

  AdjustHydrogensPreCommand::AdjustHydrogensPreCommand(Molecule *molecule, unsigned long atomId)
      : d(new AdjustHydrogensPreCommandPrivate)
  {
#ifdef DEBUG_COMMANDS
    qDebug() << "AdjustHydrogensPreCommand(atomId = " << atomId << ")";
#endif
    d->molecule = molecule;
    d->atomIds.append(atomId);
    constructor();
  }

  void AdjustHydrogensPreCommand::constructor()
  {
    foreach (unsigned long id, d->atomIds) {
      Atom *atom = d->molecule->atomById(id);
      Q_CHECK_PTR( atom );
      if (atom) {
        if (atom->isHydrogen()) {
          qDebug() << "AdjustHydrogensPreCommand::constructor(): Error, request to add hydrogens on hydrogen atom";
          continue;
        }

        foreach (unsigned long nbrId, atom->neighbors()) {
          Atom *nbr = d->molecule->atomById(nbrId);
          if (nbr)
            if (nbr->isHydrogen()) {
              d->hydrogenIds[id].append(nbrId);
              d->bondIds[id].append(d->molecule->bond(id, nbrId)->id());
            }
        }
      }
    }
  }

  AdjustHydrogensPreCommand::~AdjustHydrogensPreCommand()
  {
    delete d;
  }

  void AdjustHydrogensPreCommand::undo()
  {
#ifdef DEBUG_COMMANDS
    qDebug() << "AdjustHydrogensPreCommand::undo()";
#endif

    foreach (unsigned long id, d->atomIds) {
      Atom *atom = d->molecule->atomById(id);
      Q_CHECK_PTR( atom );

      if (atom) {
        if (atom->isHydrogen()) {
          qDebug() << "AdjustHydrogensPreCommand::undo(): Error, request to add hydrogens on hydrogen atom";
          continue;
        }

        d->molecule->addHydrogens(atom, d->hydrogenIds.value(atom->id()), d->bondIds.value(atom->id()));
      }
    }

  }

  void AdjustHydrogensPreCommand::redo()
  {
#ifdef DEBUG_COMMANDS
    qDebug() << "AdjustHydrogensPreCommand::redo()";
#endif

    foreach (unsigned long id, d->atomIds) {
      Atom *atom = d->molecule->atomById(id);
      Q_CHECK_PTR( atom );

      if (atom) {
        if (atom->isHydrogen()) {
          qDebug() << "AdjustHydrogensPreCommand::redo(): Error, request to add hydrogens on hydrogen atom";
          continue;
        }

        d->molecule->removeHydrogens(atom);
      }
    }

  }

  /////////////////////////////////////////////////////////////////////////////
  // Adjust Hydrogens PostCommand
  /////////////////////////////////////////////////////////////////////////////

  class AdjustHydrogensPostCommandPrivate {
    public:
      AdjustHydrogensPostCommandPrivate() : molecule(0) {};

      Molecule *molecule;
      QList<unsigned long> atomIds;
      QHash<unsigned long, QList<unsigned long> > hydrogenIds;
      QHash<unsigned long, QList<unsigned long> > bondIds;
  };

  AdjustHydrogensPostCommand::AdjustHydrogensPostCommand(Molecule *molecule, const QList<unsigned long> &atomIds)
      : d(new AdjustHydrogensPostCommandPrivate)
  {
#ifdef DEBUG_COMMANDS
    qDebug() << "AdjustHydrogensPostCommand()";
#endif
    d->molecule = molecule;
    d->atomIds = atomIds;
  }

  AdjustHydrogensPostCommand::AdjustHydrogensPostCommand(Molecule *molecule, unsigned long atomId)
      : d(new AdjustHydrogensPostCommandPrivate)
  {
#ifdef DEBUG_COMMANDS
    qDebug() << "AdjustHydrogensPostCommand()";
#endif
    d->molecule = molecule;
    d->atomIds.append(atomId);
  }

  AdjustHydrogensPostCommand::~AdjustHydrogensPostCommand()
  {
    delete d;
  }

  void AdjustHydrogensPostCommand::undo()
  {
#ifdef DEBUG_COMMANDS
    qDebug() << "AdjustHydrogensPostCommand::undo()";
#endif

    foreach (unsigned long id, d->atomIds) {
      Atom *atom = d->molecule->atomById(id);
      Q_CHECK_PTR( atom );

      if (atom) {
        if (atom->isHydrogen()) {
          qDebug() << "AdjustHydrogensPostCommand::undo(): Error, request to add hydrogens on hydrogen atom";
          continue;
        }

        d->molecule->removeHydrogens(atom);
      }
    }

  }

  void AdjustHydrogensPostCommand::redo()
  {
#ifdef DEBUG_COMMANDS
    qDebug() << "AdjustHydrogensPostCommand::redo()";
#endif

    if (d->hydrogenIds.isEmpty()) {
      foreach (unsigned long id, d->atomIds) {
        Atom *atom = d->molecule->atomById(id);
        Q_CHECK_PTR( atom );

        if (atom) {
          if (atom->isHydrogen()) {
            qDebug() << "AdjustHydrogensPostCommand::redo(): Error, request to add hydrogens on hydrogen atom";
            continue;
          }

          d->molecule->addHydrogens(atom); // new ids...
          // save the new ids for reuse
          foreach (unsigned long nbrId, atom->neighbors()) {
            Atom *nbr = d->molecule->atomById(nbrId);
            if (nbr)
              if (nbr->isHydrogen()) {
                Bond *bond = d->molecule->bond(id, nbrId);
                if (!bond) {
#ifdef DEBUG_COMMANDS
                  qDebug() << "Error, AdjustHydrogensPostCommand::redo...";
#endif
                  continue;
                }
                d->hydrogenIds[id].append(nbrId);
                d->bondIds[id].append(bond->id());
              }
          }
        }
      }
    } else {
      // reuse ids from before
      foreach (unsigned long id, d->atomIds) {
        Atom *atom = d->molecule->atomById(id);
        Q_CHECK_PTR( atom );
        
        if (atom) {
          if (atom->isHydrogen()) {
            qDebug() << "AdjustHydrogensPostCommand::redo(): Error, request to add hydrogens on hydrogen atom";
            continue;
          }

          d->molecule->addHydrogens(atom, d->hydrogenIds.value(atom->id()), d->bondIds.value(atom->id()));
        }
      }

    }

  }

  /////////////////////////////////////////////////////////////////////////////
  // Add Atom
  /////////////////////////////////////////////////////////////////////////////

  class AddAtomDrawCommandPrivate {
    public:
      AddAtomDrawCommandPrivate() : molecule(0), atom(0), id(FALSE_ID),
      prevId(false), postCommand(0) {}

      Molecule *molecule;
      Atom *atom;
      Eigen::Vector3d pos;
      unsigned int element;
      unsigned long id;
      bool prevId;
      AdjustHydrogens::Options adjustHydrogens;

      QUndoCommand *postCommand;
  };

  AddAtomDrawCommand::AddAtomDrawCommand(Molecule *molecule, const Eigen::Vector3d& pos, unsigned int element,
      AdjustHydrogens::Options adjustHydrogens) : d(new AddAtomDrawCommandPrivate)
  {
#ifdef DEBUG_COMMANDS
    qDebug() << "AddAtomDrawCommand_ctor1(element = " << element << ", adj = " << adjustHydrogens << ")";
#endif
    setText(QObject::tr("Add Atom"));
    d->molecule = molecule;
    d->pos = pos;
    d->element = element;
    d->adjustHydrogens = adjustHydrogens;
  }

  AddAtomDrawCommand::AddAtomDrawCommand(Molecule *molecule, Atom *atom, AdjustHydrogens::Options adjustHydrogens)
      : d(new AddAtomDrawCommandPrivate)
  {
#ifdef DEBUG_COMMANDS
    qDebug() << "AddAtomDrawCommand_ctor2(element = " << atom->atomicNumber() << ", adj = " << adjustHydrogens << ")";
#endif
    setText(QObject::tr("Add Atom"));
    d->molecule = molecule;
    d->pos = *atom->pos();
    d->element = atom->atomicNumber();
    d->atom = atom;
    d->id = atom->id();
    d->adjustHydrogens = adjustHydrogens;
  }

  AddAtomDrawCommand::~AddAtomDrawCommand()
  {
    if (d->postCommand) {
      delete d->postCommand;
      d->postCommand = 0;
    }
    delete d;
  }

  void AddAtomDrawCommand::undo()
  {
#ifdef DEBUG_COMMANDS
    qDebug() << "AddAtomDrawCommand::undo(id = " << d->id << ")";
#endif

    Atom *atom = d->molecule->atomById(d->id);
    Q_CHECK_PTR( atom );
    if (atom) {
      // Remove the previously add hydrogens if needed
      if (d->adjustHydrogens & AdjustHydrogens::RemoveOnUndo)
        d->postCommand->undo();

      d->molecule->removeAtom(atom);
    }

    d->molecule->update();
  }

  void AddAtomDrawCommand::redo()
  {
    if (d->atom) { // initial creation
      if (d->adjustHydrogens != AdjustHydrogens::Never) {
        d->postCommand = new AdjustHydrogensPostCommand(d->molecule, d->id);
        if (d->adjustHydrogens & AdjustHydrogens::AddOnRedo)
          d->postCommand->redo();
      }
      d->atom->update();
      d->atom = 0;
#ifdef DEBUG_COMMANDS
      qDebug() << "AddAtomDrawCommand::redo(id = " << d->id << ")";
#endif
      return;
    }

    Atom *atom = 0;
    if (d->id != FALSE_ID) {
      atom = d->molecule->addAtom(d->id);
      Q_CHECK_PTR( atom );
    } else {
      atom = d->molecule->addAtom();
      Q_CHECK_PTR( atom );
      d->id = atom->id();
    }
    atom->setPos(d->pos);
    atom->setAtomicNumber(d->element);

    if (d->adjustHydrogens != AdjustHydrogens::Never) {
      if (!d->postCommand)
        d->postCommand = new AdjustHydrogensPostCommand(d->molecule, d->id);
      if (d->adjustHydrogens & AdjustHydrogens::AddOnRedo)
        d->postCommand->redo();
    }

#ifdef DEBUG_COMMANDS
    qDebug() << "AddAtomDrawCommand::redo(id = " << d->id << ")";
#endif

    atom->update();
  }

  /////////////////////////////////////////////////////////////////////////////
  // Delete Atom
  /////////////////////////////////////////////////////////////////////////////

  class DeleteAtomDrawCommandPrivate {
    public:
      DeleteAtomDrawCommandPrivate() : id(FALSE_ID), preCommand(0),
      postCommand(0) {}

      Molecule *molecule;
      unsigned long id;
      QList<unsigned long> bonds;
      QList<short> bondOrders;
      QList<unsigned long> neighbors;
      Eigen::Vector3d pos;
      unsigned int element;
      int adjustHydrogens;

      QUndoCommand *preCommand;
      QUndoCommand *postCommand;
  };

  DeleteAtomDrawCommand::DeleteAtomDrawCommand(Molecule *molecule, int index, int adjustHydrogens)
      : d(new DeleteAtomDrawCommandPrivate)
  {
    setText(QObject::tr("Delete Atom"));
    d->molecule = molecule;
    Atom *atom = molecule->atom(index);
    d->id = atom->id();
#ifdef DEBUG_COMMANDS
    qDebug() << "DeleteAtomDrawCommand(id = " << d->id << ", adj = " << adjustHydrogens << ")";
#endif
    d->element = atom->atomicNumber();
    d->pos = *(atom->pos());
    d->adjustHydrogens = adjustHydrogens;
  }

  DeleteAtomDrawCommand::~DeleteAtomDrawCommand()
  {
    if (d->preCommand) {
      delete d->preCommand;
      d->preCommand = 0;
    }
    if (d->postCommand) {
      delete d->postCommand;
      d->postCommand = 0;
    }
    delete d;
  }

  void DeleteAtomDrawCommand::undo()
  {
#ifdef DEBUG_COMMANDS
    qDebug() << "DeleteAtomDrawCommand::undo()";
#endif

    if (d->adjustHydrogens)
      d->postCommand->undo();

    Atom *atom = d->molecule->addAtom(d->id);
    Q_CHECK_PTR( atom );
    atom->setAtomicNumber(d->element);
    atom->setPos(d->pos);

    // Add the bonds again...
    foreach (unsigned long id, d->bonds) {
      int index = d->bonds.indexOf(id);
      Bond *bond = d->molecule->addBond(id);
      bond->setAtoms(d->id, d->neighbors.at(index), d->bondOrders.at(index));
    }

    if (d->adjustHydrogens)
      d->preCommand->undo();

    d->molecule->update();
  }

  void DeleteAtomDrawCommand::redo()
  {
#ifdef DEBUG_COMMANDS
    qDebug() << "DeleteAtomDrawCommand::redo()";
#endif
    Atom *atom = d->molecule->atomById(d->id);
    Q_CHECK_PTR( atom );

    d->bonds.clear();
    d->bondOrders.clear();
    d->neighbors.clear();
    foreach (unsigned long id, atom->neighbors()) {
      Atom *nbr = d->molecule->atomById(id);
      if (nbr->isHydrogen()) continue;
      Bond *bond = d->molecule->bond(id, atom->id());

      d->neighbors.append(id);
      d->bonds.append(bond->id());
      d->bondOrders.append(bond->order());
    }

    if (atom) {
      QList<unsigned long> neighbors;

      if (d->adjustHydrogens) {
        if (!d->preCommand) {
          QList<unsigned long> ids;
          foreach (unsigned long id, atom->neighbors()) {
            if (!d->molecule->atomById(id)->isHydrogen()) {
              neighbors.append(id);
              ids.append(id);
            }
          }

          ids.append(atom->id());
          // Delete all hydrogens on the to be deleted atom and its heavy atom neighbors
          // The hydrogen ids are saved along with thier bond ids
          d->preCommand = new AdjustHydrogensPreCommand(d->molecule, ids);
        }
        d->preCommand->redo();
      }

      // Delete the atom, also deletes the bonds
      d->molecule->removeAtom(atom);

      if (d->adjustHydrogens) {
        if (!d->postCommand)
          // Add hydrogens to the heavy atom neighbors of the delted atom
          // Uses new ids the first time, reuses these on successive calls
          d->postCommand = new AdjustHydrogensPostCommand(d->molecule, neighbors);
        d->postCommand->redo();
      }

      d->molecule->update();
    }
  }

  /////////////////////////////////////////////////////////////////////////////
  // Add Bond
  /////////////////////////////////////////////////////////////////////////////

  class AddBondDrawCommandPrivate {
    public:
      AddBondDrawCommandPrivate() : molecule(0), bond(0), id(FALSE_ID),
                                    beginAtomId(FALSE_ID), endAtomId(FALSE_ID),
                                    prevId(false), preCommandBegin(0),
                                    postCommandBegin(0), preCommandEnd(0),
                                    postCommandEnd(0) {}

      Molecule *molecule;
      Bond *bond;
      unsigned long id;
      unsigned int beginAtomId;
      unsigned int endAtomId;
      bool prevId;
      Eigen::Vector3d pos;
      unsigned int order;
      AdjustHydrogens::Options adjustHydrogensBegin;
      AdjustHydrogens::Options adjustHydrogensEnd;

      QUndoCommand *preCommandBegin;
      QUndoCommand *postCommandBegin;
      QUndoCommand *preCommandEnd;
      QUndoCommand *postCommandEnd;
  };

  AddBondDrawCommand::AddBondDrawCommand(Molecule *molecule, Atom *beginAtom, Atom *endAtom,
      unsigned int order, AdjustHydrogens::Options adjustHydrogensOnBeginAtom,
      AdjustHydrogens::Options adjustHydrogensOnEndAtom) : d(new AddBondDrawCommandPrivate)
  {
#ifdef DEBUG_COMMANDS
    qDebug() << "AddBondDrawCommand_ctor1(begin = " << beginAtom->id() << ", end = " << endAtom->id()
             << ", order = " << order << ", adjBegin = " << adjustHydrogensOnBeginAtom
             << ", adjEnd = " << adjustHydrogensOnEndAtom << ")";
#endif
    setText(QObject::tr("Add Bond"));
    d->molecule = molecule;
    d->beginAtomId = beginAtom->id();
    d->endAtomId = endAtom->id();
    d->order = order;
    d->adjustHydrogensBegin = adjustHydrogensOnBeginAtom;
    d->adjustHydrogensEnd = adjustHydrogensOnEndAtom;
  }

  AddBondDrawCommand::AddBondDrawCommand(Molecule *molecule, Bond *bond,
      AdjustHydrogens::Options adjustHydrogensOnBeginAtom,
      AdjustHydrogens::Options adjustHydrogensOnEndAtom) : d(new AddBondDrawCommandPrivate)
  {
#ifdef DEBUG_COMMANDS
    qDebug() << "AddBondDrawCommand_ctor2(begin = " << bond->beginAtomId() << ", end = " << bond->endAtomId()
             << ", order = " << bond->order() << ", adjBegin = " << adjustHydrogensOnBeginAtom
             << ", adjEnd = " << adjustHydrogensOnEndAtom << ")";
#endif

    setText(QObject::tr("Add Bond"));
    d->molecule = molecule;
    d->beginAtomId = bond->beginAtomId();
    d->endAtomId = bond->endAtomId();
    d->order = bond->order();
    d->bond = bond;
    d->prevId = true;
    d->id = bond->id();
    d->adjustHydrogensBegin = adjustHydrogensOnBeginAtom;
    d->adjustHydrogensEnd = adjustHydrogensOnEndAtom;
  }

  AddBondDrawCommand::~AddBondDrawCommand()
  {
    if (d->preCommandBegin) {
      delete d->preCommandBegin;
      d->preCommandBegin = 0;
    }
    if (d->postCommandBegin) {
      delete d->postCommandBegin;
      d->postCommandBegin = 0;
    }
    if (d->preCommandEnd) {
      delete d->preCommandEnd;
      d->preCommandEnd = 0;
    }
    if (d->postCommandEnd) {
      delete d->postCommandEnd;
      d->postCommandEnd = 0;
    }
    delete d;
  }

  void AddBondDrawCommand::undo()
  {
#ifdef DEBUG_COMMANDS
    qDebug() << "AddBondDrawCommand::undo()";
#endif
    Bond *bond = d->molecule->bondById(d->id);
    Q_CHECK_PTR( bond );
    if (bond) {
      // remove the hydrogens added after the bond was created
      if (d->adjustHydrogensBegin & AdjustHydrogens::RemoveOnUndo)
        d->postCommandBegin->undo();
      if (d->adjustHydrogensEnd & AdjustHydrogens::RemoveOnUndo)
        d->postCommandEnd->undo();

      // remove the bond
      d->molecule->removeBond(bond);

      // restore the hydrogens to the way they were before the bond was added
      if (d->adjustHydrogensBegin & AdjustHydrogens::AddOnUndo)
        d->preCommandBegin->undo();
      if (d->adjustHydrogensEnd & AdjustHydrogens::AddOnUndo)
        d->preCommandEnd->undo();

      d->molecule->update();
    }
  }

  void AddBondDrawCommand::redo()
  {
#ifdef DEBUG_COMMANDS
    qDebug() << "AddBondDrawCommand::redo()";
#endif
    if (d->bond) { // already created the bond

      // adjust hydrogens on begin atom if needed
      if (d->adjustHydrogensBegin != AdjustHydrogens::Never) {
        QList<unsigned long> ids;
        ids.append(d->bond->beginAtomId());

        d->preCommandBegin = new AdjustHydrogensPreCommand(d->molecule, d->bond->beginAtomId());
        // remove hydrogens from begin and/or end atom
        if (d->adjustHydrogensBegin & AdjustHydrogens::RemoveOnRedo)
          d->preCommandBegin->redo();

        d->postCommandBegin = new AdjustHydrogensPostCommand(d->molecule, d->bond->beginAtomId());
        // add hydrogens again
        if (d->adjustHydrogensBegin & AdjustHydrogens::AddOnRedo)
          d->postCommandBegin->redo();
      }

      // adjust hydrogens on end atom if needed
      if (d->adjustHydrogensEnd != AdjustHydrogens::Never) {
        d->preCommandEnd = new AdjustHydrogensPreCommand(d->molecule, d->bond->endAtomId());
        // remove hydrogens from begin and/or end atom
        if (d->adjustHydrogensEnd & AdjustHydrogens::RemoveOnRedo)
          d->preCommandEnd->redo();

        d->postCommandEnd = new AdjustHydrogensPostCommand(d->molecule, d->bond->endAtomId());
        // add hydrogens again
        if (d->adjustHydrogensEnd & AdjustHydrogens::AddOnRedo)
          d->postCommandEnd->redo();
      }

      d->bond = 0;
      return;
    }

    // adjust hydrogens on begin atom if needed
    if (d->adjustHydrogensBegin != AdjustHydrogens::Never) {
      if (!d->preCommandBegin)
        d->preCommandBegin = new AdjustHydrogensPreCommand(d->molecule, d->beginAtomId);
      // remove hydrogens from begin and/or end atom
      if (d->adjustHydrogensBegin & AdjustHydrogens::RemoveOnRedo)
        d->preCommandBegin->redo();
    }

    // adjust hydrogens on end atom if needed
    if (d->adjustHydrogensEnd != AdjustHydrogens::Never) {
      if (!d->preCommandEnd)
        d->preCommandEnd = new AdjustHydrogensPreCommand(d->molecule, d->endAtomId);
      // remove hydrogens from begin and/or end atom
      if (d->adjustHydrogensEnd & AdjustHydrogens::RemoveOnRedo)
        d->preCommandEnd->redo();
    }

    Atom *beginAtom = d->molecule->atomById(d->beginAtomId);
    Q_CHECK_PTR( beginAtom );
    Atom *endAtom = d->molecule->atomById(d->endAtomId);
    Q_CHECK_PTR( endAtom );

    if (!beginAtom || !endAtom)
      return;

    Bond *bond;
    if (d->id != FALSE_ID) {
      bond = d->molecule->addBond(d->id);
      Q_CHECK_PTR( bond );
    } else {
      bond = d->molecule->addBond();
      Q_CHECK_PTR( bond );
      d->id = bond->id();
    }

    bond->setOrder(d->order);
    bond->setBegin(beginAtom);
    bond->setEnd(endAtom);

    // adjust hydrogens on begin atom if needed
    if (d->adjustHydrogensBegin != AdjustHydrogens::Never) {
      if (!d->postCommandBegin)
        d->postCommandBegin = new AdjustHydrogensPostCommand(d->molecule, d->beginAtomId);
      // remove hydrogens from begin and/or end atom
      if (d->adjustHydrogensBegin & AdjustHydrogens::AddOnRedo)
        d->postCommandBegin->redo();
    }

    // adjust hydrogens on end atom if needed
    if (d->adjustHydrogensEnd != AdjustHydrogens::Never) {
      if (!d->postCommandEnd)
        d->postCommandEnd = new AdjustHydrogensPostCommand(d->molecule, d->endAtomId);
      // remove hydrogens from begin and/or end atom
      if (d->adjustHydrogensEnd & AdjustHydrogens::AddOnRedo)
        d->postCommandEnd->redo();
    }

    d->molecule->update();
  }

  /////////////////////////////////////////////////////////////////////////////
  // Delete Bond
  /////////////////////////////////////////////////////////////////////////////

  class DeleteBondDrawCommandPrivate {
    public:
      DeleteBondDrawCommandPrivate() : id(FALSE_ID) {}

      Molecule *molecule;
      Molecule moleculeCopy;
      unsigned long id;
      int adjustHydrogens;
  };

  DeleteBondDrawCommand::DeleteBondDrawCommand(Molecule *molecule, int index, int adjustHydrogens) : d(new DeleteBondDrawCommandPrivate)
  {
    setText(QObject::tr("Delete Bond"));
    d->molecule = molecule;
    d->moleculeCopy = (*(molecule));
    d->id = molecule->bond(index)->id();
#ifdef DEBUG_COMMANDS
    qDebug() << "DeleteBondDrawCommand(id = " << d->id << ", adj = " << adjustHydrogens << ")";
#endif

    d->adjustHydrogens = adjustHydrogens;
  }

  DeleteBondDrawCommand::~DeleteBondDrawCommand()
  {
    delete d;
  }

  void DeleteBondDrawCommand::undo()
  {
#ifdef DEBUG_COMMANDS
    qDebug() << "DeleteBondDrawCommand::undo()";
#endif
    *d->molecule = d->moleculeCopy;
    d->molecule->update();
  }

  void DeleteBondDrawCommand::redo()
  {
#ifdef DEBUG_COMMANDS
    qDebug() << "DeleteBondDrawCommand::redo()";
#endif
    Bond *bond = d->molecule->bondById(d->id);
    Q_CHECK_PTR( bond );
    if(bond)
    {
      d->molecule->removeBond(bond);
      if (d->adjustHydrogens) {
        Atom *a1, *a2;
        a1 = d->molecule->atomById(bond->beginAtomId());
        a2 = d->molecule->atomById(bond->endAtomId());

        d->molecule->removeHydrogens(a1);
        d->molecule->removeHydrogens(a2);

        d->molecule->addHydrogens(a1);
        d->molecule->addHydrogens(a2);
      }
      d->molecule->update();
    }
  }

  /////////////////////////////////////////////////////////////////////////////
  // Change Element
  /////////////////////////////////////////////////////////////////////////////

  class ChangeElementDrawCommandPrivate {
    public:
      ChangeElementDrawCommandPrivate() : molecule(0), id(FALSE_ID),
      preCommand(0), postCommand(0) {}

      Molecule *molecule;
      unsigned int newElement, oldElement;
      unsigned long id;
      int adjustHydrogens;

      QUndoCommand *preCommand;
      QUndoCommand *postCommand;
  };

  ChangeElementDrawCommand::ChangeElementDrawCommand(Molecule *molecule, Atom *atom, unsigned int oldElement,
      int adjustHydrogens) : d(new ChangeElementDrawCommandPrivate)
  {
#ifdef DEBUG_COMMANDS
    qDebug() << "ChangeElementDrawCommand(id = " << atom->id() << ", old = " << oldElement 
             << ", new = " << atom->atomicNumber() << ", adj=" << adjustHydrogens << ")";
#endif

    setText(QObject::tr("Change Element"));
    d->molecule = molecule;
    d->newElement = atom->atomicNumber();
    d->oldElement = oldElement;
    d->id = atom->id();
    d->adjustHydrogens = adjustHydrogens;
  }
    
  void ChangeElementDrawCommand::setAdjustHydrogens(int adjustHydrogens)
  {
    d->adjustHydrogens = adjustHydrogens;
  }

  ChangeElementDrawCommand::~ChangeElementDrawCommand()
  {
    if (d->preCommand) {
      delete d->preCommand;
      d->preCommand = 0;
    }
    if (d->postCommand) {
      delete d->postCommand;
      d->postCommand = 0;
    }
    delete d;
  }

  void ChangeElementDrawCommand::undo()
  {
#ifdef DEBUG_COMMANDS
    qDebug() << "ChangeElementDrawCommand::undo()";
#endif
    Atom *atom = d->molecule->atomById(d->id);
    Q_CHECK_PTR( atom );

    if (atom) {
      // Remove Hydrogens if needed
      if (d->adjustHydrogens)
        d->postCommand->undo();

      // Make sure we call BeginModify / EndModify (e.g., PR#1720879)
      atom->setAtomicNumber(d->oldElement);

      // Remove Hydrogens if needed
      if (d->adjustHydrogens)
        d->preCommand->undo();

      d->molecule->update();
    }
  }

  void ChangeElementDrawCommand::redo()
  {
#ifdef DEBUG_COMMANDS
    qDebug() << "ChangeElementDrawCommand::redo()";
#endif
    Atom *atom = d->molecule->atomById(d->id);
    Q_CHECK_PTR( atom );

    if (atom) {
      // Remove Hydrogens if needed
      if (d->adjustHydrogens) {
        if (!d->preCommand) {
          QList<unsigned long> ids;
          ids.append(d->id);
          d->preCommand = new AdjustHydrogensPreCommand(d->molecule, ids);
        }
        d->preCommand->redo();
      }

      // Make sure we call BeginModify / EndModify (e.g., PR#1720879)
      atom->setAtomicNumber(d->newElement);

      // Add hydrogens again if needed
      if (d->adjustHydrogens) {
        if (!d->postCommand) {
          QList<unsigned long> ids;
          ids.append(d->id);
          d->postCommand = new AdjustHydrogensPostCommand(d->molecule, ids);
        }
        d->postCommand->redo();
      }

      d->molecule->update();
    }
  }

  /////////////////////////////////////////////////////////////////////////////
  // Change Bond Order
  /////////////////////////////////////////////////////////////////////////////

  class ChangeBondOrderDrawCommandPrivate {
    public:
      ChangeBondOrderDrawCommandPrivate() : molecule(0), id(FALSE_ID),
      preCommand(0), postCommand(0) {}

      Molecule *molecule;
      unsigned long id;
      unsigned int addBondOrder, oldBondOrder;
      int adjustHydrogens;

      QUndoCommand *preCommand;
      QUndoCommand *postCommand;
  };

  ChangeBondOrderDrawCommand::ChangeBondOrderDrawCommand(Molecule *molecule, Bond *bond,
      unsigned int oldBondOrder, int adjustHydrogens) : d(new ChangeBondOrderDrawCommandPrivate)
  {
#ifdef DEBUG_COMMANDS
    qDebug() << "ChangeBondOrderDrawCommand(id = " << bond->id() << ", old = " << oldBondOrder
             << ", new = " << bond->order() << ", adj=" << adjustHydrogens << ")";
#endif
    setText(QObject::tr("Change Bond Order"));
    d->molecule = molecule;
    d->id = bond->id();
    d->addBondOrder = bond->order();
    d->oldBondOrder = oldBondOrder;
    d->adjustHydrogens = adjustHydrogens;
  }

  ChangeBondOrderDrawCommand::~ChangeBondOrderDrawCommand()
  {
    if (d->preCommand) {
      delete d->preCommand;
      d->preCommand = 0;
    }
    if (d->postCommand) {
      delete d->postCommand;
      d->postCommand = 0;
    }
    delete d;
  }

  void ChangeBondOrderDrawCommand::undo()
  {
#ifdef DEBUG_COMMANDS
    qDebug() << "ChangeBondOrderDrawCommand::undo()";
#endif
    Bond *bond = d->molecule->bondById(d->id);
    Q_CHECK_PTR( bond );

    if (bond) {
      // Remove Hydrogens if needed
      if (d->adjustHydrogens)
        d->postCommand->undo();

      // Make sure we call BeginModify / EndModify (e.g., PR#1720879)
      bond->setOrder(d->oldBondOrder);

      // Add Hydrogens if needed
      if (d->adjustHydrogens)
        d->preCommand->undo();

      d->molecule->update();
    }
  }

  void ChangeBondOrderDrawCommand::redo()
  {
#ifdef DEBUG_COMMANDS
    qDebug() << "ChangeBondOrderDrawCommand::redo()";
#endif
    Bond *bond = d->molecule->bondById(d->id);
    Q_CHECK_PTR( bond );
    if (bond) {
      // Remove Hydrogens if needed
      if (d->adjustHydrogens) {
        if (!d->preCommand) {
          QList<unsigned long> ids;
          ids.append(bond->beginAtomId());
          ids.append(bond->endAtomId());
          d->preCommand = new AdjustHydrogensPreCommand(d->molecule, ids);
        }
        d->preCommand->redo();
      }

      // Make sure we call BeginModify / EndModify (e.g., PR#1720879)
      bond->setOrder(d->addBondOrder);

      // Add Hydrogens if needed
      if (d->adjustHydrogens) {
        if (!d->postCommand) {
          QList<unsigned long> ids;
          ids.append(bond->beginAtomId());
          ids.append(bond->endAtomId());
          d->postCommand = new AdjustHydrogensPostCommand(d->molecule, ids);
        }
        d->postCommand->redo();
      }

      d->molecule->update();
    }
  }

  /////////////////////////////////////////////////////////////////////////////
  // Insert Fragment
  /////////////////////////////////////////////////////////////////////////////

  class InsertFragmentCommandPrivate {
    public:
      InsertFragmentCommandPrivate() : molecule(0), generatedMolecule(0) {};

      Molecule *molecule;
      Molecule moleculeCopy, generatedMolecule;
  };

  InsertFragmentCommand::InsertFragmentCommand(Molecule *molecule, Molecule &generatedMolecule)
      : d(new InsertFragmentCommandPrivate)
  {
    setText(QObject::tr("Insert Fragment"));
    d->molecule = molecule;
    d->moleculeCopy = *molecule;
    d->generatedMolecule = generatedMolecule;
  }

  InsertFragmentCommand::~InsertFragmentCommand()
  {
    delete d;
  }

  void InsertFragmentCommand::undo()
  {
    *(d->molecule) = d->moleculeCopy;
    d->molecule->update();
  }

  void InsertFragmentCommand::redo()
  {
    *(d->molecule) += d->generatedMolecule;
    d->molecule->update();
  }

} // end namespace Avogadro
