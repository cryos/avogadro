/**********************************************************************
  DrawCommand - Set of command classes for drawing.

  Copyright (C) 2007 Donald Ephraim Curtis

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

} // end namespace Avogadro
