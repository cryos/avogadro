/***************************************************************************
 *   copyright            : (C) 2007 by Benoit Jacob
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "config.h"

#include <avogadro/molgeominfo.h>

#include <eigen/regression.h>

using namespace Eigen;

namespace Avogadro
{
  class MolGeomInfoPrivate
  {
    public:
      MolGeomInfoPrivate() {}
      Vector3d       center;
      Vector3d       normalVector;
      double         radius;
      Atom *         farthestAtom;
  };

  MolGeomInfo::MolGeomInfo(const Molecule *molecule) : d(new MolGeomInfoPrivate)
  {
    compute(molecule);
  }

  MolGeomInfo::~MolGeomInfo()
  {
    delete d;
  }

  const Eigen::Vector3d & MolGeomInfo::center() const
  {
    return d->center;
  }

  const Eigen::Vector3d & MolGeomInfo::normalVector() const
  {
    return d->normalVector;
  }

  const double & MolGeomInfo::radius() const
  {
    return d->radius;
  }

  const Atom *MolGeomInfo::farthestAtom() const
  {
    return d->farthestAtom;
  }

  void MolGeomInfo::compute(const Molecule *molecule)
  {
    d->farthestAtom = 0;
    d->center.loadZero();
    d->normalVector.loadZero();
    d->radius = 0.0;
    if( !molecule) return;
    if( molecule->NumAtoms() == 0 ) return;

    // compute center
    std::vector< OpenBabel::OBAtom * >::iterator atom_iterator;
    for( Atom* atom = (Atom*) const_cast<Molecule*>(molecule)->BeginAtom(atom_iterator); atom; atom = (Atom *) const_cast<Molecule*>(molecule)->NextAtom(atom_iterator) )
    {
      d->center += atom->position();
    }
    d->center /= molecule->NumAtoms();

    // compute the normal vector to the molecule's best-fitting plane
    Eigen::Vector3d * atomPositions = new Vector3d[molecule->NumAtoms()];
    int i = 0;
    for( Atom* atom = (Atom*) const_cast<Molecule*>(molecule)->BeginAtom(atom_iterator); atom; atom = (Atom *) const_cast<Molecule*>(molecule)->NextAtom(atom_iterator) )
    {
      atomPositions[i++] = atom->position();
    }
    Vector4d planeCoeffs;
    computeFittingHyperplane( molecule->NumAtoms(), atomPositions, &planeCoeffs );
    delete[] atomPositions;
    d->normalVector = Vector3d( planeCoeffs.x(), planeCoeffs.y(), planeCoeffs.z() );
    d->normalVector.normalize();

    // compute radius and the farthest atom
    d->radius = -1.0; // so that ( squaredDistanceToCenter > d->radius ) is true for at least one atom.
    for( Atom* atom = (Atom*) const_cast<Molecule*>(molecule)->BeginAtom(atom_iterator); atom; atom = (Atom *) const_cast<Molecule*>(molecule)->NextAtom(atom_iterator) )
    {
      double distanceToCenter = (atom->position() - d->center).norm();
      if( distanceToCenter > d->radius )
      {
        d->radius = distanceToCenter;
        d->farthestAtom = atom;
      }
    }
  }

} // end namespace Avogadro
