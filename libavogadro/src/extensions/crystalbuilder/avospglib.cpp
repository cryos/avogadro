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

#include "avospglib.h"

#include "obeigenconv.h"

#include <avogadro/atom.h>
#include <avogadro/molecule.h>

#include <openbabel/mol.h>
#include <openbabel/generic.h>

#include <QtCore/QDebug>
#include <QtCore/QStringList>

namespace Avogadro {
  namespace Spglib {

    unsigned int reduceToPrimitive(QList<Eigen::Vector3d> *fcoords,
                                   QList<unsigned int> *atomicNums,
                                   Eigen::Matrix3d *cellMatrix,
                                   const double cartTol)
    {
      Q_ASSERT(fcoords->size() == atomicNums->size());

      const int numAtoms = fcoords->size();

      if (numAtoms < 1) {
        qWarning() << "Cannot determine spacegroup of empty cell.";
        return 0;
      }

      // Spglib expects column vecs, so fill with transpose
      double lattice[3][3] = {
        {(*cellMatrix)(0,0), (*cellMatrix)(1,0), (*cellMatrix)(2,0)},
        {(*cellMatrix)(0,1), (*cellMatrix)(1,1), (*cellMatrix)(2,1)},
        {(*cellMatrix)(0,2), (*cellMatrix)(1,2), (*cellMatrix)(2,2)}
      };

      // Build position list
      double (*positions)[3] = new double[numAtoms][3];
      int *types = new int[numAtoms];
      const Eigen::Vector3d * fracCoord;
      for (int i = 0; i < numAtoms; ++i) {
        fracCoord         = &(*fcoords)[i];
        types[i]          = (*atomicNums)[i];
        positions[i][0]   = fracCoord->x();
        positions[i][1]   = fracCoord->y();
        positions[i][2]   = fracCoord->z();
      }

      // Find primitive cell. This updates lattice, positions, types
      // to primitive
      int numPrimitiveAtoms =
        spg_find_primitive(lattice, positions, types,
                           numAtoms, cartTol);

      // if spglib cannot find the primitive cell, return 0.
      if (numPrimitiveAtoms == 0) {
        return 0;
      }

      // find spacegroup
      char symbol[21];
      int spg = spg_get_international(symbol,
                                      lattice,
                                      positions,
                                      types,
                                      numPrimitiveAtoms,
                                      cartTol);

      // Update passed objects
      // convert col vecs to row vecs
      (*cellMatrix)(0, 0) =  lattice[0][0];
      (*cellMatrix)(0, 1) =  lattice[1][0];
      (*cellMatrix)(0, 2) =  lattice[2][0];
      (*cellMatrix)(1, 0) =  lattice[0][1];
      (*cellMatrix)(1, 1) =  lattice[1][1];
      (*cellMatrix)(1, 2) =  lattice[2][1];
      (*cellMatrix)(2, 0) =  lattice[0][2];
      (*cellMatrix)(2, 1) =  lattice[1][2];
      (*cellMatrix)(2, 2) =  lattice[2][2];

      // Trim
      while (fcoords->size() > numPrimitiveAtoms) {
        fcoords->removeLast();
        atomicNums->removeLast();
      }

      // Update
      for (int i = 0; i < numPrimitiveAtoms; ++i) {
        (*atomicNums)[i]  = types[i];
        (*fcoords)[i] = Eigen::Vector3d (positions[i]);
      }

      delete [] positions;
      delete [] types;

      if (spg > 230 || spg < 0) {
        spg = 0;
      }

      return static_cast<unsigned int>(spg);
    }

    unsigned int reduceToPrimitive(QList<Eigen::Vector3d> *fcoords,
                                   QStringList *ids,
                                   Eigen::Matrix3d *cellMatrix,
                                   const double cartTol)
    {
      QList<unsigned int> atomicNums;
      atomicNums.reserve(ids->size());

      for (int i = 0; i < ids->size(); ++i) {
        atomicNums.append(OpenBabel::etab.GetAtomicNum
                          ((*ids)[i].toStdString().c_str()));
      }

      unsigned int spg =
        reduceToPrimitive(fcoords, &atomicNums,
                          cellMatrix, cartTol);

      // Save some time if spg couldn't be found.
      if (spg == 0) {
        return 0;
      }

      Q_ASSERT(fcoords->size() == atomicNums.size());

      while (ids->size() > atomicNums.size()) {
        ids->removeLast();
      }

      for (int i = 0; i < atomicNums.size(); ++i) {
        (*ids)[i] = QString(OpenBabel::etab.GetSymbol
                            (atomicNums[i]));
      }

      return spg;
    }

    unsigned int reduceToPrimitive(Molecule * mol,
                                   OpenBabel::OBUnitCell * cell,
                                   const double cartTol)
    {
      Q_ASSERT(mol);
      Q_ASSERT(cell);

      QList<Atom*> atoms = mol->atoms();
      const unsigned int numAtoms =
        static_cast<unsigned int>(atoms.size());

      Eigen::Matrix3d cellMatrix = OB2Eigen(cell->GetCellMatrix());
      QList<Eigen::Vector3d> fcoords;
      QList<unsigned int> atomicNums;

      fcoords.reserve(numAtoms);
      atomicNums.reserve(numAtoms);

      for (unsigned int i = 0; i < numAtoms; ++i) {
        fcoords.append(OB2Eigen
                       (cell->CartesianToFractional
                        (Eigen2OB(*(atoms[i]->pos())))));
        atomicNums.append(atoms[i]->atomicNumber());
      }

      unsigned int spg =
        reduceToPrimitive(&fcoords, &atomicNums,
                          &cellMatrix, cartTol);

      // Save some time if no change
      if (spg == 0) {
        return 0;
      }

      cell->SetData(Eigen2OB(cellMatrix));

      QList<Atom*> toDelete = mol->atoms();
      for (QList<Atom*>::iterator
             it = toDelete.begin(),
             it_end = toDelete.end();
           it != it_end; ++it) {
        mol->removeAtom(*it);
      }

      for (int i = 0; i < fcoords.size(); ++i) {
        Atom *atom = mol->addAtom();
        atom->setAtomicNumber(atomicNums[i]);
        atom->setPos(OB2Eigen
                     (cell->FractionalToCartesian
                      (Eigen2OB(fcoords[i]))));
      }

      return spg;
    }

    unsigned int reduceToPrimitive(Molecule *mol,
                                   const double cartTol)
    {
      Q_ASSERT(mol);
      return reduceToPrimitive(mol, mol->OBUnitCell(), cartTol);
    }

    unsigned int getSpacegroup(const QList<Eigen::Vector3d> &fcoords,
                               const QList<unsigned int> &atomicNums,
                               const Eigen::Matrix3d &cellMatrix,
                               const double cartTol)
    {
      Q_ASSERT(fcoords.size() == atomicNums.size());

      const int numAtoms = fcoords.size();

      if (numAtoms < 1) {
        qWarning() << "Cannot determine spacegroup of empty cell.";
        return 0;
      }

      // Spglib expects column vecs, so fill with transpose
      double lattice[3][3] = {
        {cellMatrix(0,0), cellMatrix(1,0), cellMatrix(2,0)},
        {cellMatrix(0,1), cellMatrix(1,1), cellMatrix(2,1)},
        {cellMatrix(0,2), cellMatrix(1,2), cellMatrix(2,2)}
      };

      // Build position list
      double (*positions)[3] = new double[numAtoms][3];
      int *types = new int[numAtoms];
      const Eigen::Vector3d * fracCoord;
      for (int i = 0; i < numAtoms; ++i) {
        fracCoord         = &fcoords[i];
        types[i]          = atomicNums[i];
        positions[i][0]   = fracCoord->x();
        positions[i][1]   = fracCoord->y();
        positions[i][2]   = fracCoord->z();
      }

      // Find primitive cell. This updates lattice, positions, types to primitive
      unsigned int numPrimitiveAtoms =
        spg_find_primitive(lattice, positions, types, numAtoms, cartTol);

      // if spglib cannot find the primitive cell, reset the number
      // of atoms to numAtoms
      if (numPrimitiveAtoms == 0) {
        numPrimitiveAtoms = numAtoms;
      }

      // find spacegroup
      char symbol[21];
      int spg = spg_get_international(symbol,
                                      lattice,
                                      positions,
                                      types,
                                      numPrimitiveAtoms,
                                      cartTol);

      delete [] positions;
      delete [] types;

      if (spg > 230 || spg < 0) {
        spg = 0;
      }

      return static_cast<unsigned int>(spg);
    }

    unsigned int getSpacegroup(const QList<Eigen::Vector3d> &fcoords,
                               const QStringList &ids,
                               const Eigen::Matrix3d &cellMatrix,
                               const double cartTol)
    {
      QList<unsigned int> atomicNums;
      atomicNums.reserve(ids.size());

      for (int i = 0; i < ids.size(); ++i) {
        atomicNums.append(OpenBabel::etab.GetAtomicNum
                          (ids[i].toStdString().c_str()));
      }

      return getSpacegroup(fcoords, atomicNums, cellMatrix, cartTol);
    }

    unsigned int getSpacegroup(const Molecule * const mol,
                               OpenBabel::OBUnitCell * cell,
                               const double cartTol)
    {
      Q_ASSERT(mol);
      Q_ASSERT(cell);

      QList<Atom*> atoms = mol->atoms();
      const unsigned int numAtoms = static_cast<unsigned int>(atoms.size());

      Eigen::Matrix3d cellMatrix = OB2Eigen(cell->GetCellMatrix());
      QList<Eigen::Vector3d> fcoords;
      QList<unsigned int> atomicNums;

      fcoords.reserve(numAtoms);
      atomicNums.reserve(numAtoms);

      for (unsigned int i = 0; i < numAtoms; ++i) {
        fcoords.append(OB2Eigen
                       (cell->CartesianToFractional
                        (Eigen2OB(*(atoms[i]->pos())))));
        atomicNums.append(atoms[i]->atomicNumber());
      }

      return getSpacegroup(fcoords, atomicNums, cellMatrix, cartTol);
    }

    unsigned int getSpacegroup(const Molecule * const mol, const double cartTol)
    {
      Q_ASSERT(mol);
      return getSpacegroup(mol, mol->OBUnitCell(), cartTol);
    }
  }
}
