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

#define SPG_DUMP_ATOMS(desc, numAtoms)                                  \
  qDebug() << desc << numAtoms << "atoms";                              \
  for (int i = 0; i < numAtoms; ++i) {                                  \
    qDebug() << types[i] << positions[i][0]                             \
             << positions[i][1] << positions[i][2];                     \
  }

// Some helper functions
namespace {
  // Fill fcoords, atomicNums, and cellMatrix from mol and cell
  void prepareMolecule(const Avogadro::Molecule *mol,
                       const OpenBabel::OBUnitCell *cell,
                       QList<Eigen::Vector3d> *fcoords,
                       QList<unsigned int> *atomicNums,
                       Eigen::Matrix3d *cellMatrix)
  {
    // This cast is because OBUnitCell is not const correct. Remove it
    // when OB is fixed or we start using Avogadro::UnitCell
    OpenBabel::OBUnitCell *mutcell =
      const_cast<OpenBabel::OBUnitCell*>(cell);
    *cellMatrix = Avogadro::OB2Eigen(mutcell->GetCellMatrix());

    QList<Avogadro::Atom*> atoms = mol->atoms();

    const unsigned int numAtoms =
      static_cast<unsigned int>(atoms.size());

    fcoords->clear();
    atomicNums->clear();
    fcoords->reserve(numAtoms);
    atomicNums->reserve(numAtoms);

    for (QList<Avogadro::Atom*>::const_iterator
           it = atoms.constBegin(),
           it_end = atoms.constEnd();
         it != it_end; ++it) {
      fcoords->append(Avogadro::OB2Eigen
                      (mutcell->CartesianToFractional
                       (Avogadro::Eigen2OB(*((*it)->pos())))));
      atomicNums->append((*it)->atomicNumber());
    }
  }

  void applyToMolecule(Avogadro::Molecule *mol,
                       OpenBabel::OBUnitCell *cell,
                       const QList<Eigen::Vector3d> &fcoords,
                       const QList<unsigned int> &atomicNums,
                       const Eigen::Matrix3d &cellMatrix)
  {
    cell->SetData(Avogadro::Eigen2OB(cellMatrix));

    // Remove / add atoms until correct.
    const unsigned int numAtoms = static_cast<unsigned int>(fcoords.size());
    if (numAtoms != mol->numAtoms()) {
      QList<Avogadro::Atom*> atoms = mol->atoms();
      // Delete atoms from end of mol
      QList<Avogadro::Atom*>::const_iterator toDelete = atoms.constEnd();
      while (numAtoms < mol->numAtoms()) {
        --toDelete;
        mol->removeAtom(*(--toDelete)); // On first iteration this
                                        // will be the final atom.
      }
      while (numAtoms > mol->numAtoms()) {
        mol->addAtom();
      }
    }

    QList<Avogadro::Atom*> atoms = mol->atoms();
    Q_ASSERT(atoms.size() == fcoords.size());
    Q_ASSERT(atomicNums.size() == fcoords.size());
    for (int i = 0; i < fcoords.size(); ++i) {
      atoms[i]->setAtomicNumber(atomicNums[i]);
      atoms[i]->setPos(Avogadro::OB2Eigen
                   (cell->FractionalToCartesian
                    (Avogadro::Eigen2OB(fcoords[i]))));
    }
  }

  QList<unsigned int> symbolsToAtomicNumbers(const QList<QString> &ids)
  {
    QList<unsigned int> atomicNums;
    atomicNums.reserve(ids.size());
    for (QStringList::const_iterator
           it = ids.constBegin(),
           it_end = ids.constEnd();
         it != it_end; ++it) {
      atomicNums.append(OpenBabel::etab.GetAtomicNum
                        (it->toStdString().c_str()));
    }
    return atomicNums;
  }

  QList<QString> atomicNumbersToSymbols(const QList<unsigned int> &atomicNums)
  {
    QList<QString> ids;
    ids.reserve(atomicNums.size());
    for (QList<unsigned int>::const_iterator
           it = atomicNums.constBegin(),
           it_end = atomicNums.constEnd();
         it != it_end; ++it) {
      ids.append(OpenBabel::etab.GetSymbol(*it));
    }
    return ids;
  }

} // end anon namespace

namespace Avogadro {
  namespace Spglib {
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

      // find spacegroup
      char symbol[21];
      int spg = spg_get_international(symbol,
                                      lattice,
                                      positions,
                                      types,
                                      numAtoms,
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
      const QList<unsigned int> atomicNums = symbolsToAtomicNumbers(ids);

      return getSpacegroup(fcoords, atomicNums, cellMatrix, cartTol);
    }

    unsigned int getSpacegroup(const Molecule * const mol,
                               OpenBabel::OBUnitCell *cell,
                               const double cartTol)
    {
      Q_ASSERT(mol);
      if (!cell) {
        cell = mol->OBUnitCell();
      }
      Q_ASSERT(cell);

      QList<Eigen::Vector3d> fcoords;
      QList<unsigned int> atomicNums;
      Eigen::Matrix3d cellMatrix;

      prepareMolecule(mol, cell, &fcoords, &atomicNums, &cellMatrix);

      return getSpacegroup(fcoords, atomicNums, cellMatrix, cartTol);
    }

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

      // Build position list. Include space for 4*numAtoms for the
      // cell refinement
      double (*positions)[3] = new double[4*numAtoms][3];
      int *types = new int[4*numAtoms];
      const Eigen::Vector3d * fracCoord;
      for (int i = 0; i < numAtoms; ++i) {
        fracCoord         = &(*fcoords)[i];
        types[i]          = (*atomicNums)[i];
        positions[i][0]   = fracCoord->x();
        positions[i][1]   = fracCoord->y();
        positions[i][2]   = fracCoord->z();
      }

      // find spacegroup for return value
      char symbol[21];
      int spg = spg_get_international(symbol,
                                      lattice,
                                      positions,
                                      types,
                                      numAtoms,
                                      cartTol);
      SPG_DUMP_ATOMS("Original cell", numAtoms);

      // Refine the structure
      int numBravaisAtoms =
        spg_refine_cell(lattice, positions, types,
                        numAtoms, cartTol);
      SPG_DUMP_ATOMS("Bravais cell", numBravaisAtoms);

      // if spglib cannot refine the cell, return 0.
      if (numBravaisAtoms <= 0) {
        return 0;
      }

      // Find primitive cell. This updates lattice, positions, types
      // to primitive
      int numPrimitiveAtoms =
        spg_find_primitive(lattice, positions, types,
                           numBravaisAtoms, cartTol);

      // If the cell was already a primitive cell, reset
      // numPrimitiveAtoms.
      if (numPrimitiveAtoms == 0) {
        numPrimitiveAtoms = numBravaisAtoms;
      }

      SPG_DUMP_ATOMS("Primitive cell", numPrimitiveAtoms);

      // Bail if everything failed
      if (numPrimitiveAtoms <= 0) {
        return 0;
      }

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
      while (fcoords->size() < numPrimitiveAtoms) {
        fcoords->append(Eigen::Vector3d());
        atomicNums->append(0);
      }

      // Update
      Q_ASSERT(fcoords->size() == atomicNums->size());
      Q_ASSERT(fcoords->size() == numPrimitiveAtoms);
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
      QList<unsigned int> atomicNums = symbolsToAtomicNumbers(*ids);

      unsigned int spg =
        reduceToPrimitive(fcoords, &atomicNums,
                          cellMatrix, cartTol);

      // Save some time if spg couldn't be found.
      if (spg <= 0 || spg > 230) {
        return 0;
      }

      Q_ASSERT(fcoords->size() == atomicNums.size());

      *ids = atomicNumbersToSymbols(atomicNums);

      return spg;
    }

    unsigned int reduceToPrimitive(Molecule * mol,
                                   OpenBabel::OBUnitCell * cell,
                                   const double cartTol)
    {
      Q_ASSERT(mol);
      if (!cell) {
        cell = mol->OBUnitCell();
      }
      Q_ASSERT(cell);

      QList<Eigen::Vector3d> fcoords;
      QList<unsigned int> atomicNums;
      Eigen::Matrix3d cellMatrix;

      prepareMolecule(mol, cell, &fcoords, &atomicNums, &cellMatrix);

      unsigned int spg =
        reduceToPrimitive(&fcoords, &atomicNums,
                          &cellMatrix, cartTol);

      // Save some time if no change
      if (spg <= 0 || spg > 230) {
        return 0;
      }

      applyToMolecule(mol, cell, fcoords, atomicNums, cellMatrix);

      cell->SetSpaceGroup(spg);

      return spg;
    }

    unsigned int refineCrystal(QList<Eigen::Vector3d> *fcoords,
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

      // Build position list. Include space for 4*numAtoms for the
      // cell refinement
      double (*positions)[3] = new double[4*numAtoms][3];
      int *types = new int[4*numAtoms];
      const Eigen::Vector3d * fracCoord;
      for (int i = 0; i < numAtoms; ++i) {
        fracCoord         = &(*fcoords)[i];
        types[i]          = (*atomicNums)[i];
        positions[i][0]   = fracCoord->x();
        positions[i][1]   = fracCoord->y();
        positions[i][2]   = fracCoord->z();
      }

      // find spacegroup for return value
      char symbol[21];
      int spg = spg_get_international(symbol,
                                      lattice,
                                      positions,
                                      types,
                                      numAtoms,
                                      cartTol);
      SPG_DUMP_ATOMS("Original cell", numAtoms);

      // Refine the structure
      int numBravaisAtoms =
        spg_refine_cell(lattice, positions, types,
                        numAtoms, cartTol);
      SPG_DUMP_ATOMS("Bravais cell", numBravaisAtoms);

      // if spglib cannot refine the cell, return 0.
      if (numBravaisAtoms <= 0) {
        return 0;
      }

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
      while (fcoords->size() > numBravaisAtoms) {
        fcoords->removeLast();
        atomicNums->removeLast();
      }
      while (fcoords->size() < numBravaisAtoms) {
        fcoords->append(Eigen::Vector3d());
        atomicNums->append(0);
      }

      // Update
      Q_ASSERT(fcoords->size() == atomicNums->size());
      Q_ASSERT(fcoords->size() == numBravaisAtoms);
      for (int i = 0; i < numBravaisAtoms; ++i) {
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

    unsigned int refineCrystal(QList<Eigen::Vector3d> *fcoords,
                               QStringList *ids,
                               Eigen::Matrix3d *cellMatrix,
                               const double cartTol)
    {
      QList<unsigned int> atomicNums = symbolsToAtomicNumbers(*ids);

      unsigned int spg =
        refineCrystal(fcoords, &atomicNums,
                      cellMatrix, cartTol);

      // Save some time if spg couldn't be found.
      if (spg <= 0 || spg > 230) {
        return 0;
      }

      Q_ASSERT(fcoords->size() == atomicNums.size());

      *ids = atomicNumbersToSymbols(atomicNums);

      return spg;
    }

    unsigned int refineCrystal(Molecule * mol,
                               OpenBabel::OBUnitCell * cell,
                               const double cartTol)
    {
      Q_ASSERT(mol);
      if (!cell) {
        cell = mol->OBUnitCell();
      }
      Q_ASSERT(cell);

      QList<Eigen::Vector3d> fcoords;
      QList<unsigned int> atomicNums;
      Eigen::Matrix3d cellMatrix;

      prepareMolecule(mol, cell, &fcoords, &atomicNums, &cellMatrix);

      unsigned int spg =
        refineCrystal(&fcoords, &atomicNums,
                      &cellMatrix, cartTol);

      // Save some time if no change
      if (spg <= 0 || spg > 230) {
        return 0;
      }

      applyToMolecule(mol, cell, fcoords, atomicNums, cellMatrix);

      cell->SetSpaceGroup(spg);

      return spg;
    }
  }
}
