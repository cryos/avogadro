/**********************************************************************
  Primitives - Wrapper class around the OpenBabel classes

  Copyright (C) 2006 by Geoffrey R. Hutchison
  Copyright (C) 2006,2007 by Donald Ephraim Curtis

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

#include "config.h"

#include <avogadro/primitives.h>
#include <QDebug>
#include <eigen/regression.h>

namespace Avogadro {

  class PrimitivePrivate {
    public:
      PrimitivePrivate() : type(Primitive::OtherType), selected(false) {};

      enum Primitive::Type type;
      bool selected;
  };

  Primitive::Primitive(QObject *parent) : d(new PrimitivePrivate), QObject(parent) {}

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
    emit updated();
  }

  Molecule::Molecule(QObject *parent) : OpenBabel::OBMol(), Primitive(MoleculeType, parent) 
  {
    connect(this, SIGNAL(updated()), this, SLOT(updatePrimitive()));
  }

  Atom * Molecule::CreateAtom()
  {
    qDebug() << "Molecule::CreateAtom()";
    Atom *atom = new Atom(this);
    connect(atom, SIGNAL(updated()), this, SLOT(updatePrimitive()));
    emit primitiveAdded(atom);
    return(atom);
  }

  Bond * Molecule::CreateBond()
  {
    qDebug() << "Molecule::CreateBond()";
    Bond *bond = new Bond(this);
    connect(bond, SIGNAL(updated()), this, SLOT(updatePrimitive()));
    emit primitiveAdded(bond);
    return(bond);
  }

  Residue * Molecule::CreateResidue()
  {
    Residue *residue = new Residue(this);
    connect(residue, SIGNAL(updated()), this, SLOT(updatePrimitive()));
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

  void Molecule::updatePrimitive()
  {
    Primitive *primitive = qobject_cast<Primitive *>(sender());
    emit primitiveUpdated(primitive);
    qDebug() << "calling computeGeometricInfo() from Molecule::updatePrimitive()";
    computeGeometricInfo();
  }

  void Molecule::update()
  {
    emit updated();
    qDebug() << "calling computeGeometricInfo() from Molecule::update()";
    computeGeometricInfo();
  }

  void Molecule::computeGeometricInfo()
  {
    _atomFarthestFromCenter = 0;
    _center.loadZero();
    _normalVector.loadZero();
    _radius = 0.0;


    // check that there are any atoms, compute _center
    if( NumAtoms() == 0 ) return;
    std::vector< OpenBabel::OBAtom * >::iterator atom_iterator;
    for( Atom* atom = (Atom*) BeginAtom(atom_iterator); atom; atom = (Atom *) NextAtom(atom_iterator) )
    {
      _center += atom->position();
    }
    _center /= NumAtoms();

    // compute the normal vector to the molecule's best-fitting plane
    Eigen::Vector3d * atomPositions = new Eigen::Vector3d[NumAtoms()];
    int i = 0;

    for( Atom* atom = (Atom*) BeginAtom(atom_iterator); atom; atom = (Atom *) NextAtom(atom_iterator) )
    {
      atomPositions[i++] = atom->position();
    }
    Eigen::Vector4d planeCoeffs;
    Eigen::computeFittingHyperplane( NumAtoms(), atomPositions, &planeCoeffs );
    delete[] atomPositions;
    _normalVector = Eigen::Vector3d( planeCoeffs.x(), planeCoeffs.y(), planeCoeffs.z() );
    _normalVector.normalize();

    // compute radius and the farthest atom
    _radius = -1.0; // so that ( squaredDistanceToCenter > _radius ) is true for at least one atom.
    for( Atom* atom = (Atom*) BeginAtom(atom_iterator); atom; atom = (Atom *) NextAtom(atom_iterator) )
    {
      double distanceToCenter = (atom->position() - _center).norm();
      if( distanceToCenter > _radius )
      {
        _radius = distanceToCenter;
        _atomFarthestFromCenter = atom;
      }
    }
  }

  class PrimitiveQueuePrivate {
    public:
      PrimitiveQueuePrivate() {};

      QList< QList<Primitive *>* > queue;
  };

  PrimitiveQueue::PrimitiveQueue() : d(new PrimitiveQueuePrivate) { 
    for( int type=Primitive::FirstType; type<Primitive::LastType; type++ ) { 
      d->queue.append(new QList<Primitive *>()); 
    } 
  }

  PrimitiveQueue::~PrimitiveQueue() { 
    for( int i = 0; i<d->queue.size(); i++ ) { 
      delete d->queue[i];
    } 
    delete d;
  }

  const QList<Primitive *>* PrimitiveQueue::primitiveList(enum Primitive::Type type) const { 
    return(d->queue[type]); 
  }

  void PrimitiveQueue::addPrimitive(Primitive *p) { 
    d->queue[p->type()]->append(p); 
  }

  void PrimitiveQueue::removePrimitive(Primitive *p) {
    d->queue[p->type()]->removeAll(p);
  }

  int PrimitiveQueue::size() const {
    int sum = 0;
    for( int i=0; i<d->queue.size(); i++ ) {
      sum += d->queue[i]->size();
    }
    return sum;
  }

  void PrimitiveQueue::clear() {
    for( int i=0; i<d->queue.size(); i++ ) {
      d->queue[i]->clear();
    }
  }
}

#include "primitives.moc"
