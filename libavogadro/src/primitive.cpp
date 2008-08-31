/**********************************************************************
  Primitive - Wrapper class around the OpenBabel classes

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

#include <config.h>

#include <avogadro/primitive.h>
#include <Eigen/Regression>

#include <QReadWriteLock>

#include <QDebug>

using namespace OpenBabel;

namespace Avogadro {

  class PrimitivePrivate {
    public:
      PrimitivePrivate() : type(Primitive::OtherType), id(-1) {};

      enum Primitive::Type type;
      QReadWriteLock lock;

      unsigned long id;
  };

  Primitive::Primitive(QObject *parent) : QObject(parent), d_ptr(new PrimitivePrivate) {}

  Primitive::Primitive(enum Type type, QObject *parent) : QObject(parent), d_ptr(new PrimitivePrivate)
  {
    Q_D(Primitive);
    d->type = type;
  }

  Primitive::Primitive(PrimitivePrivate &dd, QObject *parent) : QObject(parent), d_ptr(&dd) {}

  Primitive::Primitive(PrimitivePrivate &dd, enum Type type, QObject *parent) : QObject(parent), d_ptr(&dd)
  {
    Q_D(Primitive);
    d->type = type;
  }

  Primitive::~Primitive()
  {
    delete d_ptr;
  }

  enum Primitive::Type Primitive::type() const
  {
    Q_D(const Primitive);
    return d->type;
  }

  QReadWriteLock *Primitive::lock()
  {
    Q_D(Primitive);
    return &d->lock;
  }

  void Primitive::update()
  {
    emit updated();
  }

  void Primitive::setId(unsigned long m_id)
  {
    Q_D(Primitive);
    d->id = m_id;
  }

  unsigned long Primitive::id() const
  {
    Q_D(const Primitive);
    return d->id;
  }


  class AtomPrivate : public PrimitivePrivate {
    public:
      AtomPrivate() : PrimitivePrivate() {}

  };

  Atom::Atom(QObject *parent) : Primitive(*new AtomPrivate, AtomType, parent), OpenBabel::OBAtom()
  {
  }

  class BondPrivate : public PrimitivePrivate {
    public:
      BondPrivate() : PrimitivePrivate() {}
  };

  Bond::Bond(QObject *parent) : Primitive(*new BondPrivate, BondType, parent), OpenBabel::OBBond()
  {
  }

  class MoleculePrivate : public PrimitivePrivate {
    public:
      MoleculePrivate() : PrimitivePrivate(), farthestAtom(0), invalidGeomInfo(true), autoId(true) {}
      mutable Eigen::Vector3d       center;
      mutable Eigen::Vector3d       normalVector;
      mutable double                radius;
      mutable Atom *                farthestAtom;
      mutable bool                  invalidGeomInfo;
      bool                          autoId;

      std::vector<Atom *>                 atoms;
      std::vector<Bond *>                 bonds;
  };

  Molecule::Molecule(QObject *parent) : Primitive(*new MoleculePrivate, MoleculeType, parent), OpenBabel::OBMol()
  {
    connect(this, SIGNAL(updated()), this, SLOT(updatePrimitive()));
  }

  Molecule::Molecule(const Molecule &other) : Primitive(*new MoleculePrivate, MoleculeType, other.parent()), OpenBabel::OBMol()
  {
    *this = other;
    connect(this, SIGNAL(updated()), this, SLOT(updatePrimitive()));
  }

  Molecule::~Molecule()
  {
  }

  Atom * Molecule::CreateAtom()
  {
    Q_D(Molecule);

    d->lock.lockForWrite();
    Atom *atom = new Atom(this);
    connect(atom, SIGNAL(updated()), this, SLOT(updatePrimitive()));

    if(!d->autoId) {
      d->lock.unlock();
      return(atom);
    }

    atom->setId(d->atoms.size());
    d->atoms.push_back(atom);
    d->lock.unlock();

    emit primitiveAdded(atom);
    return(atom);
  }

  Bond * Molecule::CreateBond()
  {
    Q_D(Molecule);

    d->lock.lockForWrite();
    Bond *bond = new Bond(this);
    connect(bond, SIGNAL(updated()), this, SLOT(updatePrimitive()));

    if(!d->autoId) {
      d->lock.unlock();
      return(bond);
    }

    bond->setId(d->bonds.size());
    d->bonds.push_back(bond);
    d->lock.unlock();

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

  Atom *Molecule::newAtom()
  {
    return static_cast<Atom *>(OBMol::NewAtom());
  }

  // do some fancy footwork when we add an atom previously created
  Atom *Molecule::newAtom(unsigned long id)
  {
    Q_D(Molecule);

    // we have to bypass the emit given by CreateAtom()
    d->autoId = false;
    Atom *atom = static_cast<Atom *>(OBMol::NewAtom());
    d->autoId = true;

    if(id >= d->atoms.size())
    {
      d->atoms.resize(id+1,0);
    }
    atom->setId(id);
    d->atoms[id] = atom;

    // now that the id is correct, emit the signal
    emit primitiveAdded(atom);
    return(atom);
  }

  void Molecule::DestroyAtom(OpenBabel::OBAtom *obatom)
  {
    Q_D(Molecule);
    Atom *atom = static_cast<Atom *>(obatom);
    if(atom) {
      atom->deleteLater();
      d->atoms[atom->id()] = 0;
      emit primitiveRemoved(atom);
    }
  }

  Atom *Molecule::getAtomById(unsigned long id) const
  {
    Q_D(const Molecule);
    if(id < d->atoms.size())
    {
      return d->atoms[id];
    }
    return 0;
  }

  Bond *Molecule::newBond()
  {
    return static_cast<Bond *>(OBMol::NewBond());
  }

  Bond *Molecule::newBond(unsigned long id)
  {
    Q_D(Molecule);

    d->autoId = false;
    Bond *bond = static_cast<Bond *>(OBMol::NewBond());
    d->autoId = true;

    if(id >= d->bonds.size())
    {
      d->bonds.resize(id+1,0);
    }
    bond->setId(id);
    d->bonds[id] = bond;

    // now that the id is correct, emit the signal
    emit primitiveAdded(bond);
    return(bond);
  }

  void Molecule::DestroyBond(OpenBabel::OBBond *obbond)
  {
    Q_D(Molecule);
    Bond *bond = static_cast<Bond *>(obbond);
    if(bond) {
      bond->deleteLater();
      d->bonds[bond->id()] = 0;
      emit primitiveRemoved(bond);
    }
  }

  Bond *Molecule::getBondById(unsigned long id) const
  {
    Q_D(const Molecule);
    if(id < d->bonds.size())
    {
      return d->bonds[id];
    }
    return 0;
  }

  void Molecule::DestroyResidue(OpenBabel::OBResidue *obresidue)
  {
    Residue *residue = static_cast<Residue *>(obresidue);
    if(residue) {
      residue->deleteLater();
      emit primitiveRemoved(residue);
    }
  }

  void Molecule::updatePrimitive()
  {
    Q_D(Molecule);
    Primitive *primitive = qobject_cast<Primitive *>(sender());
    d->invalidGeomInfo = true;
    emit primitiveUpdated(primitive);
  }

  void Molecule::update()
  {
    Q_D(Molecule);
    d->invalidGeomInfo = true;
    emit updated();
  }

  const Eigen::Vector3d & Molecule::center() const
  {
    Q_D(const Molecule);
    if( d->invalidGeomInfo ) computeGeomInfo();
    return d->center;
  }

  const Eigen::Vector3d & Molecule::normalVector() const
  {
    Q_D(const Molecule);
    if( d->invalidGeomInfo ) computeGeomInfo();
    return d->normalVector;
  }

  const double & Molecule::radius() const
  {
    Q_D(const Molecule);
    if( d->invalidGeomInfo ) computeGeomInfo();
    return d->radius;
  }

  const Atom * Molecule::farthestAtom() const
  {
    Q_D(const Molecule);
    if( d->invalidGeomInfo ) computeGeomInfo();
    return d->farthestAtom;
  }

  Molecule &Molecule::operator=(const Molecule& other)
  {
    Q_D(Molecule);

    Clear();
    d->autoId = false;
    OpenBabel::OBMol::operator=(other);
    const MoleculePrivate *e = other.d_func();
    d->atoms.resize(e->atoms.size(),0);
    d->bonds.resize(e->bonds.size(),0);


    d->autoId = true;
    // copy the unique ids of the atoms
    std::vector<OpenBabel::OBAtom*>::iterator i;
    for(Atom *lhsAtom = static_cast< Atom* >(BeginAtom( i ));
        lhsAtom; lhsAtom = static_cast< Atom* >(NextAtom( i )))
    {
      Atom *rhsAtom = static_cast< Atom* >(other.GetAtom(lhsAtom->GetIdx()));
      unsigned long id = rhsAtom->id();
      lhsAtom->setId(id);
      d->atoms[id] = lhsAtom;
      emit primitiveAdded(lhsAtom);
    }

    // copy the unique ids of the bonds
    std::vector<OpenBabel::OBBond*>::iterator j;
    for(Bond *lhsBond = static_cast< Bond* >(BeginBond( j ));
        lhsBond; lhsBond = static_cast< Bond* >(NextBond( j )))
    {
      Bond *rhsBond = static_cast< Bond* >(other.GetBond(lhsBond->GetIdx()));
      unsigned long id = rhsBond->id();
      lhsBond->setId(id);
      d->bonds[id] = lhsBond;
      emit primitiveAdded(lhsBond);
    }

    std::vector<OpenBabel::OBResidue*>::iterator k;
    for(Residue *lhsResidue = static_cast< Residue* >(BeginResidue( k ));
        lhsResidue; lhsResidue = static_cast< Residue* >(NextResidue( k )))
    {
      emit primitiveAdded(lhsResidue);
    }


	return *this;
  }

  Molecule &Molecule::operator+=(const Molecule& other)
  {
    OpenBabel::OBMol::operator+=(other);

    return *this;
  }

  void Molecule::computeGeomInfo() const
  {
    Q_D(const Molecule);
    d->invalidGeomInfo = true;
    d->farthestAtom = 0;
    d->center.setZero();
    d->normalVector.setZero();
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
      Eigen::Vector3d ** atomPositions = new Eigen::Vector3d*[NumAtoms()];
      int i = 0;
      for( Atom* atom = (Atom*) const_cast<Molecule*>(this)->BeginAtom(atom_iterator); atom; atom = (Atom *) const_cast<Molecule*>(this)->NextAtom(atom_iterator) )
      {
        atomPositions[i++] = const_cast<Eigen::Vector3d*>(&atom->pos());
      }
      Eigen::Hyperplane<double, 3> planeCoeffs;
      Eigen::fitHyperplane( NumAtoms(), atomPositions, &planeCoeffs );
      delete[] atomPositions;
      d->normalVector = planeCoeffs.normal();

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

}

#include "primitive.moc"
