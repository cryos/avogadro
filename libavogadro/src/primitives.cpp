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

#include <avogadro/primitives.h>
#include <QApplication>
#include <QDebug>

namespace Avogadro {
  class PrimitivePrivate {
    public:
      PrimitivePrivate() : type(Primitive::OtherType), selected(false) {};
      
      enum Primitive::Type type;
      bool selected;
  };

  Primitive::Primitive(QObject *parent) : d(new PrimitivePrivate), QObject(parent)
  {}
  
  Primitive::Primitive(enum Type type, QObject *parent) : d(new PrimitivePrivate), QObject(parent)
  {
    d->type = type;
  }
  
  Primitive::~Primitive()
  {
    delete d;
  }
  
  bool Primitive::isSelected() const
  {
    return d->selected;
  }
  
  void Primitive::setSelected( bool s ) 
  {
    d->selected = s;
  }
  
  void Primitive::toggleSelected()
  {
    d->selected = !d->selected;
  }
  
  enum Primitive::Type Primitive::type() const
  {
    return d->type;
  }

  void Primitive::update()
  {
    emit updated(this);
  }
  
  Molecule::Molecule(QObject *parent) : OpenBabel::OBMol(), Primitive(MoleculeType) 
  {}

  Atom * Molecule::CreateAtom()
  {
    qDebug() << "Molecule::CreateAtom()";
    Atom *atom = new Atom();
    connect(atom, SIGNAL(updated(Primitive *)), this, SLOT(updatePrimitive(Primitive *)));
    emit primitiveAdded(atom);
    return(atom);
  }
  
  Bond * Molecule::CreateBond()
  {
    qDebug() << "Molecule::CreateBond()";
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
  
  void Molecule::DestroyAtom(OpenBabel::OBAtom *obatom)
  {
    qDebug() << "DestroyAtom Called";
    Atom *atom = static_cast<Atom *>(obatom);
    if(atom) {
      emit primitiveRemoved(atom);
      atom->deleteLater();
    }
  }
  
  void Molecule::DestroyBond(OpenBabel::OBBond *obbond)
  {
    qDebug() << "DestroyBond Called";
    Bond *bond = static_cast<Bond *>(obbond);
    if(bond) {
      emit primitiveRemoved(bond);
      bond->deleteLater();
    }
  }
  
  void Molecule::DestroyResidue(OpenBabel::OBResidue *obresidue)
  {
    qDebug() << "DestroyResidue Called";
    Residue *residue = static_cast<Residue *>(obresidue);
    if(residue) {
      emit primitiveRemoved(residue);
      residue->deleteLater();
    }
  }
  
  void Molecule::updatePrimitive(Primitive *primitive)
  {
    emit primitiveUpdated(primitive);
  }
}
