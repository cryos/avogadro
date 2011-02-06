/**********************************************************************
  AvoSpglib - Spglib wrapper for Avogadro

  Copyright (C) 2011 by David C. Lonie

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.openmolecules.net/>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation version 2 of the License.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
 ***********************************************************************/

#ifndef AVOSPGLIB_H
#define AVOSPGLIB_H

#include <Eigen/Core>

#include <QtCore/QList>
#include <QtCore/QString>

namespace OpenBabel {
  class OBUnitCell;
}

namespace Avogadro {
  class Molecule;

  namespace Spglib {

    extern "C" {
#include "spglib/spglib.h"
    }

    /**
     * Return the spacegroup number of the crystal described by the
     * arguments.
     *
     * @param fcoords Fractional coordinates of atoms.
     * @param atomicNums Atomic numbers of atoms
     * @param cellMatrix Unit cell matrix as row vectors.
     * @param cartTol Tolerance in same units as cellMatrix.
     *
     * @return Spacegroup number if found, 0 otherwise.
     */
    unsigned int getSpacegroup(const QList<Eigen::Vector3d> &fcoords,
                               const QList<unsigned int> &atomicNums,
                               const Eigen::Matrix3d &cellMatrix,
                               const double cartTol);

    /**
     * Return the spacegroup number of the crystal described by the
     * arguments.
     *
     * @param fcoords Fractional coordinates of atoms.
     * @param ids Element symbols of atoms.
     * @param cellMatrix Unit cell matrix as row vectors.
     * @param cartTol Tolerance in same units as cellMatrix.
     *
     * @return Spacegroup number if found, 0 otherwise.
     */
    unsigned int getSpacegroup(const QList<Eigen::Vector3d> &fcoords,
                               const QStringList &ids,
                               const Eigen::Matrix3d &cellMatrix,
                               const double cartTol);

    /**
     * @overload
     *
     * Return the spacegroup number of the crystal described by the
     * arguments.
     *
     * @param mol Molecule from which to extract coordinates
     * @param cell OpenBabel OBUnitCell to use for cell info
     * @param cartTol Tolerance in same units as cellMatrix.
     *
     * @return Spacegroup number if found, 0 otherwise.
     */
    unsigned int getSpacegroup(const Molecule * const mol,
                               OpenBabel::OBUnitCell * cell,
                               const double cartTol);

    /**
     * @overload
     *
     * Return the spacegroup number of the crystal described by the
     * arguments.
     *
     * @param mol Molecule from which to extract coordinates and unit
     * cell info
     * @param cartTol Tolerance in same units as cellMatrix.
     *
     * @return Spacegroup number if found, 0 otherwise.
     */
    unsigned int getSpacegroup(const Molecule * const mol,
                               const double cartTol);

  }
}

#endif
