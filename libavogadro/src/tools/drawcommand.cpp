/**********************************************************************
  DrawCommand - Set of command classes for drawing.

  Copyright (C) 2007 Donald Ephraim Curtis
  Copyright (C) 2008 Tim Vandermeersch

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

namespace Avogadro {

  /////////////////////////////////////////////////////////////////////////////
  // Add Atom
  /////////////////////////////////////////////////////////////////////////////

  class AddAtomDrawCommandPrivate {
    public:
      AddAtomDrawCommandPrivate() : molecule(0), index(0) {};

      Molecule *molecule;
      Eigen::Vector3d pos;
      int index;
      unsigned int element;
  };

  AddAtomDrawCommand::AddAtomDrawCommand(Molecule *molecule, const Eigen::Vector3d& pos, unsigned int element) : d(new AddAtomDrawCommandPrivate)
  {
    setText(QObject::tr("Add Atom"));
    d->molecule = molecule;
    d->pos = pos;
    d->element = element;
  }

  AddAtomDrawCommand::AddAtomDrawCommand(Molecule *molecule, Atom *atom) : d(new AddAtomDrawCommandPrivate)
  {
    setText(QObject::tr("Add Atom"));
    d->molecule = molecule;
    d->pos = atom->pos();
    d->element = atom->GetAtomicNum();
    d->index = atom->GetIdx();
  }

  AddAtomDrawCommand::~AddAtomDrawCommand()
  {
    delete d;
  }

  void AddAtomDrawCommand::undo()
  {
    OpenBabel::OBAtom *atom = d->molecule->GetAtom(d->index);

    if(atom)
    {
      d->molecule->BeginModify();
      d->molecule->DeleteAtom(atom);
      d->molecule->EndModify();
      d->molecule->update();
      d->index = -1;
    }
  }

  void AddAtomDrawCommand::redo()
  {
    if(d->index >= 0) {
      return;
    }

    d->molecule->BeginModify();
    Atom *atom = static_cast<Atom*>(d->molecule->NewAtom());
    d->index = atom->GetIdx();
    atom->setPos(d->pos);
    atom->SetAtomicNum(d->element);
    d->molecule->EndModify();
    atom->update();
  }
  
  /////////////////////////////////////////////////////////////////////////////
  // Delete Atom
  /////////////////////////////////////////////////////////////////////////////

  class DeleteAtomDrawCommandPrivate {
    public:
      DeleteAtomDrawCommandPrivate() : index(-1) {};

      Molecule *molecule;
      Molecule moleculeCopy;
      int index;
  };

  DeleteAtomDrawCommand::DeleteAtomDrawCommand(Molecule *molecule, int index) : d(new DeleteAtomDrawCommandPrivate)
  {
    setText(QObject::tr("Delete Atom"));
    d->molecule = molecule;
    d->moleculeCopy = (*(molecule));
    d->index = index;
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
    OpenBabel::OBAtom *atom = d->molecule->GetAtom(d->index);
    if(atom)
    {
      d->molecule->DeleteAtom(atom);
      d->molecule->update();
    }
  }
  
  /////////////////////////////////////////////////////////////////////////////
  // Add Bond
  /////////////////////////////////////////////////////////////////////////////

  class AddBondDrawCommandPrivate {
    public:
      AddBondDrawCommandPrivate() : molecule(0), index(-1) {};

      Molecule *molecule;
      Eigen::Vector3d pos;
      int index;
      int beginAtomIndex;
      int endAtomIndex;
      unsigned int order;
  };

  AddBondDrawCommand::AddBondDrawCommand(Molecule *molecule, Atom *beginAtom, Atom *endAtom, unsigned int order) : d(new AddBondDrawCommandPrivate)
  {
    setText(QObject::tr("Add Bond"));
    d->molecule = molecule;
    d->beginAtomIndex = beginAtom->GetIdx();
    d->endAtomIndex = endAtom->GetIdx();
    d->order = order;
  }

  AddBondDrawCommand::AddBondDrawCommand(Molecule *molecule, Bond *bond) : d(new AddBondDrawCommandPrivate)
  {
    setText(QObject::tr("Add Bond"));
    d->molecule = molecule;
    d->beginAtomIndex = bond->GetBeginAtomIdx();
    d->endAtomIndex = bond->GetEndAtomIdx();
    d->order = bond->GetBondOrder();
    d->index = bond->GetIdx();
  }

  AddBondDrawCommand::~AddBondDrawCommand()
  {
    delete d;
  }

  void AddBondDrawCommand::undo()
  {
    OpenBabel::OBBond *bond = d->molecule->GetBond(d->index);

    if(bond)
    {
      d->molecule->BeginModify();
      d->molecule->DeleteBond(bond);
      d->molecule->EndModify();
      d->molecule->update();
      d->index = -1;
    }
  }

  void AddBondDrawCommand::redo()
  {
    if(d->index >= 0) {
      return;
    }

    OpenBabel::OBAtom *beginAtom = d->molecule->GetAtom(d->beginAtomIndex);
    OpenBabel::OBAtom *endAtom = d->molecule->GetAtom(d->endAtomIndex);
    if(!beginAtom || !endAtom) {
      return;
    }

    d->molecule->BeginModify();
    Bond *bond = static_cast<Bond *>(d->molecule->NewBond());
    d->index = bond->GetIdx();
    bond->SetBondOrder(d->order);
    bond->SetBegin(beginAtom);
    bond->SetEnd(endAtom);
    beginAtom->AddBond(bond);
    endAtom->AddBond(bond);
    d->molecule->EndModify();
    d->molecule->update();
  }
  
  /////////////////////////////////////////////////////////////////////////////
  // Delete Bond
  /////////////////////////////////////////////////////////////////////////////

  class DeleteBondDrawCommandPrivate {
    public:
      DeleteBondDrawCommandPrivate() : index(-1) {};

      Molecule *molecule;
      Molecule moleculeCopy;
      int index;
  };

  DeleteBondDrawCommand::DeleteBondDrawCommand(Molecule *molecule, int index) : d(new DeleteBondDrawCommandPrivate)
  {
    setText(QObject::tr("Delete Bond"));
    d->molecule = molecule;
    d->moleculeCopy = (*(molecule));
    d->index = index;
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
    OpenBabel::OBBond *bond = d->molecule->GetBond(d->index);
    if(bond)
    {
      d->molecule->DeleteBond(bond);
      d->molecule->update();
    }
  }
  
  /////////////////////////////////////////////////////////////////////////////
  // Change Element
  /////////////////////////////////////////////////////////////////////////////

  class ChangeElementDrawCommandPrivate {
    public:
      ChangeElementDrawCommandPrivate() : molecule(0), index(0) {};

      Molecule *molecule;
      unsigned int newElement, oldElement;
      int index;
  };

  ChangeElementDrawCommand::ChangeElementDrawCommand(Molecule *molecule, Atom *atom, unsigned int element) : d(new ChangeElementDrawCommandPrivate)
  {
    setText(QObject::tr("Change Element"));
    d->molecule = molecule;
    d->oldElement = atom->GetAtomicNum();
    d->newElement = element;
    d->index = atom->GetIdx();
  }

  ChangeElementDrawCommand::~ChangeElementDrawCommand()
  {
    delete d;
  }

  void ChangeElementDrawCommand::undo()
  {
    OpenBabel::OBAtom *atom = d->molecule->GetAtom(d->index);
    
    if(atom)
    {
      // Make sure we call BeginModify / EndModify (e.g., PR#1720879)
      d->molecule->BeginModify();
      atom->SetAtomicNum(d->oldElement);
      d->molecule->EndModify();
      d->molecule->update();
    }
  }

  void ChangeElementDrawCommand::redo()
  {
    OpenBabel::OBAtom *atom = d->molecule->GetAtom(d->index);
    
    if(atom)
    {
      // Make sure we call BeginModify / EndModify (e.g., PR#1720879)
      d->molecule->BeginModify();
      atom->SetAtomicNum(d->newElement);
      d->molecule->EndModify();
      d->molecule->update();
    }
  }
  
  /////////////////////////////////////////////////////////////////////////////
  // Change Bond Order
  /////////////////////////////////////////////////////////////////////////////

  class ChangeBondOrderDrawCommandPrivate {
    public:
      ChangeBondOrderDrawCommandPrivate() : molecule(0), index(0) {};

      Molecule *molecule;
      unsigned int newBondOrder, oldBondOrder;
      int index;
  };

  ChangeBondOrderDrawCommand::ChangeBondOrderDrawCommand(Molecule *molecule, Bond *bond, unsigned int bondOrder) : d(new ChangeBondOrderDrawCommandPrivate)
  {
    setText(QObject::tr("Change Bond Order"));
    d->molecule = molecule;
    d->oldBondOrder = bond->GetBondOrder();
    d->newBondOrder = bondOrder;
    d->index = bond->GetIdx();
  }

  ChangeBondOrderDrawCommand::~ChangeBondOrderDrawCommand()
  {
    delete d;
  }

  void ChangeBondOrderDrawCommand::undo()
  {
    OpenBabel::OBBond *bond = d->molecule->GetBond(d->index);
    
    if(bond)
    {
      // Make sure we call BeginModify / EndModify (e.g., PR#1720879)
      d->molecule->BeginModify();
      bond->SetBondOrder(d->oldBondOrder);
      d->molecule->EndModify();
      d->molecule->update();
    }
  }

  void ChangeBondOrderDrawCommand::redo()
  {
    OpenBabel::OBBond *bond = d->molecule->GetBond(d->index);
    
    if(bond)
    {
      // Make sure we call BeginModify / EndModify (e.g., PR#1720879)
      d->molecule->BeginModify();
      bond->SetBondOrder(d->newBondOrder);
      d->molecule->EndModify();
      d->molecule->update();
    }
  }
 


} // end namespace Avogadro
