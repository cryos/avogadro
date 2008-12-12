/**********************************************************************
  SlaterSet - Slater basis sets

  Copyright (C) 2008 Marcus D. Hanwell

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.sourceforge.net/>

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

#include "slaterset.h"

#include <avogadro/molecule.h>
#include <avogadro/atom.h>
#include <avogadro/cube.h>

#include <Eigen/Array>
#include <Eigen/LU>
#include <Eigen/QR>

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

using namespace Eigen;

namespace Avogadro
{
  struct SlaterShell
  {
    SlaterSet *set;     // A pointer to the SlaterSet, cannot write to member vars
    Cube *cube;       // The target cube, used to initialise temp cubes too
    unsigned int pos;  // The index of position of the point to calculate the MO for
    unsigned int state;// The MO number to calculate
  };

  using std::vector;

  static const double BOHR_TO_ANGSTROM = 0.529177249;
  static const double ANGSTROM_TO_BOHR = 1.0 / 0.529177249;

  SlaterSet::SlaterSet()
  {
  }

  SlaterSet::~SlaterSet()
  {

  }

  bool SlaterSet::addAtoms(const std::vector<Eigen::Vector3d> &pos)
  {
    m_atomPos = pos;
    return true;
  }

  bool SlaterSet::addSlaterIndices(const std::vector<int> &i)
  {
    m_slaterIndices = i;
    return true;
  }

  bool SlaterSet::addSlaterTypes(const std::vector<int> &t)
  {
    m_slaterTypes = t;
    return true;
  }

  bool SlaterSet::addZetas(const std::vector<double> &zetas)
  {
    m_zetas = zetas;
    return true;
  }

  bool SlaterSet::addPQNs(const std::vector<double> &pqns)
  {
    m_pqns = pqns;
    return true;
  }

  bool SlaterSet::addOverlapMatrix(const Eigen::MatrixXd &m)
  {
    m_overlap.resize(m.rows(), m.cols());
    m_overlap = m;
    return true;
  }

  bool SlaterSet::addEigenVectors(const Eigen::MatrixXd &e)
  {
    m_eigenVectors.resize(e.rows(), e.cols());
    m_eigenVectors = e;
    return true;
  }

  unsigned int SlaterSet::numMOs()
  {
    return m_overlap.cols();
  }

  inline bool SlaterSet::isSmall(double val)
  {
    if (val > -1e-15 && val < 1e-15)
      return true;
    else
      return false;
  }

  unsigned int SlaterSet::factorial(unsigned int n)
  {
    if (n <= 1)
      return n;
    return (n * factorial(n-1));
  }

  void SlaterSet::outputAll()
  {

  }

  bool SlaterSet::calculateCubeMO(Cube *cube, unsigned int state)
  {
    // Set up the calculation and ideally use the new QtConcurrent code to
    // multithread the calculation...
    if (state < 1 || static_cast<int>(state) > m_overlap.rows())
      return false;

    if (!m_initialized)
      initialize();

    // It is more efficient to process each shell over the entire cube than it
    // is to process each MO at each point in the cube. This is probably the best
    // point at which to multithread too - QtConcurrent!
    m_slaterShells.resize(cube->data()->size());

    qDebug() << "Number of points:" << m_slaterShells.size();

    for (int i = 0; i < m_slaterShells.size(); ++i) {
      m_slaterShells[i].set = this;
      m_slaterShells[i].cube = cube;
      m_slaterShells[i].pos = i;
      m_slaterShells[i].state = state;
    }

    // Lock the cube until we are done.
    cube->lock()->lockForWrite();

    // Watch for the future
    connect(&m_watcher, SIGNAL(finished()), this, SLOT(calculationComplete()));

    // The main part of the mapped reduced function...
    m_future = QtConcurrent::map(m_slaterShells, SlaterSet::processPoint);
    // Connect our watcher to our future
    m_watcher.setFuture(m_future);

    return true;
  }

  void SlaterSet::calculationComplete()
  {
    disconnect(&m_watcher, SIGNAL(finished()), this, SLOT(calculationComplete()));
    qDebug() << m_slaterShells[0].cube->data()->at(0) << m_slaterShells[0].cube->data()->at(1);
    qDebug() << "Calculation complete - cube map...";
    m_slaterShells[0].cube->lock()->unlock();
    m_slaterShells.clear();
  }

  bool SlaterSet::initialize()
  {
    // Convert the exponents into Angstroms
    for (unsigned int i = 0; i < m_zetas.size(); ++i)
      m_zetas[i] = m_zetas[i] / BOHR_TO_ANGSTROM;
  
    m_normalized.resize(m_overlap.cols(), m_overlap.rows());

    SelfAdjointEigenSolver<MatrixXd> s(m_overlap);
    MatrixXd p = s.eigenvectors();
    MatrixXd m = p * s.eigenvalues().cwise().inverse().cwise().sqrt().asDiagonal() * p.inverse();
    m_normalized = m * m_eigenVectors;

    if (!(m_overlap*m*m).isIdentity())
      qDebug() << "Identity test failed - do you need a newer version of Eigen?";

    m_factors.resize(m_zetas.size());
    // Calculate the normalizations of the orbitals
    for (unsigned int i = 0; i < m_zetas.size(); ++i) {
      switch (m_slaterTypes[i]) {
        case S:
          m_factors[i] = pow(2.0 * m_zetas[i], m_pqns[i] + 0.5) *
                         sqrt(1.0 / (4.0*M_PI) / factorial(2*m_pqns[i]));
          break;
        case PX:
        case PY:
        case PZ:
          m_factors[i] = pow(2.0 * m_zetas[i], m_pqns[i] + 0.5) *
                         sqrt(3.0 / (4.0*M_PI) / factorial(2*m_pqns[i]));
          break;
        default:
         ;
      }
    }
    m_initialized = true;
    return true;
  }

  void SlaterSet::processPoint(SlaterShell &shell)
  {
    SlaterSet *set = shell.set;
    unsigned int atomsSize = set->m_atomPos.size();
    unsigned int basisSize = set->m_zetas.size();

    vector<Vector3d> deltas;
    vector<double> dr;
    deltas.reserve(atomsSize);
    dr.reserve(atomsSize);

    // Simply the row of the matrix to operate on
    unsigned int indexMO = shell.state - 1;

    // Calculate our position
    Vector3d pos = shell.cube->position(shell.pos);// * ANGSTROM_TO_BOHR;

    // Calculate the deltas for the position
    for (unsigned int i = 0; i < atomsSize; ++i) {
      deltas.push_back(pos - set->m_atomPos[i]);
      dr.push_back(deltas[i].norm());
    }

    // Now calculate the value at this point in space
    double tmp = 0.0;
    for (unsigned int i = 0; i < basisSize; ++i) {
      tmp += pointSlater(shell.set, deltas[set->m_slaterIndices[i]],
                        dr[set->m_slaterIndices[i]], i, indexMO);
    }
    // Set the value
    shell.cube->setValue(shell.pos, tmp);
  }

  inline double SlaterSet::pointSlater(SlaterSet *set, const Eigen::Vector3d &delta,
                      const double &dr, unsigned int slater, unsigned int indexMO)
  {
    double tmp = set->m_normalized(slater, indexMO) *
                 set->m_factors[slater] * exp(- set->m_zetas[slater] * dr);
    switch (set->m_slaterTypes[slater]) {
      case S:
        for (int i = 0; i <= set->m_pqns[slater]-1; ++i)
          tmp *= dr;
        break;
      case PX:
        for (int i = 0; i <= set->m_pqns[slater]-2; ++i)
          tmp *= dr;
        tmp *= delta.x();
        break;
      case PY:
        for (int i = 0; i <= set->m_pqns[slater]-2; ++i)
          tmp *= dr;
        tmp *= delta.y();
        break;
      case PZ:
        for (int i = 0; i <= set->m_pqns[slater]-2; ++i)
          tmp *= dr;
        tmp *= delta.z();
        break;
      default:
        return 0.0;
    }
    return tmp;
  }

}

#include "slaterset.moc"
