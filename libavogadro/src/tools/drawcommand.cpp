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
    AddAtomDrawCommandPrivate() : molecule(0), atom(0) {};

    Molecule *molecule;
    Atom *atom;
    Eigen::Vector3d pos;
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
    d->atom = atom;
    d->adjustValence = adjustValence;
  }

  AddAtomDrawCommand::~AddAtomDrawCommand()
  {
    delete d;
  }

  void AddAtomDrawCommand::undo()
  {
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
    AddBondDrawCommandPrivate() : molecule(0), bond(0), beginAtom(0), endAtom(0) {};

    Molecule *molecule;
    Bond *bond;
    Atom *beginAtom;
    Atom *endAtom;
    Eigen::Vector3d pos;
    unsigned int order;
    int adjustValence;
  };

  AddBondDrawCommand::AddBondDrawCommand(Molecule *molecule, Atom *beginAtom, Atom *endAtom, unsigned int order, int adjustValence) : d(new AddBondDrawCommandPrivate)
  {
    setText(QObject::tr("Add Bond"));
    d->molecule = molecule;
    d->beginAtom = beginAtom;
    d->endAtom = endAtom;
    d->order = order;
    d->adjustValence = adjustValence;
  }

  AddBondDrawCommand::AddBondDrawCommand(Molecule *molecule, Bond *bond, int adjustValence) : d(new AddBondDrawCommandPrivate)
  {
    setText(QObject::tr("Add Bond"));
    d->molecule = molecule;
    d->beginAtom = static_cast<Atom*>(bond->GetBeginAtom());
    d->endAtom = static_cast<Atom*>(bond->GetEndAtom());
    d->order = bond->GetBondOrder();
    d->bond = bond;
    d->adjustValence = adjustValence;
  }

  AddBondDrawCommand::~AddBondDrawCommand()
  {
    delete d;
  }

  void AddBondDrawCommand::undo()
  {
    if(d->bond)
      {
        d->molecule->BeginModify();
        d->molecule->DeleteBond(d->bond);
        d->molecule->EndModify();
        if (d->adjustValence) {
          if (!d->beginAtom->IsHydrogen())
            d->molecule->DeleteHydrogens(d->beginAtom);
          if (!d->endAtom->IsHydrogen())
            d->molecule->DeleteHydrogens(d->endAtom);
          
          UnsetFlags(d->molecule);
            
	        if (!d->beginAtom->IsHydrogen())
            d->molecule->AddHydrogens(d->beginAtom);
          if (!d->endAtom->IsHydrogen())
            d->molecule->AddHydrogens(d->endAtom);
        }

        d->molecule->update();
        d->bond = 0;
      }
  }

  void AddBondDrawCommand::redo()
  {
    if(!d->beginAtom || !d->endAtom) {
      return;
    }

    if(d->bond) { // already created the bond
      if (d->adjustValence) {
	      if (!d->beginAtom->IsHydrogen())
          d->molecule->DeleteHydrogens(d->beginAtom);
        if (!d->endAtom->IsHydrogen())
          d->molecule->DeleteHydrogens(d->endAtom);
        
        UnsetFlags(d->molecule);
        
        if (!d->beginAtom->IsHydrogen())
	        d->molecule->AddHydrogens(d->beginAtom);
        if (!d->endAtom->IsHydrogen())
          d->molecule->AddHydrogens(d->endAtom);
      }
      d->bond->SetBondOrder(d->order);
      
      return;
    }

    d->molecule->BeginModify();
    d->bond = static_cast<Bond *>(d->molecule->NewBond());
    d->bond->SetBondOrder(d->order);
    d->bond->SetBegin(d->beginAtom);
    d->bond->SetEnd(d->endAtom);
    d->beginAtom->AddBond(d->bond);
    d->endAtom->AddBond(d->bond);
    d->molecule->EndModify();
    if (d->adjustValence) {
      if (!d->beginAtom->IsHydrogen())
        d->molecule->DeleteHydrogens(d->beginAtom);
      if (!d->endAtom->IsHydrogen())
        d->molecule->DeleteHydrogens(d->endAtom);
      
      UnsetFlags(d->molecule);
      
      if (!d->beginAtom->IsHydrogen())
        d->molecule->AddHydrogens(d->endAtom);
      if (!d->endAtom->IsHydrogen())
        d->molecule->AddHydrogens(d->beginAtom);
    }
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
          UnsetFlags(d->molecule);
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
    ChangeBondOrderDrawCommandPrivate() : molecule(0), bond(0) {};

    Molecule *molecule;
    Bond *bond;
    unsigned int newBondOrder, oldBondOrder;
    int adjustValence;
  };

  ChangeBondOrderDrawCommand::ChangeBondOrderDrawCommand(Molecule *molecule, Bond *bond, unsigned int oldBondOrder, int adjustValence) : d(new ChangeBondOrderDrawCommandPrivate)
  {
    setText(QObject::tr("Change Bond Order"));
    d->molecule = molecule;
    d->bond = bond;
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
    if(d->bond)
      {
        // Make sure we call BeginModify / EndModify (e.g., PR#1720879)
        d->molecule->BeginModify();
        d->bond->SetBondOrder(d->oldBondOrder);
        d->molecule->EndModify();
        if (d->adjustValence) {
          OBAtom *a1, *a2;
          a1 = d->bond->GetBeginAtom();
          a2 = d->bond->GetEndAtom();
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
    if(d->bond)
      {
        // Make sure we call BeginModify / EndModify (e.g., PR#1720879)
        d->molecule->BeginModify();
        d->bond->SetBondOrder(d->newBondOrder);
        d->molecule->EndModify();
        if (d->adjustValence) {

          OBAtom *a1, *a2;
          a1 = d->bond->GetBeginAtom();
          a2 = d->bond->GetEndAtom();
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
