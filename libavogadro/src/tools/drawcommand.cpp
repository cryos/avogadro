/**********************************************************************
  DrawCommand - Set of command classes for drawing.

  Copyright (C) 2007 Donald Ephraim Curtis
  Copyright (C) 2008 Tim Vandermeersch
  Copyright (C) 2008 Geoffrey Hutchison

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

#include "drawcommand.h"
#include <avogadro/primitive.h>
#include <openbabel/obiter.h>

#include <QDebug>

using namespace OpenBabel;

namespace Avogadro {

  /// Utility function -- unset OpenBabel perception
  void UnsetFlags(Molecule *mol)
  {
    mol->UnsetFlag(OB_AROMATIC_MOL);
    mol->UnsetFlag(OB_SSSR_MOL);
    mol->UnsetFlag(OB_RINGFLAGS_MOL);
    mol->UnsetFlag(OB_ATOMTYPES_MOL);
    mol->UnsetFlag(OB_RINGTYPES_MOL);
    mol->UnsetFlag(OB_CHIRALITY_MOL);
    mol->UnsetFlag(OB_HYBRID_MOL);
    mol->UnsetFlag(OB_IMPVAL_MOL);
    mol->UnsetFlag(OB_KEKULE_MOL);
    mol->UnsetFlag(OB_CLOSURE_MOL);
    mol->UnsetFlag(OB_H_ADDED_MOL);
    mol->UnsetFlag(OB_AROM_CORRECTED_MOL);
  }

  /////////////////////////////////////////////////////////////////////////////
  // Add Atom
  /////////////////////////////////////////////////////////////////////////////

  class AddAtomDrawCommandPrivate {
    public:
      AddAtomDrawCommandPrivate() : molecule(0), atom(0), id(-1), prevId(false) {};

      Molecule *molecule;
      Atom *atom;
      Eigen::Vector3d pos;
      unsigned int element;
      // if adjustValence == 1 then add on redo and undo
      // if adjustValence == 2 then only adjust on undo (remove)
      int adjustValence;
      unsigned long id;
      bool prevId;
  };

  AddAtomDrawCommand::AddAtomDrawCommand(Molecule *molecule, const Eigen::Vector3d& pos, unsigned int element, int adjustValence) : d(new AddAtomDrawCommandPrivate)
  {
    setText(QObject::tr("Add Atom"));
    d->molecule = molecule;
    d->pos = pos;
    d->element = element;
    d->adjustValence = adjustValence;
    d->prevId = false;
  }

  AddAtomDrawCommand::AddAtomDrawCommand(Molecule *molecule, Atom *atom, int adjustValence) : d(new AddAtomDrawCommandPrivate)
  {
    setText(QObject::tr("Add Atom"));
    d->molecule = molecule;
    d->pos = atom->pos();
    d->element = atom->GetAtomicNum();
    d->atom = atom;
    d->adjustValence = adjustValence;
    d->id = atom->id();
    d->prevId = true;
  }

  AddAtomDrawCommand::~AddAtomDrawCommand()
  {
    delete d;
  }

  void AddAtomDrawCommand::undo()
  {
    Atom *atom = d->molecule->getAtomById(d->id);
    if(atom)
    {
      d->molecule->BeginModify();
      if (d->adjustValence) {
      qDebug() << "Adjusting Atom valence";
        if (!atom->IsHydrogen())
        {
          d->molecule->DeleteHydrogens(atom);
        }
      }
      d->molecule->DeleteAtom(atom);
      d->molecule->EndModify();
      //      d->molecule->update();
    }
  }

  void AddAtomDrawCommand::redo()
  {
    if(d->atom) { // initial creation
      if (d->adjustValence==1) {
      qDebug() << "Adjusting Atom valence";
        if (!d->atom->IsHydrogen()) {
          d->molecule->DeleteHydrogens(d->atom);
          d->molecule->AddHydrogens(d->atom);
        }
      }
      d->atom = 0;
      return;
    }

    Atom *atom = 0;
    d->molecule->BeginModify();
    if(d->prevId)
    {
      atom = d->molecule->newAtom(d->id);
    }
    else
    {
      atom = d->molecule->newAtom();
      d->id = atom->id();
      d->prevId = true;
    }
    atom->setPos(d->pos);
    atom->SetAtomicNum(d->element);
    d->molecule->EndModify();
    if (d->adjustValence==1) {
      qDebug() << "Adjusting Atom valence";
      if (!atom->IsHydrogen()) {
        d->molecule->AddHydrogens(atom);
      }
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
    d->moleculeCopy = (*(molecule));
    d->id = static_cast<Atom *>(molecule->GetAtom(index))->id();
    d->adjustValence = adjustValence;
  }

  DeleteAtomDrawCommand::~DeleteAtomDrawCommand()
  {
    delete d;
  }

  void DeleteAtomDrawCommand::undo()
  {
    *d->molecule = d->moleculeCopy;
    d->molecule->update();
  }

  void DeleteAtomDrawCommand::redo()
  {
    Atom *atom = d->molecule->getAtomById(d->id);
    if(atom)
    {
      QList<OBAtom*> neighbors;

      if (d->adjustValence) {
        // Delete any hydrogens on this atom
        d->molecule->DeleteHydrogens(atom);
        // Now that we've deleted any attached hydrogens,
        // Adjust the valence on any bonded atom
        FOR_NBORS_OF_ATOM(n, atom) {
          neighbors.append(&*n);
          d->molecule->DeleteHydrogens(&*n);
        }
      }
      d->molecule->DeleteAtom(atom);

      if (d->adjustValence) {
        // Finally, add back hydrogens to neighbors
        foreach (OBAtom *n, neighbors)
          d->molecule->AddHydrogens(n);
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
    setText(QObject::tr("Add Bond"));
    d->molecule = molecule;
    d->beginAtomId = beginAtom->id();
    d->endAtomId = endAtom->id();
    d->order = order;
    d->adjustValence = adjustValence;
  }

  AddBondDrawCommand::AddBondDrawCommand(Molecule *molecule, Bond *bond, int adjustValence) : d(new AddBondDrawCommandPrivate)
  {
    setText(QObject::tr("Add Bond"));
    d->molecule = molecule;
    d->beginAtomId = static_cast<Atom*>(bond->GetBeginAtom())->id();
    d->endAtomId = static_cast<Atom*>(bond->GetEndAtom())->id();
    d->order = bond->GetBondOrder();
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
    Bond *bond = d->molecule->getBondById(d->id);
    if(bond)
    {
      Atom *beginAtom = static_cast<Atom*>(bond->GetBeginAtom());
      Atom *endAtom = static_cast<Atom*>(bond->GetEndAtom());

      d->molecule->BeginModify();
      d->molecule->DeleteBond(bond);
      d->molecule->EndModify();
      if (d->adjustValence) {
        if (!beginAtom->IsHydrogen()) {
          d->molecule->DeleteHydrogens(beginAtom);
        }
        if (!endAtom->IsHydrogen()) {
          d->molecule->DeleteHydrogens(endAtom);
        }

        UnsetFlags(d->molecule);

        if (!beginAtom->IsHydrogen()) {
          d->molecule->AddHydrogens(beginAtom);
        }
        if (!endAtom->IsHydrogen()) {
          d->molecule->AddHydrogens(endAtom);
        }
      }
      d->molecule->update();
      return;
    }
  }

  void AddBondDrawCommand::redo()
  {

    if(d->bond) { // already created the bond
      Atom *beginAtom = static_cast<Atom*>(d->bond->GetBeginAtom());
      Atom *endAtom = static_cast<Atom*>(d->bond->GetEndAtom());
      if (d->adjustValence) {
        if (!beginAtom->IsHydrogen()) {
          d->molecule->DeleteHydrogens(beginAtom);
        }
        if (!endAtom->IsHydrogen()) {
          d->molecule->DeleteHydrogens(endAtom);
        }

        UnsetFlags(d->molecule);

        if (!beginAtom->IsHydrogen()) {
          d->molecule->AddHydrogens(beginAtom);
        }
        if (!endAtom->IsHydrogen()) {
          d->molecule->AddHydrogens(endAtom);
        }
      }
      d->bond = 0;
      return;
    }

    Atom *beginAtom = d->molecule->getAtomById(d->beginAtomId);
    Atom *endAtom = d->molecule->getAtomById(d->endAtomId);

    if(!beginAtom || !endAtom)
    {
      return;
    }

    d->molecule->BeginModify();
    Bond *bond;
    if(d->prevId)
    {
      bond = d->molecule->newBond(d->id);
    }
    else
    {
      bond = d->molecule->newBond();
      d->id = bond->id();
      d->prevId = true;
    }
    bond->SetBondOrder(d->order);
    bond->SetBegin(beginAtom);
    bond->SetEnd(endAtom);
    beginAtom->AddBond(bond);
    endAtom->AddBond(bond);
    d->molecule->EndModify();
    if (d->adjustValence) {
      if (!beginAtom->IsHydrogen())
      {
        d->molecule->DeleteHydrogens(beginAtom);
      }
      if (!endAtom->IsHydrogen())
      {
        d->molecule->DeleteHydrogens(endAtom);
      }

      UnsetFlags(d->molecule);

      if (!beginAtom->IsHydrogen())
      {
        d->molecule->AddHydrogens(endAtom);
      }
      if (!endAtom->IsHydrogen())
      {
        d->molecule->AddHydrogens(beginAtom);
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
    d->id = static_cast<Bond *>(molecule->GetBond(index))->id();
    d->adjustValence = adjustValence;
  }

  DeleteBondDrawCommand::~DeleteBondDrawCommand()
  {
    delete d;
  }

  void DeleteBondDrawCommand::undo()
  {
    *d->molecule = d->moleculeCopy;
    d->molecule->update();
  }

  void DeleteBondDrawCommand::redo()
  {
    Bond *bond = d->molecule->getBondById(d->id);
    if(bond)
    {
      d->molecule->DeleteBond(bond);
      if (d->adjustValence) {
        OBAtom *a1, *a2;
        a1 = bond->GetBeginAtom();
        a2 = bond->GetEndAtom();
        d->molecule->DeleteHydrogens(a1);
        d->molecule->DeleteHydrogens(a2);

        UnsetFlags(d->molecule);

        d->molecule->AddHydrogens(a1);
        d->molecule->AddHydrogens(a2);
      }
      d->molecule->update();
    }
  }

  /////////////////////////////////////////////////////////////////////////////
  // Change Element
  /////////////////////////////////////////////////////////////////////////////

  class ChangeElementDrawCommandPrivate {
    public:
      ChangeElementDrawCommandPrivate() : molecule(0), id(0) {};

      Molecule *molecule;
      unsigned int newElement, oldElement;
      unsigned long id;
      int adjustValence;
  };

  ChangeElementDrawCommand::ChangeElementDrawCommand(Molecule *molecule, Atom *atom, unsigned int oldElement, int adjustValence) : d(new ChangeElementDrawCommandPrivate)
  {
    setText(QObject::tr("Change Element"));
    d->molecule = molecule;
    d->newElement = atom->GetAtomicNum();
    d->oldElement = oldElement;
    d->id = atom->id();
    d->adjustValence = adjustValence;
  }

  ChangeElementDrawCommand::~ChangeElementDrawCommand()
  {
    delete d;
  }

  void ChangeElementDrawCommand::undo()
  {
    OBAtom *atom = d->molecule->getAtomById(d->id);

    if(atom)
    {
      // Make sure we call BeginModify / EndModify (e.g., PR#1720879)
      d->molecule->BeginModify();
      atom->SetAtomicNum(d->oldElement);
      d->molecule->EndModify();
      d->molecule->update();
      if (d->adjustValence) {
        UnsetFlags(d->molecule);
        d->molecule->DeleteHydrogens(atom);
        d->molecule->AddHydrogens(atom);
      }
    }
  }

  void ChangeElementDrawCommand::redo()
  {
    OBAtom *atom = d->molecule->GetAtom(d->id);

    if(atom)
    {
      // Make sure we call BeginModify / EndModify (e.g., PR#1720879)
      d->molecule->BeginModify();
      atom->SetAtomicNum(d->newElement);
      d->molecule->EndModify();
      if (d->adjustValence) {
        UnsetFlags(d->molecule);
        d->molecule->DeleteHydrogens(atom);
        d->molecule->AddHydrogens(atom);
      }
      d->molecule->update();
    }
  }

  /////////////////////////////////////////////////////////////////////////////
  // Change Bond Order
  /////////////////////////////////////////////////////////////////////////////

  class ChangeBondOrderDrawCommandPrivate {
    public:
      ChangeBondOrderDrawCommandPrivate() : molecule(0), id(-1) {};

      Molecule *molecule;
      unsigned long id;
      unsigned int newBondOrder, oldBondOrder;
      int adjustValence;
  };

  ChangeBondOrderDrawCommand::ChangeBondOrderDrawCommand(Molecule *molecule, Bond *bond, unsigned int oldBondOrder, int adjustValence) : d(new ChangeBondOrderDrawCommandPrivate)
  {
    setText(QObject::tr("Change Bond Order"));
    d->molecule = molecule;
    d->id = bond->id();
    d->newBondOrder = bond->GetBondOrder();
    d->oldBondOrder = oldBondOrder;
    d->adjustValence = adjustValence;
  }

  ChangeBondOrderDrawCommand::~ChangeBondOrderDrawCommand()
  {
    delete d;
  }

  void ChangeBondOrderDrawCommand::undo()
  {
    Bond *bond = d->molecule->getBondById(d->id);
    if(bond)
    {
      // Make sure we call BeginModify / EndModify (e.g., PR#1720879)
      d->molecule->BeginModify();
      bond->SetBondOrder(d->oldBondOrder);
      d->molecule->EndModify();
      if (d->adjustValence) {
        OBAtom *a1, *a2;
        a1 = bond->GetBeginAtom();
        a2 = bond->GetEndAtom();
        d->molecule->DeleteHydrogens(a1);
        d->molecule->DeleteHydrogens(a2);

        UnsetFlags(d->molecule);
        d->molecule->AddHydrogens(a1);
        d->molecule->AddHydrogens(a2);
      }
      d->molecule->update();
    }
  }

  void ChangeBondOrderDrawCommand::redo()
  {
    Bond *bond = d->molecule->getBondById(d->id);
    if(bond)
    {
      // Make sure we call BeginModify / EndModify (e.g., PR#1720879)
      d->molecule->BeginModify();
      bond->SetBondOrder(d->newBondOrder);
      d->molecule->EndModify();
      if (d->adjustValence) {

        OBAtom *a1, *a2;
        a1 = bond->GetBeginAtom();
        a2 = bond->GetEndAtom();
        d->molecule->DeleteHydrogens(a1);
        d->molecule->DeleteHydrogens(a2);

        UnsetFlags(d->molecule);
        d->molecule->AddHydrogens(a1);
        d->molecule->AddHydrogens(a2);
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
