/**********************************************************************
  Primitives - Wrapper class around the OpenBabel classes

  Copyright (C) 2006 by Geoffrey R. Hutchison
  Some portions Copyright (C) 2006 by Donald E. Curtis

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.sourceforge.net/>

  Some code is based on Open Babel
  For more information, see <http://openbabel.sourceforge.net/>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation version 2 of the License.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
 ***********************************************************************/

#include "primitives.moc"

#include <QApplication>
#include <QDebug>

using namespace Avogadro;

Molecule::Molecule(QObject *parent) 
  : OpenBabel::OBMol(), Primitive(MoleculeType) 
{
  // hack against qt const functions
  _self = this;
}

Atom * Molecule::CreateAtom()
{
  Atom *atom = new Atom();
  return(atom);
}

Bond * Molecule::CreateBond()
{
  Bond *bond = new Bond();
  return(bond);
}

Residue * Molecule::CreateResidue()
{
  Residue *residue = new Residue();
  return(residue);
}

Atom * Molecule::NewAtom()
{
  Atom *atom = (Atom *) OBMol::NewAtom();
  emit primitiveAdded(atom);
  return(atom);
}

Bond * Molecule::NewBond()
{
  Bond *bond = (Bond *) OBMol::NewBond();
  emit primitiveAdded(bond);
  return(bond);
}

Residue * Molecule::NewResidue()
{
  Residue *residue = (Residue *) OBMol::NewResidue();
  emit primitiveAdded(residue);
  return(residue);
}
