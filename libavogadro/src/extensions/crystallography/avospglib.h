/**********************************************************************
  AvoSpglib - Spglib wrapper for Avogadro

  Copyright (C) 2011 by David C. Lonie

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.cc/>

  This source code is released under the New BSD License, (the "License").

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
 ***********************************************************************/

#ifndef AVOSPGLIB_H
#define AVOSPGLIB_H

#include "config.h"

#include <Eigen/Core>

#include <QtCore/QList>
#include <QtCore/QString>
#include <QtCore/QSharedPointer>

#define AVOSPGLIB_TOL 0.1

namespace OpenBabel {
  class OBUnitCell;
  class SpaceGroup;
}

namespace Avogadro {
  class Molecule;

  namespace Spglib {

    extern "C" {
#include "spglib/spglib.h"
    }

    // Takes care of allocated data, always instatiate with a deleter.
    typedef QSharedPointer<SpglibDataset> Dataset;

    /**
     * Fetch the Hall symbol given a Hall number.
     *
     * @param hall_number Hall number, an opaque index provided by Dataset.
     *
     * @return Hall symbol if found, empty array otherwise.
     */
    QByteArray getHallSymbol(int hall_number);

    /**
     * Fetch the Hall number given a Hall symbol.
     *
     * @param hall_symbol Hall symbol.
     *
     * @return Hall number if found, 0 otherwise.
     */
    int getHallNumber(const QByteArray hall_symbol);

    /**
     * @overload
     *
     * Fetch the Hall number given a Hall symbol.
     *
     * @param hall_symbol Hall symbol.
     *
     * @return Hall number if found, 0 otherwise.
     */
    int getHallNumber(const char* hall_symbol);

    /**
     * Fetch the OpenBabel spacegroup structure equivalent to the
     * provided Spglib dataset.
     *
     * @param spg_data Spglib dataset.
     *
     * @return OpenBabel spacegroup if found, NULL otherwise.
     */
    const OpenBabel::SpaceGroup* toOpenBabel(Dataset spg_data);

    /**
     * @overload
     *
     * Fetch the OpenBabel spacegroup structure described by the
     * argument.
     *
     * @param hall_symbol Hall symbol.
     *
     * @return OpenBabel spacegroup if found, NULL otherwise.
     */
    const OpenBabel::SpaceGroup* toOpenBabel(const char* hall_symbol);

    /**
     * @overload
     *
     * Fetch the OpenBabel spacegroup structure described by the
     * argument.
     *
     * @param hall_number Hall number, an opaque index provided by Dataset.
     *
     * @return OpenBabel spacegroup if found, NULL otherwise.
     */
    const OpenBabel::SpaceGroup* toOpenBabel(int hall_number);

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
     * Return the spacegroup data of the crystal described by the
     * arguments.
     *
     * @param fcoords Fractional coordinates of atoms.
     * @param atomicNums Atomic numbers of atoms
     * @param cellMatrix Unit cell matrix as row vectors.
     * @param cartTol Tolerance in same units as cellMatrix.
     *
     * @return Spacegroup data if found, null dataset otherwise.
     */
    Dataset getDataset(const QList<Eigen::Vector3d> &fcoords,
                       const QList<unsigned int> &atomicNums,
                       const Eigen::Matrix3d &cellMatrix,
                       const double cartTol = AVOSPGLIB_TOL);

    /**
     * @overload
     *
     * Return the spacegroup data of the crystal described by the
     * arguments.
     *
     * @param fcoords Fractional coordinates of atoms.
     * @param ids Element symbols of atoms.
     * @param cellMatrix Unit cell matrix as row vectors.
     * @param cartTol Tolerance in same units as cellMatrix.
     *
     * @return Spacegroup data if found, null dataset otherwise.
     */
    Dataset  getDataset(const QList<Eigen::Vector3d> &fcoords,
                        const QStringList &ids,
                        const Eigen::Matrix3d &cellMatrix,
                        const double cartTol = AVOSPGLIB_TOL);

    /**
     * @overload
     *
     * Return the spacegroup data of the crystal described by the
     * arguments.
     *
     * @param mol Molecule from which to extract coordinates
     * @param cell OpenBabel OBUnitCell to use for cell info. If 0
     * (default), mol->OBUnitCell() is used.
     * @param cartTol Tolerance in same units as cellMatrix.
     *
     * @return Spacegroup data if found, null dataset otherwise.
     */
    Dataset  getDataset(const Molecule * const mol,
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
     * @param standardize Whether or not to standardize the cell.
     *
     * @return Spacegroup number if found, 0 otherwise.
     */
    unsigned int reduceToPrimitive(QList<Eigen::Vector3d> *fcoords,
                                   QList<unsigned int> *atomicNums,
                                   Eigen::Matrix3d *cellMatrix,
                                   const double cartTol = AVOSPGLIB_TOL,
                                   bool standardize = false);

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
     * @param standardize Whether or not to standardize the cell.
     *
     * @return Spacegroup number if found, 0 otherwise.
     */
    unsigned int reduceToPrimitive(QList<Eigen::Vector3d> *fcoords,
                                   QStringList *ids,
                                   Eigen::Matrix3d *cellMatrix,
                                   const double cartTol = AVOSPGLIB_TOL,
                                   bool standardize = false);

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
     * @param standardize Whether or not to standardize the cell.
     *
     * @return Spacegroup number if found, 0 otherwise.
     */
    unsigned int reduceToPrimitive(Molecule *mol,
                                   OpenBabel::OBUnitCell *cell = 0,
                                   const double cartTol = AVOSPGLIB_TOL,
                                   bool standardize = false);

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
