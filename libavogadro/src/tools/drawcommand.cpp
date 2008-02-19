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

  /////////////////////////////////////////////////////////////////////////////
  // Add Atom
  /////////////////////////////////////////////////////////////////////////////

  class AddAtomDrawCommandPrivate {
  public:
    AddAtomDrawCommandPrivate() : molecule(0), atom(0) {};

    Molecule *molecule;
    Atom *atom;
    Eigen::Vector3d pos;
    //int index;
    unsigned int element;
    int adjustValence;
  };

  AddAtomDrawCommand::AddAtomDrawCommand(Molecule *molecule, const Eigen::Vector3d& pos, unsigned int element, int adjustValence) : d(new AddAtomDrawCommandPrivate)
  {
    setText(QObject::tr("Add Atom"));
    d->molecule = molecule;
    d->pos = pos;
    d->element = element;
    d->adjustValence = adjustValence;
  }

  AddAtomDrawCommand::AddAtomDrawCommand(Molecule *molecule, Atom *atom, int adjustValence) : d(new AddAtomDrawCommandPrivate)
  {
    setText(QObject::tr("Add Atom"));
    d->molecule = molecule;
    d->pos = atom->pos();
    d->element = atom->GetAtomicNum();
    //d->index = atom->GetIdx();
    d->atom = atom;
    d->adjustValence = adjustValence;
  }

  AddAtomDrawCommand::~AddAtomDrawCommand()
  {
    delete d;
  }

  void AddAtomDrawCommand::undo()
  {
    //OBAtom *atom = d->molecule->GetAtom(d->index);

    if(d->atom)
      {
        d->molecule->BeginModify();
        if (d->adjustValence) {
          if (!d->atom->IsHydrogen())
            d->molecule->DeleteHydrogens(d->atom);
        }
        d->molecule->DeleteAtom(d->atom);
        d->molecule->EndModify();
        d->molecule->update();
        //d->index = -1;
	d->atom = 0;
      }
  }

  void AddAtomDrawCommand::redo()
  {
    if(d->atom) { // initial creation
      if (d->adjustValence) {
        if (!d->atom->IsHydrogen()) {
	  d->molecule->DeleteHydrogens(d->atom);
          d->molecule->AddHydrogens(d->atom);
          //d->index = atom->GetIdx();
	}
      }
      return;
    }

    d->molecule->BeginModify();
    d->atom = static_cast<Atom*>(d->molecule->NewAtom());
    d->atom->setPos(d->pos);
    d->atom->SetAtomicNum(d->element);
    d->molecule->EndModify();
    if (d->adjustValence) {
      if (!d->atom->IsHydrogen()) {
        d->molecule->AddHydrogens(d->atom);
      }
    }
    //d->index = atom->GetIdx();
    d->atom->update();
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
    int adjustValence;
  };

  DeleteAtomDrawCommand::DeleteAtomDrawCommand(Molecule *molecule, int index, int adjustValence) : d(new DeleteAtomDrawCommandPrivate)
  {
    setText(QObject::tr("Delete Atom"));
    d->molecule = molecule;
    d->moleculeCopy = (*(molecule));
    d->index = index;
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
    OBAtom *atom = d->molecule->GetAtom(d->index);
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
    AddBondDrawCommandPrivate() : molecule(0), index(-1) {};

    Molecule *molecule;
    Eigen::Vector3d pos;
    int index;
    int beginAtomIndex;
    int endAtomIndex;
    unsigned int order;
    int adjustValence;
  };

  AddBondDrawCommand::AddBondDrawCommand(Molecule *molecule, Atom *beginAtom, Atom *endAtom, unsigned int order, int adjustValence) : d(new AddBondDrawCommandPrivate)
  {
    setText(QObject::tr("Add Bond"));
    d->molecule = molecule;
    d->beginAtomIndex = beginAtom->GetIdx();
    d->endAtomIndex = endAtom->GetIdx();
    d->order = order;
    d->adjustValence = adjustValence;
  }

  AddBondDrawCommand::AddBondDrawCommand(Molecule *molecule, Bond *bond, int adjustValence) : d(new AddBondDrawCommandPrivate)
  {
    setText(QObject::tr("Add Bond"));
    d->molecule = molecule;
    d->beginAtomIndex = bond->GetBeginAtomIdx();
    d->endAtomIndex = bond->GetEndAtomIdx();
    d->order = bond->GetBondOrder();
    d->index = bond->GetIdx();
    d->adjustValence = adjustValence;
  }

  AddBondDrawCommand::~AddBondDrawCommand()
  {
    delete d;
  }

  void AddBondDrawCommand::undo()
  {
    OBBond *bond = d->molecule->GetBond(d->index);

    if(bond)
      {
        OBAtom *beginAtom = bond->GetBeginAtom();
        OBAtom *endAtom = bond->GetEndAtom();

        d->molecule->BeginModify();
	std::cout << "Num bonds = " << d->molecule->NumBonds() << std::endl;
        d->molecule->DeleteBond(bond);
	std::cout << "Num bonds = " << d->molecule->NumBonds() << std::endl;
        d->molecule->EndModify();
        if (d->adjustValence) {
          if (!beginAtom->IsHydrogen())
            d->molecule->DeleteHydrogens(beginAtom);
          if (!endAtom->IsHydrogen())
            d->molecule->DeleteHydrogens(endAtom);
          
          d->molecule->UnsetImplicitValencePerceived();
            
	  if (!beginAtom->IsHydrogen())
            d->molecule->AddHydrogens(beginAtom);
          if (!endAtom->IsHydrogen())
            d->molecule->AddHydrogens(endAtom);
        }

        d->molecule->update();
        d->index = -1;
      }
  }

  void AddBondDrawCommand::redo()
  {
    OBAtom *beginAtom = d->molecule->GetAtom(d->beginAtomIndex);
    OBAtom *endAtom = d->molecule->GetAtom(d->endAtomIndex);
    if(!beginAtom || !endAtom) {
      return;
    }

    if(d->index >= 0) { // already created the bond
      if (d->adjustValence) {
        OBBond *bnd = d->molecule->GetBond(d->index);
        
	if (!beginAtom->IsHydrogen())
          d->molecule->DeleteHydrogens(beginAtom);
        if (!endAtom->IsHydrogen())
          d->molecule->DeleteHydrogens(endAtom);
        
        d->molecule->UnsetImplicitValencePerceived();
        
        if (!beginAtom->IsHydrogen())
	  d->molecule->AddHydrogens(beginAtom);
        if (!endAtom->IsHydrogen())
          d->molecule->AddHydrogens(endAtom);
        
	d->index = bnd->GetIdx(); // Add/Delete Hydrogens could change the bond index
        d->beginAtomIndex = bnd->GetBeginAtomIdx();
        d->endAtomIndex = bnd->GetEndAtomIdx();
      }
      return;
    }

    d->molecule->BeginModify();
    Bond *bond = static_cast<Bond *>(d->molecule->NewBond());
    bond->SetBondOrder(d->order);
    bond->SetBegin(beginAtom);
    bond->SetEnd(endAtom);
    beginAtom->AddBond(bond);
    endAtom->AddBond(bond);
    d->molecule->EndModify();
    if (d->adjustValence) {
      if (!beginAtom->IsHydrogen())
        d->molecule->DeleteHydrogens(beginAtom);
      if (!endAtom->IsHydrogen())
        d->molecule->DeleteHydrogens(endAtom);
      
      d->molecule->UnsetImplicitValencePerceived();
      
      if (!beginAtom->IsHydrogen())
        d->molecule->AddHydrogens(endAtom);
      if (!endAtom->IsHydrogen())
        d->molecule->AddHydrogens(beginAtom);
    }
    d->index = bond->GetIdx();
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
    int adjustValence;
  };

  DeleteBondDrawCommand::DeleteBondDrawCommand(Molecule *molecule, int index, int adjustValence) : d(new DeleteBondDrawCommandPrivate)
  {
    setText(QObject::tr("Delete Bond"));
    d->molecule = molecule;
    d->moleculeCopy = (*(molecule));
    d->index = index;
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
    OBBond *bond = d->molecule->GetBond(d->index);
    if(bond)
      {
        d->molecule->DeleteBond(bond);
        if (d->adjustValence) {
          OBAtom *a1, *a2;
          a1 = bond->GetBeginAtom();
          a2 = bond->GetEndAtom();
          d->molecule->DeleteHydrogens(a1);
          d->molecule->DeleteHydrogens(a2);

          d->molecule->UnsetImplicitValencePerceived();
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
    ChangeElementDrawCommandPrivate() : molecule(0), index(0) {};

    Molecule *molecule;
    unsigned int newElement, oldElement;
    int index;
    int adjustValence;
  };

  ChangeElementDrawCommand::ChangeElementDrawCommand(Molecule *molecule, Atom *atom, unsigned int oldElement, int adjustValence) : d(new ChangeElementDrawCommandPrivate)
  {
    setText(QObject::tr("Change Element"));
    d->molecule = molecule;
    d->newElement = atom->GetAtomicNum();
    d->oldElement = oldElement;
    d->index = atom->GetIdx();
    d->adjustValence = adjustValence;
  }

  ChangeElementDrawCommand::~ChangeElementDrawCommand()
  {
    delete d;
  }

  void ChangeElementDrawCommand::undo()
  {
    OBAtom *atom = d->molecule->GetAtom(d->index);
    
    if(atom)
      {
        // Make sure we call BeginModify / EndModify (e.g., PR#1720879)
        d->molecule->BeginModify();
        atom->SetAtomicNum(d->oldElement);
        d->molecule->EndModify();
        d->molecule->update();
        if (d->adjustValence) {
          d->molecule->UnsetImplicitValencePerceived();
          d->molecule->DeleteHydrogens(atom);
          d->molecule->AddHydrogens(atom);
        }
      }
  }

  void ChangeElementDrawCommand::redo()
  {
    OBAtom *atom = d->molecule->GetAtom(d->index);
    
    if(atom)
      {
        // Make sure we call BeginModify / EndModify (e.g., PR#1720879)
        d->molecule->BeginModify();
        atom->SetAtomicNum(d->newElement);
        d->molecule->EndModify();
        if (d->adjustValence) {
          d->molecule->UnsetImplicitValencePerceived();
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
    ChangeBondOrderDrawCommandPrivate() : molecule(0), index(0) {};

    Molecule *molecule;
    unsigned int newBondOrder, oldBondOrder;
    int index;
    int adjustValence;
  };

  ChangeBondOrderDrawCommand::ChangeBondOrderDrawCommand(Molecule *molecule, Bond *bond, unsigned int bondOrder, int adjustValence) : d(new ChangeBondOrderDrawCommandPrivate)
  {
    setText(QObject::tr("Change Bond Order"));
    d->molecule = molecule;
    d->oldBondOrder = bond->GetBondOrder();
    d->newBondOrder = bondOrder;
    d->index = bond->GetIdx();
    d->adjustValence = adjustValence;
  }

  ChangeBondOrderDrawCommand::~ChangeBondOrderDrawCommand()
  {
    delete d;
  }

  void ChangeBondOrderDrawCommand::undo()
  {
    OBBond *bond = d->molecule->GetBond(d->index);
    
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

          d->molecule->UnsetImplicitValencePerceived();
          d->molecule->AddHydrogens(a1);
          d->molecule->AddHydrogens(a2);
        }
        d->molecule->update();
      }
  }

  void ChangeBondOrderDrawCommand::redo()
  {
    OBBond *bond = d->molecule->GetBond(d->index);
    
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

          d->molecule->UnsetImplicitValencePerceived();
          d->molecule->AddHydrogens(a1);
          d->molecule->AddHydrogens(a2);
        }
        d->molecule->update();
      }
  }
 
  /////////////////////////////////////////////////////////////////////////////
  // Insert Smiles
  /////////////////////////////////////////////////////////////////////////////

  class InsertSmilesDrawCommandPrivate {
  public:
    InsertSmilesDrawCommandPrivate() : molecule(0), generatedMolecule(0) {};

    Molecule *molecule;
    Molecule moleculeCopy, generatedMolecule;
  };

  InsertSmilesDrawCommand::InsertSmilesDrawCommand(Molecule *molecule, Molecule &generatedMolecule) : d(new InsertSmilesDrawCommandPrivate)
  {
    setText(QObject::tr("Insert SMILES"));
    d->molecule = molecule;
    d->moleculeCopy = *molecule;
    d->generatedMolecule = generatedMolecule;
  }

  InsertSmilesDrawCommand::~InsertSmilesDrawCommand()
  {
    delete d;
  }

  void InsertSmilesDrawCommand::undo()
  {
    *(d->molecule) = d->moleculeCopy;
    d->molecule->update();
  }

  void InsertSmilesDrawCommand::redo()
  {
    *(d->molecule) += d->generatedMolecule;
    d->molecule->update();
  }


} // end namespace Avogadro
