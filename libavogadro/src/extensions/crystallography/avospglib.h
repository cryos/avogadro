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

#define AVOSPGLIB_TOL 0.1

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
                               const double cartTol = AVOSPGLIB_TOL);

    /**
     * @overload
     *
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
                               const double cartTol = AVOSPGLIB_TOL);

    /**
     * @overload
     *
     * Return the spacegroup number of the crystal described by the
     * arguments.
     *
     * @param mol Molecule from which to extract coordinates
     * @param cell OpenBabel OBUnitCell to use for cell info. If 0
     * (default), mol->OBUnitCell() is used.
     * @param cartTol Tolerance in same units as cellMatrix.
     *
     * @return Spacegroup number if found, 0 otherwise.
     */
    unsigned int getSpacegroup(const Molecule * const mol,
                               OpenBabel::OBUnitCell *cell = 0,
                               const double cartTol = AVOSPGLIB_TOL);

    /**
     * Reduce the crystal described by the arguments to it's primitive
     * form.
     *
     * @param fcoords Fractional coordinates of atoms.
     * @param atomicNums Atomic numbers of atoms
     * @param cellMatrix Unit cell matrix as row vectors.
     * @param cartTol Tolerance in same units as cellMatrix.
     *
     * @return Spacegroup number if found, 0 otherwise.
     */
    unsigned int reduceToPrimitive(QList<Eigen::Vector3d> *fcoords,
                                   QList<unsigned int> *atomicNums,
                                   Eigen::Matrix3d *cellMatrix,
                                   const double cartTol = AVOSPGLIB_TOL);

    /**
     * @overload
     *
     * Reduce the crystal described by the arguments to it's primitive
     * form.
     *
     * @param fcoords Fractional coordinates of atoms.
     * @param ids Element symbols of atoms.
     * @param cellMatrix Unit cell matrix as row vectors.
     * @param cartTol Tolerance in same units as cellMatrix.
     *
     * @return Spacegroup number if found, 0 otherwise.
     */
    unsigned int reduceToPrimitive(QList<Eigen::Vector3d> *fcoords,
                                   QStringList *ids,
                                   Eigen::Matrix3d *cellMatrix,
                                   const double cartTol = AVOSPGLIB_TOL);

    /**
     * @overload
     *
     * Reduce the crystal described by the arguments to it's primitive
     * form. This overload will also set the spacegroup for \a cell.
     *
     * @param mol Molecule from which to extract coordinates
     * @param cell OpenBabel OBUnitCell to use for cell info. If 0
     * (default), mol->OBUnitCell() is used.
     * @param cartTol Tolerance in same units as cellMatrix.
     *
     * @return Spacegroup number if found, 0 otherwise.
     */
    unsigned int reduceToPrimitive(Molecule *mol,
                                   OpenBabel::OBUnitCell *cell = 0,
                                   const double cartTol = AVOSPGLIB_TOL);

    /**
     * Symmetrize the crystal described by the arguments.
     *
     * @note This function returns the symmetrized Bravais cell, which
     * may contain up to four times as many atoms as the original
     * cell.
     *
     * @param fcoords Fractional coordinates of atoms.
     * @param atomicNums Atomic numbers of atoms
     * @param cellMatrix Unit cell matrix as row vectors.
     * @param cartTol Tolerance in same units as cellMatrix.
     *
     * @return Spacegroup number if found, 0 otherwise.
     */
    unsigned int refineCrystal(QList<Eigen::Vector3d> *fcoords,
                               QList<unsigned int> *atomicNums,
                               Eigen::Matrix3d *cellMatrix,
                               const double cartTol = AVOSPGLIB_TOL);

    /**
     * @overload
     *
     * Symmetrize the crystal described by the arguments.
     *
     * @note This function returns the symmetrized Bravais cell, which
     * may contain up to four times as many atoms as the original
     * cell.
     *
     * @param fcoords Fractional coordinates of atoms.
     * @param ids Element symbols of atoms.
     * @param cellMatrix Unit cell matrix as row vectors.
     * @param cartTol Tolerance in same units as cellMatrix.
     *
     * @return Spacegroup number if found, 0 otherwise.
     */
    unsigned int refineCrystal(QList<Eigen::Vector3d> *fcoords,
                               QStringList *ids,
                               Eigen::Matrix3d *cellMatrix,
                               const double cartTol = AVOSPGLIB_TOL);

    /**
     * @overload
     *
     * Symmetrize the crystal described by the arguments.
     *
     * @note This function returns the symmetrized Bravais cell, which
     * may contain up to four times as many atoms as the original
     * cell.
     *
     * @param mol Molecule from which to extract coordinates
     * @param cell OpenBabel OBUnitCell to use for cell info. If 0
     * (default), mol->OBUnitCell() is used.
     * @param cartTol Tolerance in same units as cellMatrix.
     *
     * @return Spacegroup number if found, 0 otherwise.
     */
    unsigned int refineCrystal(Molecule *mol,
                               OpenBabel::OBUnitCell *cell = 0,
                               const double cartTol = AVOSPGLIB_TOL);

  }
}

#endif
