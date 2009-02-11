/**********************************************************************
  BasisSet - encapsulation of Gaussian type basis sets

  Copyright (C) 2008-2009 Marcus D. Hanwell
  Copyright (C) 2008 Albert De Fusco

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.openmolecules.net/>

  This library is free software; you can redistribute it and/or modify
  it under the terms of the GNU Library General Public License as
  published by the Free Software Foundation; either version 2.1 of the
  License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.
 **********************************************************************/

#include "basisset.h"

#ifdef WIN32
#define _USE_MATH_DEFINES
#include <math.h> // needed for M_PI
#endif

#include <avogadro/molecule.h>
#include <avogadro/atom.h>
#include <avogadro/cube.h>

#include <cmath>

#include <QtConcurrentMap>
#include <QFuture>
#include <QFutureWatcher>
#include <QReadWriteLock>
#include <QDebug>

using std::vector;
using Eigen::Vector3d;
using Eigen::Vector3i;

namespace Avogadro
{
  struct BasisShell
  {
    BasisSet *set;     // A pointer to the BasisSet, cannot write to member vars
    Cube *tCube;       // The target cube, used to initialise temp cubes too
    unsigned int pos;  // The index ofposition of the point to calculate the MO for
    unsigned int state;// The MO number to calculate
  };

  static const double BOHR_TO_ANGSTROM = 0.529177249;
  static const double ANGSTROM_TO_BOHR = 1.0 / 0.529177249;

  BasisSet::BasisSet() : m_numMOs(0), m_electrons(0), m_init(false)
  {
  }

  BasisSet::~BasisSet()
  {
  }

  unsigned int BasisSet::addAtom(const Vector3d& pos, int)
  {
    m_init = false;
    // Add to the new data structure, delete the old soon
    m_atomPos.push_back(pos);

    return m_atomPos.size() - 1;
  }

  unsigned int BasisSet::addBasis(unsigned int atom, orbital type)
  {
    // Count the number of independent basis functions
    switch (type) {
      case S:
        m_numMOs++;
        break;
      case P:
        m_numMOs += 3;
        break;
      case SP:
        m_numMOs += 4;
        break;
      case D:
        m_numMOs += 6;
        break;
      case D5:
        m_numMOs += 5;
        break;
      default:
        // Should never hit here
        ;
    }
    m_init = false;

    // Add to the new data structure, delete the old soon
    m_symmetry.push_back(type);
    m_atomIndices.push_back(atom);
    return m_symmetry.size() - 1;
  }

  unsigned int BasisSet::addGTO(unsigned int, double c, double a)
  {
    // Use the new data structure
    if (m_gtoIndices.size() < m_atomIndices.size()) {
      // First GTO added for this basis - add the gto index
      m_gtoIndices.push_back(m_gtoA.size());
    }
    m_gtoA.push_back(a);
    m_gtoC.push_back(c);

    return m_gtoA.size() - 1;
  }

  void BasisSet::addMOs(const vector<double>& MOs)
  {
    m_init = false;

    // The new way - initalise a matrix and read the data into it
    m_moMatrix.resize(m_numMOs, m_numMOs);
    // Now read in the vector row by row
    for (unsigned int i = 0; i < m_numMOs; ++i)
      for (unsigned int j = 0; j < m_numMOs; ++j)
        m_moMatrix.coeffRef(i, j) = MOs[i + j*m_numMOs];
  }

  void BasisSet::addMO(double)
  {
    m_init = false;
  }

  bool BasisSet::setDensityMatrix(const Eigen::MatrixXd &m)
  {
    m_density.resize(m.rows(), m.cols());
    m_density = m;
    return true;
  }

  bool BasisSet::calculateCubeMO(Cube *cube, int state)
  {
    // Set up the calculation and ideally use the new QtConcurrent code to
    // multithread the calculation...
    if (state < 1 || state > m_moMatrix.rows())
      return false;

    // Must be called before calculations begin
    initCalculation();

    // It is more efficient to process each shell over the entire cube than it
    // is to process each MO at each point in the cube. This is probably the best
    // point at which to multithread too - QtConcurrent!
    m_basisShells = new QVector<BasisShell>(cube->data()->size());

    for (int i = 0; i < m_basisShells->size(); ++i) {
      (*m_basisShells)[i].set = this;
      (*m_basisShells)[i].tCube = cube;
      (*m_basisShells)[i].pos = i;
      (*m_basisShells)[i].state = state;
    }

    // Lock the cube until we are done.
    cube->lock()->lockForWrite();

    // Watch for the future
    connect(&m_watcher, SIGNAL(finished()), this, SLOT(calculationComplete()));

    // The main part of the mapped reduced function...
    m_future = QtConcurrent::map(*m_basisShells, BasisSet::processPoint);
    // Connect our watcher to our future
    m_watcher.setFuture(m_future);

    return true;
  }

  void BasisSet::calculationComplete()
  {
    disconnect(&m_watcher, SIGNAL(finished()), this, SLOT(calculationComplete()));
    qDebug() << (*m_basisShells)[0].tCube->data()->at(0) << (*m_basisShells)[0].tCube->data()->at(1);
    (*m_basisShells)[0].tCube->lock()->unlock();
    delete m_basisShells;
  }

  inline bool BasisSet::isSmall(double val)
  {
    if (val > -1e-15 && val < 1e-15)
      return true;
    else
      return false;
  }

  void BasisSet::initCalculation()
  {
    // This currently just involves normalising all contraction coefficients
    m_numAtoms = m_atomPos.size();

    // Initialise the new data structures that are hopefully more efficient
    unsigned int indexMO = 0;
    m_moIndices.resize(m_symmetry.size());
    // Add a final entry to the gtoIndices
    m_gtoIndices.push_back(m_gtoA.size());
    for(unsigned int i = 0; i < m_symmetry.size(); ++i) {
      switch (m_symmetry[i]) {
        case S:
          m_moIndices[i] = indexMO++;
          m_cIndices.push_back(m_gtoCN.size());
          for(unsigned j = m_gtoIndices[i]; j < m_gtoIndices[i+1]; ++j) {
            m_gtoCN.push_back(m_gtoC[j] * pow(2.0 * m_gtoA[j] / M_PI, 0.75));
          }
          break;
        case P:
          m_moIndices[i] = indexMO;
          indexMO += 3;
          m_cIndices.push_back(m_gtoCN.size());
          for(unsigned j = m_gtoIndices[i]; j < m_gtoIndices[i+1]; ++j) {
            m_gtoCN.push_back(m_gtoC[j] * pow(128.0 * pow(m_gtoA[j], 5.0)
                            / (M_PI*M_PI*M_PI), 0.25));
            m_gtoCN.push_back(m_gtoCN.back());
            m_gtoCN.push_back(m_gtoCN.back());
          }
          break;
        case D:
          // Cartesian - 6 d components
          // Order in xx, yy, zz, xy, xz, yz
          m_moIndices[i] = indexMO;
          indexMO += 6;
          m_cIndices.push_back(m_gtoCN.size());
          for(unsigned j = m_gtoIndices[i]; j < m_gtoIndices[i+1]; ++j) {
            m_gtoCN.push_back(m_gtoC[j] * pow(2048.0 * pow(m_gtoA[j], 7.0)
                          / (9.0 * M_PI*M_PI*M_PI), 0.25));
            m_gtoCN.push_back(m_gtoCN.back());
            m_gtoCN.push_back(m_gtoCN.back());

            m_gtoCN.push_back(m_gtoC[j] * pow(2048.0 * pow(m_gtoA[j], 7.0)
                          / (M_PI*M_PI*M_PI), 0.25));
            m_gtoCN.push_back(m_gtoCN.back());
            m_gtoCN.push_back(m_gtoCN.back());
          }
          break;
        case D5:
          // Spherical - 5 d components
          // Order in d0, d+1, d-1, d+2, d-2
          // Form d(z^2-r^2), dxz, dyz, d(x^2-y^2), dxy
          m_moIndices[i] = indexMO;
          indexMO += 5;
          m_cIndices.push_back(m_gtoCN.size());
          for(unsigned j = m_gtoIndices[i]; j < m_gtoIndices[i+1]; ++j) {
            m_gtoCN.push_back(m_gtoC[j] * pow(2048.0 * pow(m_gtoA[j], 7.0)
                          / (9.0 * M_PI*M_PI*M_PI), 0.25));
            m_gtoCN.push_back(m_gtoC[j] * pow(2048.0 * pow(m_gtoA[j], 7.0)
                          / (M_PI*M_PI*M_PI), 0.25));
            m_gtoCN.push_back(m_gtoCN.back());
            // I think this is correct but reaally need to check...
            m_gtoCN.push_back(m_gtoC[j] * pow(128.0 * pow(m_gtoA[j], 7.0)
                          / (M_PI*M_PI*M_PI), 0.25));
            m_gtoCN.push_back(m_gtoC[j] * pow(2048.0 * pow(m_gtoA[j], 7.0)
                          / (M_PI*M_PI*M_PI), 0.25));
          }
          break;
        default:
          ;
      }
    }
  }

  /// This is the stuff we actually use right now - porting to new data structure
  void BasisSet::processPoint(BasisShell &shell)
  {
    BasisSet *set = shell.set;
    unsigned int atomsSize = set->m_numAtoms;
    unsigned int basisSize = set->m_symmetry.size();
    std::vector<int> &basis = set->m_symmetry;
    vector<Vector3d> deltas;
    vector<double> dr2;
    deltas.reserve(atomsSize);
    dr2.reserve(atomsSize);

    unsigned int indexMO = shell.state-1;

    // Calculate our position
    Vector3d pos = shell.tCube->position(shell.pos) * ANGSTROM_TO_BOHR;

    // Calculate the deltas for the position
    for (unsigned int i = 0; i < atomsSize; ++i) {
      deltas.push_back(pos - set->m_atomPos[i]);
      dr2.push_back(deltas[i].squaredNorm());
    }

    // Now calculate the value at this point in space
    double tmp = 0.0;
    for (unsigned int i = 0; i < basisSize; ++i) {
      switch(basis[i]) {
        case S:
          tmp += pointS(shell.set, i,
                        dr2[set->m_atomIndices[i]], indexMO);
          break;
        case P:
          tmp += pointP(shell.set, i, deltas[set->m_atomIndices[i]],
                        dr2[set->m_atomIndices[i]], indexMO);
          break;
        case D:
          tmp += pointD(shell.set, i, deltas[set->m_atomIndices[i]],
                        dr2[set->m_atomIndices[i]], indexMO);
          break;
        case D5:
          tmp += pointD5(shell.set, i, deltas[set->m_atomIndices[i]],
                         dr2[set->m_atomIndices[i]], indexMO);
          break;
        default:
          // Not handled - return a zero contribution
          qDebug() << "Error: Cannot process basis.";
      }
    }
    // Set the value
    shell.tCube->setValue(shell.pos, tmp);
  }

  inline double BasisSet::pointS(BasisSet *set, unsigned int moIndex,
                       const double &dr2, unsigned int indexMO)
  {
    // If the MO coefficient is very small skip it
    if (isSmall(set->m_moMatrix.coeffRef(set->m_moIndices[moIndex], indexMO))) {
      return 0.0;
    }

    // S type orbitals - the simplest of the calculations with one component
    double tmp = 0.0;
    unsigned int cIndex = set->m_cIndices[moIndex];
    for (unsigned int i = set->m_gtoIndices[moIndex];
         i < set->m_gtoIndices[moIndex+1]; ++i) {
      tmp += set->m_gtoCN[cIndex++] * exp(-set->m_gtoA[i] * dr2);
    }
    // There is one MO coefficient per S shell basis
    return tmp * set->m_moMatrix.coeffRef(set->m_moIndices[moIndex], indexMO);
  }

  inline double BasisSet::pointP(BasisSet *set, unsigned int moIndex,
                       const Vector3d &delta,
                       const double &dr2, unsigned int indexMO)
  {
    // P type orbitals have three components and each component has a different
    // independent MO weighting. Many things can be cached to save time though
    unsigned int baseIndex = set->m_moIndices[moIndex];
    if (isSmall(set->m_moMatrix.coeffRef(baseIndex  , indexMO)) &&
        isSmall(set->m_moMatrix.coeffRef(baseIndex+1, indexMO)) &&
        isSmall(set->m_moMatrix.coeffRef(baseIndex+2, indexMO))) {
      return 0.0;
    }

    double tmp = 0.0;
    // Calculate the prefactors for Px, Py and Pz
    double Px = set->m_moMatrix.coeffRef(baseIndex  , indexMO) * delta.x();
    double Py = set->m_moMatrix.coeffRef(baseIndex+1, indexMO) * delta.y();
    double Pz = set->m_moMatrix.coeffRef(baseIndex+2, indexMO) * delta.z();

    // Now iterate through the P type GTOs and sum their contributions
    unsigned int cIndex = set->m_cIndices[moIndex];
    for (unsigned int i = set->m_gtoIndices[moIndex];
         i < set->m_gtoIndices[moIndex+1]; ++i) {
      double tmpGTO = exp(-set->m_gtoA[i] * dr2);
      tmp += set->m_gtoCN[cIndex++] * Px * tmpGTO;
      tmp += set->m_gtoCN[cIndex++] * Py * tmpGTO;
      tmp += set->m_gtoCN[cIndex++] * Pz * tmpGTO;
    }
    return tmp;
  }

  inline double BasisSet::pointD(BasisSet *set, unsigned int moIndex,
                       const Vector3d &delta,
                       const double &dr2, unsigned int indexMO)
  {
    // D type orbitals have five components and each component has a different
    // independent MO weighting. Many things can be cached to save time though
    unsigned int baseIndex = set->m_moIndices[moIndex];
    double tmp = 0.0;
    // Calculate the prefactors
    double Dxx = set->m_moMatrix.coeffRef(baseIndex  , indexMO) * delta.x()
                 * delta.x();
    double Dyy = set->m_moMatrix.coeffRef(baseIndex+1, indexMO) * delta.y()
                 * delta.y();
    double Dzz = set->m_moMatrix.coeffRef(baseIndex+2, indexMO) * delta.z()
                 * delta.z();
    double Dxy = set->m_moMatrix.coeffRef(baseIndex+3, indexMO) * delta.x()
                 * delta.y();
    double Dxz = set->m_moMatrix.coeffRef(baseIndex+4, indexMO) * delta.x()
                 * delta.z();
    double Dyz = set->m_moMatrix.coeffRef(baseIndex+5, indexMO) * delta.y()
                 * delta.z();

    // Now iterate through the D type GTOs and sum their contributions
    unsigned int cIndex = set->m_cIndices[moIndex];
    for (unsigned int i = set->m_gtoIndices[moIndex];
         i < set->m_gtoIndices[moIndex+1]; ++i) {
      // Calculate the common factor
      double tmpGTO = exp(-set->m_gtoA[i] * dr2);
      tmp += set->m_gtoCN[cIndex++] * Dxx * tmpGTO; // Dxx
      tmp += set->m_gtoCN[cIndex++] * Dyy * tmpGTO; // Dyy
      tmp += set->m_gtoCN[cIndex++] * Dzz * tmpGTO; // Dzz
      tmp += set->m_gtoCN[cIndex++] * Dxy * tmpGTO; // Dxy
      tmp += set->m_gtoCN[cIndex++] * Dxz * tmpGTO; // Dxz
      tmp += set->m_gtoCN[cIndex++] * Dyz * tmpGTO; // Dyz
    }
    return tmp;
  }

  inline double BasisSet::pointD5(BasisSet *set, unsigned int moIndex,
                        const Vector3d &delta,
                        const double &dr2, unsigned int indexMO)
  {
    // D type orbitals have five components and each component has a different
    // MO weighting. Many things can be cached to save time
    unsigned int baseIndex = set->m_moIndices[moIndex];
    double tmp = 0.0;
    // Calculate the prefactors
    double xx = delta.x() * delta.x();
    double yy = delta.y() * delta.y();
    double zz = delta.z() * delta.z();
    double xy = delta.x() * delta.y();
    double xz = delta.x() * delta.z();
    double yz = delta.y() * delta.z();

    double D0  = set->m_moMatrix.coeffRef(baseIndex  , indexMO) * (zz - dr2);
    double D1p = set->m_moMatrix.coeffRef(baseIndex+1, indexMO) * xz;
    double D1n = set->m_moMatrix.coeffRef(baseIndex+2, indexMO) * yz;
    double D2p = set->m_moMatrix.coeffRef(baseIndex+3, indexMO) * (xx - yy);
    double D2n = set->m_moMatrix.coeffRef(baseIndex+4, indexMO) * xy;

    // Not iterate through the GTOs
    unsigned int cIndex = set->m_cIndices[moIndex];
    for (unsigned int i = set->m_gtoIndices[moIndex];
         i < set->m_gtoIndices[moIndex+1]; ++i) {
      double tmpGTO = exp(-set->m_gtoA[i] * dr2);
      tmp += set->m_gtoCN[cIndex++] * D0 * tmpGTO;  // D0
      tmp += set->m_gtoCN[cIndex++] * D1p * tmpGTO; // D1p
      tmp += set->m_gtoCN[cIndex++] * D1n * tmpGTO; // D1n
      tmp += set->m_gtoCN[cIndex++] * D2p * tmpGTO; // D2p
      tmp += set->m_gtoCN[cIndex++] * D2n * tmpGTO; // D2n
    }
    return tmp;
  }

  int BasisSet::numMOs()
  {
    // Return the total number of MOs
    return m_moMatrix.rows();
  }

  void BasisSet::addAtoms(Molecule* mol)
  {
    // Add our atoms to the molecule
    for (unsigned int i = 0; i < m_atomPos.size(); ++i) {
      Atom* atom = mol->addAtom();
      atom->setPos(m_atomPos[i] * BOHR_TO_ANGSTROM);
//      atom->setAtomicNumber(m_atoms.at(i)->num);
    }
  }

  void BasisSet::outputAll()
  {

  }

}

#include "basisset.moc"
