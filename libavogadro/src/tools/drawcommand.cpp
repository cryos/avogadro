/**********************************************************************
  DrawCommand - Set of command classes for drawing.

  Copyright (C) 2007 Donald Ephraim Curtis
  Copyright (C) 2008 Tim Vandermeersch
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
    qDebug() << "AdjustHydrogensPreCommand()";
    d->molecule = molecule;
    d->atomIds = atomIds;

    foreach (unsigned long id, atomIds) {
      Atom *atom = molecule->atomById(id);
      if (atom) {
        if (atom->isHydrogen())
          continue;

        foreach (unsigned long nbrId, atom->neighbors()) {
          Atom *nbr = molecule->atomById(nbrId);
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
    qDebug() << "AdjustHydrogensPreCommand::undo()";

    // invert the list too produce exactly the same ids
    QList<unsigned long> reverseList;
    for (int i = d->atomIds.size() - 1; i >= 0; --i)
      reverseList.append(d->atomIds.at(i));

    foreach (unsigned long id, reverseList) {
      Atom *atom = d->molecule->atomById(id);

      if (atom) {
        if (atom->isHydrogen())
          continue;

        d->molecule->addHydrogens(atom, d->hydrogenIds.value(atom->id()), d->bondIds.value(atom->id()));
      }
    }

  }

  void AdjustHydrogensPreCommand::redo()
  {
    qDebug() << "AdjustHydrogensPreCommand::redo()";

    foreach (unsigned long id, d->atomIds) {
      Atom *atom = d->molecule->atomById(id);

      if (atom) {
        if (atom->isHydrogen())
          continue;

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
    qDebug() << "AdjustHydrogensPostCommand()";
    d->molecule = molecule;
    d->atomIds = atomIds;
  }

  AdjustHydrogensPostCommand::~AdjustHydrogensPostCommand()
  {
    delete d;
  }

  void AdjustHydrogensPostCommand::undo()
  {
    qDebug() << "AdjustHydrogensPostCommand::undo()";
 
    foreach (unsigned long id, d->atomIds) {
      Atom *atom = d->molecule->atomById(id);

      if (atom) {
        if (atom->isHydrogen())
          continue;

        d->molecule->removeHydrogens(atom);
      }
    }

  }

  void AdjustHydrogensPostCommand::redo()
  {
    qDebug() << "AdjustHydrogensPostCommand::redo()";

    if (d->hydrogenIds.isEmpty()) {
      foreach (unsigned long id, d->atomIds) {
        Atom *atom = d->molecule->atomById(id);
 
        if (atom) {
          if (atom->isHydrogen())
            continue;

          d->molecule->addHydrogens(atom); // new ids...
          // save the new ids for reuse
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
    } else {    
      // reuse ids from before
      foreach (unsigned long id, d->atomIds) {
        Atom *atom = d->molecule->atomById(id);
        if (atom) { 
          if (atom->isHydrogen())
            continue;
        
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
      AddAtomDrawCommandPrivate() : molecule(0), atom(0), id(-1), prevId(false), postCommand(0) {};

      Molecule *molecule;
      Atom *atom;
      Eigen::Vector3d pos;
      unsigned int element;
      // if adjustValence == 1 then add on redo and undo
      // if adjustValence == 2 then only adjust on undo (remove)
      int adjustValence;
      unsigned long id;
      bool prevId;
      
      QUndoCommand *postCommand;
  };

  AddAtomDrawCommand::AddAtomDrawCommand(Molecule *molecule, const Eigen::Vector3d& pos, unsigned int element, 
      int adjustValence) : d(new AddAtomDrawCommandPrivate)
  {
    qDebug() << "AddAtomDrawCommand(element=" << element << ", adj=" << adjustValence << ")"; 
    setText(QObject::tr("Add Atom"));
    d->molecule = molecule;
    d->pos = pos;
    d->element = element;
    d->adjustValence = adjustValence;
  }

  AddAtomDrawCommand::AddAtomDrawCommand(Molecule *molecule, Atom *atom, int adjustValence) : d(new AddAtomDrawCommandPrivate)
  {
    qDebug() << "AddAtomDrawCommand(element=" << atom->atomicNumber() << ", adj=" << adjustValence << ")"; 
    setText(QObject::tr("Add Atom"));
    d->molecule = molecule;
    d->pos = *atom->pos();
    d->element = atom->atomicNumber();
    d->atom = atom;
    d->adjustValence = adjustValence;
    d->id = atom->id();
  }

  AddAtomDrawCommand::~AddAtomDrawCommand()
  {
    delete d->postCommand;
    d->postCommand = 0;
    delete d;
  }

  void AddAtomDrawCommand::undo()
  {
    qDebug() << "AddAtomDrawCommand::undo()";
    Atom *atom = d->molecule->atomById(d->id);
    if (atom) {
      // Remove the previously add hydrogens if needed
      if (d->adjustValence)
        d->postCommand->undo();

      d->molecule->removeAtom(atom);
    }
  }

  void AddAtomDrawCommand::redo()
  {
    qDebug() << "AddAtomDrawCommand::redo()";

    if (d->atom) { // initial creation
      if (d->adjustValence) {
        QList<unsigned long> ids;
        ids.append(d->id);
        d->postCommand = new AdjustHydrogensPostCommand(d->molecule, ids);
        d->postCommand->redo();
      }
      d->atom = 0;
      return;
    }

    Atom *atom = 0;
    if (d->id != static_cast<unsigned long>(-1)) {
      atom = d->molecule->addAtom(d->id);
    } else {
      atom = d->molecule->addAtom();
      d->id = atom->id();
    }
    atom->setPos(d->pos);
    atom->setAtomicNumber(d->element);

    if (d->adjustValence) {
      if (!d->postCommand) {
        QList<unsigned long> ids;
        ids.append(d->id);
        d->postCommand = new AdjustHydrogensPostCommand(d->molecule, ids);
      }
      d->postCommand->redo();
    }

    atom->update();
  }

  /////////////////////////////////////////////////////////////////////////////
  // Delete Atom
  /////////////////////////////////////////////////////////////////////////////

  class DeleteAtomDrawCommandPrivate {
    public:
      DeleteAtomDrawCommandPrivate() : id(-1) {};

      Molecule *molecule;
      Molecule moleculeCopy;
      unsigned long id;
      int adjustValence;
  };

  DeleteAtomDrawCommand::DeleteAtomDrawCommand(Molecule *molecule, int index, int adjustValence) : d(new DeleteAtomDrawCommandPrivate)
  {
    setText(QObject::tr("Delete Atom"));
    d->molecule = molecule;
    d->moleculeCopy = *molecule;
    d->id = molecule->atom(index)->id();
    qDebug() << "DeleteAtomDrawCommand(id=" << d->id << ", adj=" << adjustValence << ")"; 
    d->adjustValence = adjustValence;
  }

  DeleteAtomDrawCommand::~DeleteAtomDrawCommand()
  {
    delete d;
  }

  void DeleteAtomDrawCommand::undo()
  {
    qDebug() << "DeleteAtomDrawCommand::undo()";
    *d->molecule = d->moleculeCopy;
    d->molecule->update();
  }

  void DeleteAtomDrawCommand::redo()
  {
    qDebug() << "DeleteAtomDrawCommand::redo()";
    Atom *atom = d->molecule->atomById(d->id);
    if(atom)
    {
      QList<Atom*> neighbors;

      if (d->adjustValence) {
        // Delete any hydrogens on this atom
        d->molecule->removeHydrogens(atom);
        // Now that we've deleted any attached hydrogens,
        // Adjust the valence on any bonded atom
        foreach (unsigned long id, atom->neighbors()) {
          Atom *nbr = d->molecule->atomById(id);
          d->molecule->removeHydrogens(d->molecule->atomById(id));
          neighbors.append(nbr);
        }
      }

      // Delete the atom, also deletes the bonds
      d->molecule->removeAtom(atom);

      if (d->adjustValence) {
        // Finally, add back hydrogens to neighbors
        foreach (Atom *nbr, neighbors)
          d->molecule->addHydrogens(nbr);
      }
      d->molecule->update();
    }
  }

  /////////////////////////////////////////////////////////////////////////////
  // Add Bond
  /////////////////////////////////////////////////////////////////////////////

  class AddBondDrawCommandPrivate {
    public:
      AddBondDrawCommandPrivate() : molecule(0), bond(0), id(-1), beginAtomId(-1), endAtomId(-1), prevId(false) {};

      Molecule *molecule;
      Bond *bond;
      unsigned long id;
      unsigned int beginAtomId;
      unsigned int endAtomId;
      bool prevId;
      Eigen::Vector3d pos;
      unsigned int order;
      int adjustValence;
  };

  AddBondDrawCommand::AddBondDrawCommand(Molecule *molecule, Atom *beginAtom, Atom *endAtom, unsigned int order, int adjustValence) : d(new AddBondDrawCommandPrivate)
  {
    qDebug() << "AddBondDrawCommand(begin=" << beginAtom->id() << ", end=" << endAtom->id() 
             << ", adj=" << adjustValence << ")"; 
    setText(QObject::tr("Add Bond"));
    d->molecule = molecule;
    d->beginAtomId = beginAtom->id();
    d->endAtomId = endAtom->id();
    d->order = order;
    d->adjustValence = adjustValence;
  }

  AddBondDrawCommand::AddBondDrawCommand(Molecule *molecule, Bond *bond, int adjustValence) : d(new AddBondDrawCommandPrivate)
  {
    qDebug() << "AddBondDrawCommand(begin=" << bond->beginAtomId() << ", end=" << bond->endAtomId() 
             << ", adj=" << adjustValence << ")"; 
 
    setText(QObject::tr("Add Bond"));
    d->molecule = molecule;
    d->beginAtomId = bond->beginAtomId();
    d->endAtomId = bond->endAtomId();
    d->order = bond->order();
    d->bond = bond;
    d->prevId = true;
    d->id = bond->id();
    d->adjustValence = adjustValence;
  }

  AddBondDrawCommand::~AddBondDrawCommand()
  {
    delete d;
  }

  void AddBondDrawCommand::undo()
  {
    qDebug() << "AddBondDrawCommand::undo()";
    Bond *bond = d->molecule->bondById(d->id);
    if(bond)
    {
      Atom* beginAtom = d->molecule->atomById(bond->beginAtomId());
      Atom* endAtom = d->molecule->atomById(bond->endAtomId());

      d->molecule->removeBond(bond);
      if (d->adjustValence) {
        if (!beginAtom->isHydrogen()) {
          d->molecule->removeHydrogens(beginAtom);
        }
        if (!endAtom->isHydrogen()) {
          d->molecule->removeHydrogens(endAtom);
        }

        if (!beginAtom->isHydrogen()) {
          d->molecule->addHydrogens(beginAtom);
        }
        if (!endAtom->isHydrogen()) {
          d->molecule->addHydrogens(endAtom);
        }
      }
      d->molecule->update();
      return;
    }
  }

  void AddBondDrawCommand::redo()
  {
    qDebug() << "AddBondDrawCommand::redo()";
    if(d->bond) { // already created the bond
      Atom* beginAtom = d->molecule->atomById(d->bond->beginAtomId());
      Atom* endAtom = d->molecule->atomById(d->bond->endAtomId());
      if (d->adjustValence) {
        if (!beginAtom->isHydrogen() && !endAtom->isHydrogen()) {
          d->molecule->removeHydrogens(beginAtom);
          d->molecule->removeHydrogens(endAtom);

          d->molecule->addHydrogens(beginAtom);
          d->molecule->addHydrogens(endAtom);
        }

      }
      d->bond = 0;
      return;
    }

    Atom *beginAtom = d->molecule->atomById(d->beginAtomId);
    Atom *endAtom = d->molecule->atomById(d->endAtomId);

    if(!beginAtom || !endAtom)
    {
      return;
    }

    Bond *bond;
    if(d->prevId)
    {
      bond = d->molecule->addBond(d->id);
    }
    else
    {
      bond = d->molecule->addBond();
      d->id = bond->id();
      d->prevId = true;
    }
    bond->setOrder(d->order);
    bond->setBegin(beginAtom);
    bond->setEnd(endAtom);
    if (d->adjustValence) {
      if (!beginAtom->isHydrogen() && !endAtom->isHydrogen())
      {
        d->molecule->removeHydrogens(beginAtom);
        d->molecule->removeHydrogens(endAtom);

        d->molecule->addHydrogens(endAtom);
        d->molecule->addHydrogens(beginAtom);
      }
    }
    d->molecule->update();
  }

  /////////////////////////////////////////////////////////////////////////////
  // Delete Bond
  /////////////////////////////////////////////////////////////////////////////

  class DeleteBondDrawCommandPrivate {
    public:
      DeleteBondDrawCommandPrivate() : id(-1) {};

      Molecule *molecule;
      Molecule moleculeCopy;
      unsigned long id;
      int adjustValence;
  };

  DeleteBondDrawCommand::DeleteBondDrawCommand(Molecule *molecule, int index, int adjustValence) : d(new DeleteBondDrawCommandPrivate)
  {
    setText(QObject::tr("Delete Bond"));
    d->molecule = molecule;
    d->moleculeCopy = (*(molecule));
    d->id = molecule->bond(index)->id();
    d->adjustValence = adjustValence;
  }

  DeleteBondDrawCommand::~DeleteBondDrawCommand()
  {
    delete d;
  }

  void DeleteBondDrawCommand::undo()
  {
    qDebug() << "DeleteBondDrawCommand::undo()";
    *d->molecule = d->moleculeCopy;
    d->molecule->update();
  }

  void DeleteBondDrawCommand::redo()
  {
    qDebug() << "DeleteBondDrawCommand::redo()";
    Bond *bond = d->molecule->bondById(d->id);
    if(bond)
    {
      d->molecule->removeBond(bond);
      if (d->adjustValence) {
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
      ChangeElementDrawCommandPrivate() : molecule(0), id(-1), preCommand(0), postCommand(0) {};

      Molecule *molecule;
      unsigned int newElement, oldElement;
      unsigned long id;
      int adjustValence;
      
      QUndoCommand *preCommand;
      QUndoCommand *postCommand;
  };

  ChangeElementDrawCommand::ChangeElementDrawCommand(Molecule *molecule, Atom *atom, unsigned int oldElement, 
      int adjustValence) : d(new ChangeElementDrawCommandPrivate)
  {
    qDebug() << "ChangeElementDrawCommand(id=" << atom->id() << ", old=" << oldElement << ", adj=" << adjustValence << ")";  
    setText(QObject::tr("Change Element"));
    d->molecule = molecule;
    d->newElement = atom->atomicNumber();
    d->oldElement = oldElement;
    d->id = atom->id();
    d->adjustValence = adjustValence;
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
    qDebug() << "ChangeElementDrawCommand::undo()";
    Atom *atom = d->molecule->atomById(d->id);

    if (atom) {
      // Remove Hydrogens if needed
      if (d->adjustValence)
        d->postCommand->undo();

      // Make sure we call BeginModify / EndModify (e.g., PR#1720879)
      atom->setAtomicNumber(d->oldElement);
 
      // Remove Hydrogens if needed
      if (d->adjustValence)
        d->preCommand->undo();
      
      d->molecule->update();
    }
  }

  void ChangeElementDrawCommand::redo()
  {
    qDebug() << "ChangeElementDrawCommand::redo()";
    Atom *atom = d->molecule->atomById(d->id);

    if (atom) {
      // Remove Hydrogens if needed
      if (d->adjustValence) {
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
      if (d->adjustValence) {
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
      ChangeBondOrderDrawCommandPrivate() : molecule(0), id(-1), preCommand(0), postCommand(0) {};

      Molecule *molecule;
      unsigned long id;
      unsigned int addBondOrder, oldBondOrder;
      int adjustValence;

      QUndoCommand *preCommand;
      QUndoCommand *postCommand;
  };

  ChangeBondOrderDrawCommand::ChangeBondOrderDrawCommand(Molecule *molecule, Bond *bond, 
      unsigned int oldBondOrder, int adjustValence) : d(new ChangeBondOrderDrawCommandPrivate)
  {
    qDebug() << "ChangeBondOrderDrawCommand(id=" << bond->id() << ", old=" << oldBondOrder 
             << ", adj=" << adjustValence << ")";  
    setText(QObject::tr("Change Bond Order"));
    d->molecule = molecule;
    d->id = bond->id();
    d->addBondOrder = bond->order();
    d->oldBondOrder = oldBondOrder;
    d->adjustValence = adjustValence;
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
    qDebug() << "ChangeBondOrderDrawCommand::undo()";
    Bond *bond = d->molecule->bondById(d->id);
    
    if (bond) {
      // Remove Hydrogens if needed
      if (d->adjustValence)
        d->postCommand->undo();

      // Make sure we call BeginModify / EndModify (e.g., PR#1720879)
      bond->setOrder(d->oldBondOrder);
 
      // Add Hydrogens if needed
      if (d->adjustValence)
        d->preCommand->undo();
      
      d->molecule->update();
    }
  }

  void ChangeBondOrderDrawCommand::redo()
  {
    qDebug() << "ChangeBondOrderDrawCommand::redo()";
    Bond *bond = d->molecule->bondById(d->id);
    if (bond) {      
      // Remove Hydrogens if needed
      if (d->adjustValence) {
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
      if (d->adjustValence) {
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

  InsertFragmentCommand::InsertFragmentCommand(Molecule *molecule, Molecule &generatedMolecule) : d(new InsertFragmentCommandPrivate)
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
