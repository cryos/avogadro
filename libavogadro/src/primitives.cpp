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

  class MoleculePrivate {
    public:
      Eigen::Vector3d       center;
      Eigen::Vector3d       normalVector;
      double                radius;
      Atom *                farthestAtom;
      bool                  invalidGeomInfo;
  };

  Molecule::Molecule(QObject *parent) : OpenBabel::OBMol(), Primitive(MoleculeType, parent), d(new MoleculePrivate) 
  {
    connect(this, SIGNAL(updated()), this, SLOT(updatePrimitive()));
  }

  Molecule::~Molecule()
  {
    delete(d);
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
    d->invalidGeomInfo = true;
  }

  void Molecule::update()
  {
    emit updated();
    d->invalidGeomInfo = true;
  }

  const Eigen::Vector3d & Molecule::center() const
  {
    if( d->invalidGeomInfo ) computeGeomInfo();
    return d->center;
  }
    
  const Eigen::Vector3d & Molecule::normalVector() const
  {
    if( d->invalidGeomInfo ) computeGeomInfo();
    return d->normalVector;
  }
    
  const double & Molecule::radius() const
  {
    if( d->invalidGeomInfo ) computeGeomInfo();
    return d->radius;
  }
    
  const Atom * Molecule::farthestAtom() const
  {
    if( d->invalidGeomInfo ) computeGeomInfo();
    return d->farthestAtom;
  }

  void Molecule::computeGeomInfo() const
  {
    d->invalidGeomInfo = true;
    d->farthestAtom = 0;
    d->center.loadZero();
    d->normalVector.loadZero();
    d->radius = 0.0;
    if( NumAtoms() != 0 )
    {
      // compute center
      std::vector< OpenBabel::OBAtom * >::iterator atom_iterator;
      for( Atom* atom = (Atom*) const_cast<Molecule*>(this)->BeginAtom(atom_iterator); atom; atom = (Atom *) const_cast<Molecule*>(this)->NextAtom(atom_iterator) )
      {
        d->center += atom->pos();
      }
      d->center /= NumAtoms();
  
      // compute the normal vector to the molecule's best-fitting plane
      Eigen::Vector3d * atomPositions = new Eigen::Vector3d[NumAtoms()];
      int i = 0;
      for( Atom* atom = (Atom*) const_cast<Molecule*>(this)->BeginAtom(atom_iterator); atom; atom = (Atom *) const_cast<Molecule*>(this)->NextAtom(atom_iterator) )
      {
        atomPositions[i++] = atom->pos();
      }
      Eigen::Vector4d planeCoeffs;
      Eigen::computeFittingHyperplane( NumAtoms(), atomPositions, &planeCoeffs );
      delete[] atomPositions;
      d->normalVector = Eigen::Vector3d( planeCoeffs.x(), planeCoeffs.y(), planeCoeffs.z() );
      d->normalVector.normalize();
  
      // compute radius and the farthest atom
      d->radius = -1.0; // so that ( squaredDistanceToCenter > d->radius ) is true for at least one atom.
      for( Atom* atom = (Atom*) const_cast<Molecule*>(this)->BeginAtom(atom_iterator); atom; atom = (Atom *) const_cast<Molecule*>(this)->NextAtom(atom_iterator) )
      {
        double distanceToCenter = (atom->pos() - d->center).norm();
        if( distanceToCenter > d->radius )
        {
          d->radius = distanceToCenter;
          d->farthestAtom = atom;
        }
      }
    }
    d->invalidGeomInfo = false;
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
