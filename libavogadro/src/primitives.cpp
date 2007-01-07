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
}

Atom * Molecule::CreateAtom()
{
  qDebug() << "CreateAtom Called()";
  Atom *atom = new Atom();
  connect(atom, SIGNAL(updated(Primitive *)), this, SLOT(updatePrimitive(Primitive *)));
  emit primitiveAdded(atom);
  return(atom);
}

Bond * Molecule::CreateBond()
{
  Bond *bond = new Bond();
  connect(bond, SIGNAL(updated(Primitive *)), this, SLOT(updatePrimitive(Primitive *)));
  emit primitiveAdded(bond);
  return(bond);
}

Residue * Molecule::CreateResidue()
{
  Residue *residue = new Residue();
  connect(residue, SIGNAL(updated(Primitive *)), this, SLOT(updatePrimitive(Primitive *)));
  emit primitiveAdded(residue);
  return(residue);
}

void Molecule::DestroyAtom(OpenBabel::OBAtom *atom)
{
  qDebug() << "DestroyAtom Called";
  if(atom) {
    emit primitiveRemoved(static_cast<Atom *>(atom));
    delete atom;
  }
}

void Molecule::DestroyBond(OpenBabel::OBBond *bond)
{
  if(bond) {
    emit primitiveRemoved(static_cast<Bond *>(bond));
    delete bond;
  }
}

void Molecule::DestroyResidue(OpenBabel::OBResidue *residue)
{
  if(residue) {
    emit primitiveRemoved(static_cast<Residue *>(residue));
    delete residue;
  }
}

void Molecule::updatePrimitive(Primitive *primitive)
{
  emit primitiveUpdated(primitive);
}

void Primitive::update()
{
  emit updated(this);
}
