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
using Eigen::MatrixXd;

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

    // Set up the points we want to calculate the density at
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

  bool BasisSet::calculateCubeDensity(Cube *cube)
  {
    // FIXME Still not working, committed so others could see current state.

    // Must be called before calculations begin
    initCalculation();

    // Set up the points we want to calculate the density at
    m_basisShells = new QVector<BasisShell>(cube->data()->size());

    for (int i = 0; i < m_basisShells->size(); ++i) {
      (*m_basisShells)[i].set = this;
      (*m_basisShells)[i].tCube = cube;
      (*m_basisShells)[i].pos = i;
    }

    // Lock the cube until we are done.
    cube->lock()->lockForWrite();

    // Watch for the future
    connect(&m_watcher, SIGNAL(finished()), this, SLOT(calculationComplete()));

    // The main part of the mapped reduced function...
    m_future = QtConcurrent::map(*m_basisShells, BasisSet::processDensity);
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
    if (val > -1e-20 && val < 1e-20)
      return true;
    else
      return false;
  }

  void BasisSet::initCalculation()
  {
    if (m_init)
      return;
    // This currently just involves normalising all contraction coefficients
    m_numAtoms = m_atomPos.size();
    m_gtoCN.clear();

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
          // Normalization of the S-type orbitals (normalization used in JMol)
          // (8 * alpha^3 / pi^3)^0.25 * exp(-alpha * r^2)
          for(unsigned j = m_gtoIndices[i]; j < m_gtoIndices[i+1]; ++j) {
            m_gtoCN.push_back(m_gtoC[j] * pow(m_gtoA[j], 0.75) * 0.71270547);
          }
          break;
        case P:
          m_moIndices[i] = indexMO;
          indexMO += 3;
          m_cIndices.push_back(m_gtoCN.size());
          // Normalization of the P-type orbitals (normalization used in JMol)
          // (128 alpha^5 / pi^3)^0.25 * [x|y|z]exp(-alpha * r^2)
          for(unsigned j = m_gtoIndices[i]; j < m_gtoIndices[i+1]; ++j) {
            m_gtoCN.push_back(m_gtoC[j] * pow(m_gtoA[j], 1.25) * 1.425410941);
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
          // Normalization of the P-type orbitals (normalization used in JMol)
          // xx|yy|zz: (2048 alpha^7/9pi^3)^0.25 [xx|yy|zz]exp(-alpha r^2)
          // xy|xz|yz: (2048 alpha^7/pi^3)^0.25 [xy|xz|yz]exp(-alpha r^2)
          for(unsigned j = m_gtoIndices[i]; j < m_gtoIndices[i+1]; ++j) {
            m_gtoCN.push_back(m_gtoC[j] * pow(m_gtoA[j], 1.75) * 1.645922781);
            m_gtoCN.push_back(m_gtoCN.back());
            m_gtoCN.push_back(m_gtoCN.back());

            m_gtoCN.push_back(m_gtoC[j] * pow(m_gtoA[j], 1.75) * 2.850821881);
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
          qDebug() << "Basis set not handled - results may be incorrect.";
      }
    }
    m_init = true;
    outputAll();
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
          ;
      }
    }
    // Set the value
    shell.tCube->setValue(shell.pos, tmp);
  }

  void BasisSet::processDensity(BasisShell &shell)
  {
    BasisSet *set = shell.set;
    unsigned int atomsSize = set->m_numAtoms;
    unsigned int basisSize = set->m_symmetry.size();
    unsigned int matrixSize = set->m_density.rows();
    std::vector<int> &basis = set->m_symmetry;
    vector<Vector3d> deltas;
    vector<double> dr2;
    deltas.reserve(atomsSize);
    dr2.reserve(atomsSize);

    // Calculate our position
    Vector3d pos = shell.tCube->position(shell.pos) * ANGSTROM_TO_BOHR;
    // Calculate the deltas for the position
    for (unsigned int i = 0; i < atomsSize; ++i) {
      deltas.push_back(pos - set->m_atomPos[i]);
      dr2.push_back(deltas[i].squaredNorm());
    }

    // Calculate the basis set values at this point
    MatrixXd values(matrixSize, 1);
    for (unsigned int i = 0; i < basisSize; ++i) {
      unsigned int cAtom = set->m_atomIndices[i];
      switch(basis[i]) {
        case S:
          pointS(shell.set, dr2[cAtom], i, values);
          break;
        case P:
          pointP(shell.set, deltas[cAtom], dr2[cAtom], i, values);
          break;
        case D:
          pointD(shell.set, deltas[cAtom], dr2[cAtom], i, values);
          break;
        case D5:
          pointD5(shell.set, deltas[cAtom], dr2[cAtom], i, values);
          break;
        default:
          // Not handled - return a zero contribution
          ;
      }
    }

    // Now calculate the value of the density at this point in space
    double rho = 0.0;
    for (unsigned int i = 0; i < matrixSize; ++i) {
      // Calculate the off-diagonal parts of the matrix
      for (unsigned int j = 0; j < i; ++j) {
        rho += 2.0 * set->m_density.coeffRef(i, j)
             * (values.coeffRef(i, 0) * values.coeffRef(j, 0));
      }
      // Now calculate the matrix diagonal
      rho += set->m_density.coeffRef(i, i)
           * (values.coeffRef(i, 0) * values.coeffRef(i, 0));
    }

    // Set the value
    shell.tCube->setValue(shell.pos, rho);
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
    double x = 0.0, y = 0.0, z = 0.0;

    // Now iterate through the P type GTOs and sum their contributions
    unsigned int cIndex = set->m_cIndices[moIndex];
    for (unsigned int i = set->m_gtoIndices[moIndex];
         i < set->m_gtoIndices[moIndex+1]; ++i) {
      double tmpGTO = exp(-set->m_gtoA[i] * dr2);
      x += set->m_gtoCN[cIndex++] * delta.x() * tmpGTO;
      y += set->m_gtoCN[cIndex++] * delta.y() * tmpGTO;
      z += set->m_gtoCN[cIndex++] * delta.z() * tmpGTO;
    }

    // Calculate the prefactors for Px, Py and Pz
    double Px = set->m_moMatrix.coeffRef(baseIndex  , indexMO);
    double Py = set->m_moMatrix.coeffRef(baseIndex+1, indexMO);
    double Pz = set->m_moMatrix.coeffRef(baseIndex+2, indexMO);

    return Px*x + Py*y + Pz*z;
  }

  inline double BasisSet::pointD(BasisSet *set, unsigned int moIndex,
                       const Vector3d &delta,
                       const double &dr2, unsigned int indexMO)
  {
    // D type orbitals have six components and each component has a different
    // independent MO weighting. Many things can be cached to save time though
    unsigned int baseIndex = set->m_moIndices[moIndex];
    double xx = 0.0, yy = 0.0, zz = 0.0, xy = 0.0, xz = 0.0, yz = 0.0;

    // Now iterate through the D type GTOs and sum their contributions
    unsigned int cIndex = set->m_cIndices[moIndex];
    for (unsigned int i = set->m_gtoIndices[moIndex];
         i < set->m_gtoIndices[moIndex+1]; ++i) {
      // Calculate the common factor
      double tmpGTO = exp(-set->m_gtoA[i] * dr2);
      xx += set->m_gtoCN[cIndex++] * tmpGTO; // Dxx
      yy += set->m_gtoCN[cIndex++] * tmpGTO; // Dyy
      zz += set->m_gtoCN[cIndex++] * tmpGTO; // Dzz
      xy += set->m_gtoCN[cIndex++] * tmpGTO; // Dxy
      xz += set->m_gtoCN[cIndex++] * tmpGTO; // Dxz
      yz += set->m_gtoCN[cIndex++] * tmpGTO; // Dyz
    }

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
    return Dxx*xx + Dyy*yy + Dzz*zz + Dxy*xy + Dxz*xz + Dyz*yz;
  }

  inline double BasisSet::pointD5(BasisSet *set, unsigned int moIndex,
                        const Vector3d &delta,
                        const double &dr2, unsigned int indexMO)
  {
    // D type orbitals have five components and each component has a different
    // MO weighting. Many things can be cached to save time
    unsigned int baseIndex = set->m_moIndices[moIndex];
    double d0 = 0.0, d1p = 0.0, d1n = 0.0, d2p = 0.0, d2n = 0.0;

    // Now iterate through the D type GTOs and sum their contributions
    unsigned int cIndex = set->m_cIndices[moIndex];
    for (unsigned int i = set->m_gtoIndices[moIndex];
         i < set->m_gtoIndices[moIndex+1]; ++i) {
      // Calculate the common factor
      double tmpGTO = exp(-set->m_gtoA[i] * dr2);
      d0  += set->m_gtoCN[cIndex++] * tmpGTO;
      d1p += set->m_gtoCN[cIndex++] * tmpGTO;
      d1n += set->m_gtoCN[cIndex++] * tmpGTO;
      d2p += set->m_gtoCN[cIndex++] * tmpGTO;
      d2n += set->m_gtoCN[cIndex++] * tmpGTO;
    }

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

    return D0*d0 + D1p*d1p + D1n*d1n + D2p*d2p + D2n*d2n;
  }

  inline void BasisSet::pointS(BasisSet *set, const double &dr2, int basis,
                               Eigen::MatrixXd &out)
  {
    // S type orbitals - the simplest of the calculations with one component
    double tmp = 0.0;
    unsigned int cIndex = set->m_cIndices[basis];
    for (unsigned int i = set->m_gtoIndices[basis];
         i < set->m_gtoIndices[basis+1]; ++i) {
      tmp += set->m_gtoCN[cIndex++] * exp(-set->m_gtoA[i] * dr2);
    }
    out.coeffRef(set->m_moIndices[basis], 0) = tmp;
  }

  inline void BasisSet::pointP(BasisSet *set, const Vector3d &delta,
                               const double &dr2, int basis,
                               Eigen::MatrixXd &out)
  {
    double x = 0.0, y = 0.0, z = 0.0;

    // Now iterate through the P type GTOs and sum their contributions
    unsigned int cIndex = set->m_cIndices[basis];
    for (unsigned int i = set->m_gtoIndices[basis];
         i < set->m_gtoIndices[basis+1]; ++i) {
      double tmpGTO = exp(-set->m_gtoA[i] * dr2);
      x += set->m_gtoCN[cIndex++] * tmpGTO;
      y += set->m_gtoCN[cIndex++] * tmpGTO;
      z += set->m_gtoCN[cIndex++] * tmpGTO;
    }

    // Save values to the matrix
    int baseIndex = set->m_moIndices[basis];
    out.coeffRef(baseIndex  , 0) = x * delta.x();
    out.coeffRef(baseIndex+1, 0) = y * delta.y();
    out.coeffRef(baseIndex+2, 0) = z * delta.z();
  }

  inline void BasisSet::pointD(BasisSet *set, const Eigen::Vector3d &delta,
                               const double &dr2, int basis,
                               Eigen::MatrixXd &out)
  {
    // D type orbitals have six components and each component has a different
    // independent MO weighting. Many things can be cached to save time though
    double xx = 0.0, yy = 0.0, zz = 0.0, xy = 0.0, xz = 0.0, yz = 0.0;

    // Now iterate through the D type GTOs and sum their contributions
    unsigned int cIndex = set->m_cIndices[basis];
    for (unsigned int i = set->m_gtoIndices[basis];
         i < set->m_gtoIndices[basis+1]; ++i) {
      // Calculate the common factor
      double tmpGTO = exp(-set->m_gtoA[i] * dr2);
      xx += set->m_gtoCN[cIndex++] * tmpGTO; // Dxx
      yy += set->m_gtoCN[cIndex++] * tmpGTO; // Dyy
      zz += set->m_gtoCN[cIndex++] * tmpGTO; // Dzz
      xy += set->m_gtoCN[cIndex++] * tmpGTO; // Dxy
      xz += set->m_gtoCN[cIndex++] * tmpGTO; // Dxz
      yz += set->m_gtoCN[cIndex++] * tmpGTO; // Dyz
    }

    // Save values to the matrix
    int baseIndex = set->m_moIndices[basis];
    out.coeffRef(baseIndex  , 0) = delta.x() * delta.x() * xx;
    out.coeffRef(baseIndex+1, 0) = delta.y() * delta.y() * yy;
    out.coeffRef(baseIndex+2, 0) = delta.z() * delta.z() * zz;
    out.coeffRef(baseIndex+3, 0) = delta.x() * delta.y() * xy;
    out.coeffRef(baseIndex+4, 0) = delta.x() * delta.z() * xz;
    out.coeffRef(baseIndex+5, 0) = delta.y() * delta.z() * yz;
  }

  inline void BasisSet::pointD5(BasisSet *set, const Eigen::Vector3d &delta,
                                const double &dr2, int basis,
                                Eigen::MatrixXd &out)
  {
    // D type orbitals have six components and each component has a different
    // independent MO weighting. Many things can be cached to save time though
    double d0 = 0.0, d1p = 0.0, d1n = 0.0, d2p = 0.0, d2n = 0.0;

    // Now iterate through the D type GTOs and sum their contributions
    unsigned int cIndex = set->m_cIndices[basis];
    for (unsigned int i = set->m_gtoIndices[basis];
         i < set->m_gtoIndices[basis+1]; ++i) {
      // Calculate the common factor
      double tmpGTO = exp(-set->m_gtoA[i] * dr2);
      d0  += set->m_gtoCN[cIndex++] * tmpGTO;
      d1p += set->m_gtoCN[cIndex++] * tmpGTO;
      d1n += set->m_gtoCN[cIndex++] * tmpGTO;
      d2p += set->m_gtoCN[cIndex++] * tmpGTO;
      d2n += set->m_gtoCN[cIndex++] * tmpGTO;
    }

    // Calculate the prefactors
    double xx = delta.x() * delta.x();
    double yy = delta.y() * delta.y();
    double zz = delta.z() * delta.z();
    double xy = delta.x() * delta.y();
    double xz = delta.x() * delta.z();
    double yz = delta.y() * delta.z();

    // Save values to the matrix
    int baseIndex = set->m_moIndices[basis];
    out.coeffRef(baseIndex  , 0) = (zz - dr2) * d0;
    out.coeffRef(baseIndex+1, 0) = xz * d1p;
    out.coeffRef(baseIndex+2, 0) = yz * d1n;
    out.coeffRef(baseIndex+3, 0) = (xx - yy) * d2p;
    out.coeffRef(baseIndex+4, 0) = xy * d2n;
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
    // Can be called to print out a summary of the basis set as read in
    qDebug() << "\nGaussian Basis Set\nNumber of atoms:" << m_numAtoms;
    for (uint i = 0; i < m_symmetry.size(); ++i) {
      qDebug() << i
          << "\tAtom Index:" << m_atomIndices[i]
          << "\tSymmetry:" << m_symmetry[i]
          << "\tMO Index:" << m_moIndices[i]
          << "\tGTO Index:" << m_gtoIndices[i];
    }
    qDebug() << m_symmetry.size() << m_gtoIndices.size()
        << m_gtoIndices[m_symmetry.size()];
    for (uint i = 0; i < m_symmetry.size(); ++i) {
      switch(m_symmetry[i]) {
        case S:
          qDebug() << "Shell" << i << "\tS\n  MO 1\t"
              << m_moMatrix(0, m_moIndices[i])
              << m_moMatrix(m_moIndices[i], 0);
          break;
        case P:
          qDebug() << "Shell" << i << "\tP\n  MO 1\t"
              << m_moMatrix(0, m_moIndices[i])
              << "\t" << m_moMatrix(0, m_moIndices[i+1])
              << "\t" << m_moMatrix(0, m_moIndices[i+2]);
          break;
        case D:
          qDebug() << "Shell" << i << "\tD\n  MO 1\t"
              << m_moMatrix(0, m_moIndices[i])
              << "\t" << m_moMatrix(0, m_moIndices[i+1])
              << "\t" << m_moMatrix(0, m_moIndices[i+2])
              << "\t" << m_moMatrix(0, m_moIndices[i+3])
              << "\t" << m_moMatrix(0, m_moIndices[i+4])
              << "\t" << m_moMatrix(0, m_moIndices[i+5]);
          break;
        case D5:
          qDebug() << "Shell" << i << "\tD\n  MO 1\t"
              << m_moMatrix(0, m_moIndices[i])
              << "\t" << m_moMatrix(0, m_moIndices[i+1])
              << "\t" << m_moMatrix(0, m_moIndices[i+2])
              << "\t" << m_moMatrix(0, m_moIndices[i+3])
              << "\t" << m_moMatrix(0, m_moIndices[i+4]);
          break;
        default:
          qDebug() << "Error: unhandled type...";
      }
      unsigned int cIndex = m_gtoIndices[i];
      for (uint j = m_gtoIndices[i]; j < m_gtoIndices[i+1]; ++j) {
        qDebug() << cIndex
          << "\tc:" << m_gtoC[cIndex]
          << "\ta:" << m_gtoA[cIndex];
        ++cIndex;
      }
    }
    qDebug() << "\n";
  }

}

