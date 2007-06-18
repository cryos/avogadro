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

#include "config.h"

#include <avogadro/primitive.h>
#include <QDebug>
#include <eigen/regression.h>
#include <openbabel/obiter.h>

using namespace OpenBabel;

namespace Avogadro {

  class PrimitivePrivate {
    public:
      PrimitivePrivate() : type(Primitive::OtherType) {};

      enum Primitive::Type type;
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

  void Primitive::update()
  {
    emit updated();
  }

  class MoleculePrivate : public PrimitivePrivate {
    public:
      MoleculePrivate() : PrimitivePrivate(), farthestAtom(0), invalidGeomInfo(true) {}
      mutable Eigen::Vector3d       center;
      mutable Eigen::Vector3d       normalVector;
      mutable double                radius;
      mutable Atom *                farthestAtom;
      mutable bool                  invalidGeomInfo;
  };

  Molecule::Molecule(QObject *parent) : Primitive(*new MoleculePrivate, MoleculeType, parent), OpenBabel::OBMol()
  {
    connect(this, SIGNAL(updated()), this, SLOT(updatePrimitive()));
  }

  Molecule::Molecule(const Molecule &other) : Primitive(*new MoleculePrivate, MoleculeType, other.parent()), OpenBabel::OBMol(other)
  {
    connect(this, SIGNAL(updated()), this, SLOT(updatePrimitive()));
  }

  Molecule::~Molecule()
  {
  }

  Atom * Molecule::CreateAtom()
  {
    Atom *atom = new Atom(this);
    connect(atom, SIGNAL(updated()), this, SLOT(updatePrimitive()));
    emit primitiveAdded(atom);
    return(atom);
  }

  Bond * Molecule::CreateBond()
  {
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
    Atom *atom = static_cast<Atom *>(obatom);
    if(atom) {
      emit primitiveRemoved(atom);
      atom->deleteLater();
    }
  }

  void Molecule::DestroyBond(OpenBabel::OBBond *obbond)
  {
    Bond *bond = static_cast<Bond *>(obbond);
    if(bond) {
      emit primitiveRemoved(bond);
      bond->deleteLater();
    }
  }

  void Molecule::DestroyResidue(OpenBabel::OBResidue *obresidue)
  {
    Residue *residue = static_cast<Residue *>(obresidue);
    if(residue) {
      emit primitiveRemoved(residue);
      residue->deleteLater();
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
    OpenBabel::OBMol::operator=(other);
	return *this;
  }

  Molecule &Molecule::operator+=(const Molecule& other)
  {
    OpenBabel::OBMol::operator+=(other);

    return *this;
  }

  void Molecule::computeGeomInfo() const
  {
//     MoleculePrivate *d = reinterpret_cast<MoleculePrivate *>(d_ptr);
    Q_D(const Molecule);
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

}

#include "primitive.moc"
