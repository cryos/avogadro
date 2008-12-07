/**********************************************************************
  BasisSet - encapsulation of basis sets

  Copyright (C) 2008 Marcus D. Hanwell
  Copyright (C) 2008 Albert De Fusco

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

#include "basisset.h"

#include <avogadro/molecule.h>
#include <avogadro/atom.h>
#include <avogadro/cube.h>

#include <cmath>

#include <QtConcurrentMap>
#include <QFuture>
#include <QDebug>

using namespace Eigen;
using namespace std;

namespace Avogadro
{
  struct BasisShell
  {
    BasisSet *set;     // A pointer to the BasisSet, cannot write to member vars
    Cube *tCube;       // The target cube, used to initialise temp cubes too
    Cube *cube;        // A pointer to the Cube that will be calculated
    std::vector<double> *vals; // A vector of values
    unsigned int shell;// The shell to calculate
    unsigned int state;// The MO number to calculate
  };

  using std::vector;

  static const double BOHR_TO_ANGSTROM = 0.529177249;
  static const double ANGSTROM_TO_BOHR = 1.0 / 0.529177249;

  BasisSet::BasisSet() : m_MO(0), m_cPos(0), m_numMOs(0), m_numCs(0),
    m_electrons(0), m_init(false)
  {
  }

  BasisSet::~BasisSet()
  {
    for (unsigned int i = 0; i < m_atoms.size(); ++i)
      delete m_atoms.at(i);
    m_atoms.clear();

    for (unsigned int i = 0; i < m_basis.size(); ++i)
    {
      for (unsigned int j = 0; j < m_basis.at(i)->GTOs.size(); ++j)
        delete m_basis.at(i)->GTOs.at(j);
      delete m_basis.at(i);
    }
    m_basis.clear();
  }

  unsigned int BasisSet::addAtom(const Vector3d& pos, int num)
  {
    qDebug() << "New atom added" << num;
    QAtom *tmp = new QAtom;
    tmp->pos = pos;
    tmp->num = num;
    m_atoms.push_back(tmp);
    m_shells.resize(m_atoms.size());
    m_init = false;
    return m_atoms.size() - 1;
  }

  unsigned int BasisSet::addBasis(unsigned int atom, orbital type)
  {
    Basis *tmp = new Basis;
    tmp->atom = atom;
    tmp->index = m_numMOs;
    tmp->type = type;
    m_basis.push_back(tmp);
    m_shells[atom].push_back(tmp);
    // Count the number of independent basis functions
    switch (type)
    {
      case S:
        m_numMOs++;
        break;
      case P:
        m_numMOs += 3;
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
    return m_basis.size() - 1;
  }

  unsigned int BasisSet::addGTO(unsigned int basis, double c, double a)
  {
    // Add a new GTO - if normalised is true then un-normalise c
    // In fact I don't think this should be called and so am going to assume it
    // isn't for now - do any programs give you normalised contraction coeffs???
/*    if (normalised)
    {
      double norm;
      switch (m_basis[basis]->type)
      {
        case S:
          norm = pow(2.0*a / M_PI, 0.75);
          break;
        case P:
          norm = pow(128.0 * pow(a, 5.0) / (M_PI*M_PI*M_PI), 0.25);
          break;
        default:
          norm = 1.0;
      }
      c /= norm;
    } */
    GTO *tmp = new GTO;
    tmp->c = c;
    tmp->a = a;
    m_basis[basis]->GTOs.push_back(tmp);
    m_init = false;

    switch (m_basis[basis]->type) {
      case S:
        m_numCs++;
        break;
      case P:
        m_numCs +=3;
        break;
      case D:
        m_numCs +=6;
        break;
      case D5:
        m_numCs +=5;
      default:
        qDebug() << "Unhandled orbital type.";
    }

    return m_basis[basis]->GTOs.size() - 1;
  }

  void BasisSet::addMOs(const vector<double>& MOs)
  {
    // Append the MOs to our internal vector
    m_MOs.reserve(m_MOs.size() + MOs.size());
    for (unsigned int i = 0; i < MOs.size(); ++i)
      m_MOs.push_back(MOs[i]);
    m_init = false;
  }

  void BasisSet::addMO(double MO)
  {
    m_MOs.push_back(MO);
    m_init = false;
  }

  double BasisSet::calculateMO(const Vector3d& pos, unsigned int state)
  {
    // First build up a list of deltas between atoms and the given position
    if (state < 1 || state > m_MOs.size())
      return 0.0;

    if (!m_init) {
      initCalculation();
      m_init = true;
    }

    vector<Vector3d> deltas;
    vector<double> dr2;
    deltas.reserve(m_atoms.size());
    dr2.reserve(m_atoms.size());
    for (unsigned int i = 0; i < m_atoms.size(); ++i) {
      deltas.push_back(pos - m_atoms.at(i)->pos);
      dr2.push_back(deltas[i].norm2());
    }

    // Reset the m_MO counter and start processing shells
    m_MO = (state - 1) * m_numMOs;
    m_cPos = 0;

    double tmp = 0.0;
    for (unsigned int i = 0; i < m_basis.size(); ++i) {
      tmp += processBasis(m_basis[i], deltas.at(m_basis[i]->atom),
        dr2.at(m_basis[i]->atom));
    }
    return tmp;
  }

  bool BasisSet::calculateCubeMO(Cube *cube, unsigned int state)
  {
    // Set up the calculation and ideally use the new QtConcurrent code to
    // multithread the calculation...
    if (state < 1 || state > m_MOs.size())
      return 0.0;

    initCalculation();

    // It is more efficient to process each shell over the entire cube than it
    // is to process each MO at each point in the cube. This is probably the best
    // point at which to multithread too - QtConcurrent!
    QVector<BasisShell> basisShells(m_shells.size());

    for (uint i = 0; i < m_shells.size(); ++i) {
//      Cube *mapCube = new Cube;
//      mapCube->setLimits(*cube);
      basisShells[i].set = this;
      basisShells[i].tCube = cube;
//      basisShells[i].cube = mapCube;
      basisShells[i].shell = i;
      basisShells[i].state = state;
      // FIXME - QtConcurrent Map should do this!
//    processShell(&basisShells[i]);
    }

    // Let's get this Qt Magic on!
    QFuture<void> future = QtConcurrent::map(basisShells, BasisSet::processShell);
//    future.waitForFinished();
//    QFuture<Cube *> future = QtConcurrent::mapped(basisShells, &BasisSet::processShell);

//    QFuture<Cube *> future = QtConcurrent::blockingMappedReduced(basisShells,
//                                                      &BasisSet::processShell,
//                                                      &BasisSet::reduceShells);
    future.waitForFinished();

    foreach(BasisShell shell, basisShells) {
//      cube->addData(*shell.cube->data());
//      delete shell.cube;
      cube->addData(*shell.vals);
      delete shell.vals;
    }
    return true;

  }

  void BasisSet::initCalculation()
  {
    // This currently just involves normalising all contraction coefficients
    m_c.clear();
    // Assign c and MO indices to the GTOs and basis sets
    unsigned int index = 0;
    unsigned int indexMO = 0;
    foreach(Basis *basis, m_basis) {
      switch (basis->type) {
        case S:
          basis->index = indexMO++;
          foreach(GTO *gto, basis->GTOs) {
            m_c.push_back(gto->c * pow(2.0 * gto->a / M_PI, 0.75));
            gto->index = index++;
          }
          break;
        case P:
          basis->index = indexMO;
          indexMO += 3;
          foreach(GTO *gto, basis->GTOs) {
            m_c.push_back(gto->c * pow(128.0 * pow(gto->a, 5.0)
                          / (M_PI*M_PI*M_PI), 0.25));
            m_c.push_back(m_c.at(m_c.size()-1));
            m_c.push_back(m_c.at(m_c.size()-1));
            gto->index = index;
            index += 3;
          }
          break;
        case D:
          // Cartesian - 6 d components
          // Order in xx, yy, zz, xy, xz, yz
          basis->index = indexMO;
          indexMO += 6;
          foreach(GTO *gto, basis->GTOs) {
            m_c.push_back(gto->c * pow(2048.0 * pow(gto->a, 7.0)
                          / (9.0 * M_PI*M_PI*M_PI), 0.25));
            m_c.push_back(m_c.at(m_c.size()-1));
            m_c.push_back(m_c.at(m_c.size()-1));

            m_c.push_back(gto->c * pow(2048.0 * pow(gto->a, 7.0)
                          / (M_PI*M_PI*M_PI), 0.25));
            m_c.push_back(m_c.at(m_c.size()-1));
            m_c.push_back(m_c.at(m_c.size()-1));
            gto->index = index;
            index += 6;
          }
          break;
        case D5:
          // Spherical - 5 d components
          // Order in d0, d+1, d-1, d+2, d-2
          // Form d(z^2-r^2), dxz, dyz, d(x^2-y^2), dxy
          basis->index = indexMO;
          indexMO += 5;
          foreach(GTO *gto, basis->GTOs) {
            m_c.push_back(gto->c * pow(2048.0 * pow(gto->a, 7.0)
                          / (9.0 * M_PI*M_PI*M_PI), 0.25));
            m_c.push_back(gto->c * pow(2048.0 * pow(gto->a, 7.0)
                          / (M_PI*M_PI*M_PI), 0.25));
            m_c.push_back(m_c.at(m_c.size()-1));
            // I think this is correct but reaally need to check...
            m_c.push_back(gto->c * pow(128.0 * pow(gto->a, 7.0)
                          / (M_PI*M_PI*M_PI), 0.25));
            m_c.push_back(gto->c * pow(2048.0 * pow(gto->a, 7.0)
                          / (M_PI*M_PI*M_PI), 0.25));
            gto->index = index;
            index += 5;
          }
          break;
        default:
          ;
      }
    }
  }

  double BasisSet::processBasis(const Basis* basis, const Vector3d& delta,
    double dr2)
  {
    switch(basis->type)
    {
      case S:
        return doS(basis, dr2);
        break;
      case P:
        return doP(basis, delta, dr2);
        break;
      case D:
        return doD(basis, delta, dr2);
        break;
      case D5:
        return doD5(basis, delta, dr2);
        break;
      default:
        // Not handled - return a zero contribution
        return 0.0;
    }
  }

  double BasisSet::doS(const Basis* basis, double dr2)
  {
    // S type orbitals - the simplest of the calculations with one component
    double tmp = 0.0;
    for (unsigned int i = 0; i < basis->GTOs.size(); ++i) {
      tmp += m_c.at(m_cPos++) * exp(-basis->GTOs.at(i)->a * dr2);
    }
    // There is one MO coefficient per S shell basis - advance by 1 and use
    return m_MOs.at(m_MO++) * tmp;
  }

  double BasisSet::doP(const Basis* basis, const Eigen::Vector3d& delta, double dr2)
  {
    // P type orbitals have three components and each component has a different
    // independent MO weighting. Many things can be cached to save time though
    double tmp = 0.0;
    // Calculate the prefactors for Px, Py and Pz
    double Px = m_MOs.at(m_MO++) * delta.x();
    double Py = m_MOs.at(m_MO++) * delta.y();
    double Pz = m_MOs.at(m_MO++) * delta.z();

    // Now iterate through the P type GTOs and sum their contributions
    for (unsigned int i = 0; i < basis->GTOs.size(); ++i)
    {
      // Calculate the common factor
      double tmpGTO = exp(-basis->GTOs.at(i)->a * dr2);
      tmp += m_c.at(m_cPos++) * Px * tmpGTO; // Px
      tmp += m_c.at(m_cPos++) * Py * tmpGTO; // Py
      tmp += m_c.at(m_cPos++) * Pz * tmpGTO; // Pz
    }
    return tmp;
  }

  double BasisSet::doD(const Basis* basis, const Eigen::Vector3d& delta, double dr2)
  {
    // D type orbitals have five components and each component has a different
    // independent MO weighting. Many things can be cached to save time though
    double tmp = 0.0;
    // Calculate the prefactors
    double Dxx = m_MOs.at(m_MO++) * delta.x() * delta.x();
    double Dyy = m_MOs.at(m_MO++) * delta.y() * delta.y();
    double Dzz = m_MOs.at(m_MO++) * delta.z() * delta.z();
    double Dxy = m_MOs.at(m_MO++) * delta.x() * delta.y();
    double Dxz = m_MOs.at(m_MO++) * delta.x() * delta.z();
    double Dyz = m_MOs.at(m_MO++) * delta.y() * delta.z();

    // Now iterate through the D type GTOs and sum their contributions
    for (unsigned int i = 0; i < basis->GTOs.size(); ++i)
    {
      // Calculate the common factor
      double tmpGTO = exp(-basis->GTOs.at(i)->a * dr2);
      tmp += m_c.at(m_cPos++) * Dxx * tmpGTO; // Dxx
      tmp += m_c.at(m_cPos++) * Dyy * tmpGTO; // Dyy
      tmp += m_c.at(m_cPos++) * Dzz * tmpGTO; // Dzz
      tmp += m_c.at(m_cPos++) * Dxy * tmpGTO; // Dxy
      tmp += m_c.at(m_cPos++) * Dxz * tmpGTO; // Dxz
      tmp += m_c.at(m_cPos++) * Dyz * tmpGTO; // Dyz
    }
    return tmp;
  }

  double BasisSet::doD5(const Basis* basis, const Eigen::Vector3d& delta, double dr2)
  {
    // D type orbitals have five components and each component has a different
    // MO weighting. Many things can be cached to save time
    double tmp = 0.0;
    // Calculate the prefactors
    double xx = delta.x() * delta.x();
    double yy = delta.y() * delta.y();
    double zz = delta.z() * delta.z();
    double xy = delta.x() * delta.y();
    double xz = delta.x() * delta.z();
    double yz = delta.y() * delta.z();

    double D0 = m_MOs.at(m_MO++) * (zz - dr2);
    double D1p = m_MOs.at(m_MO++) * xz;
    double D1n = m_MOs.at(m_MO++) * yz;
    double D2p = m_MOs.at(m_MO++) * (xx - yy);
    double D2n = m_MOs.at(m_MO++) * xy;

    // Not iterate through the GTOs
    for (unsigned int i = 0; i < basis->GTOs.size(); ++i)
    {
      double tmpGTO = exp(-basis->GTOs.at(i)->a * dr2);
      tmp += m_c.at(m_cPos++) * D0 * tmpGTO;  // D0
      tmp += m_c.at(m_cPos++) * D1p * tmpGTO; // D1p
      tmp += m_c.at(m_cPos++) * D1n * tmpGTO; // D1n
      tmp += m_c.at(m_cPos++) * D2p * tmpGTO; // D2p
      tmp += m_c.at(m_cPos++) * D2n * tmpGTO; // D2n
    }
    return tmp;
  }

  Cube * BasisSet::processShell(BasisShell &shell)
  {
    // Static member variable. Must be re-entrant in order to be used by
    // Qt Concurrent
//    Cube *cube = new Cube();
//    cube->setLimits(*shell.tCube);
//    shell.cube = cube;

    double step = shell.tCube->spacing().x() * ANGSTROM_TO_BOHR;
    Vector3i dim = shell.tCube->dimensions();
    Vector3d min = shell.tCube->min() * ANGSTROM_TO_BOHR;

    unsigned int size = dim.x() * dim.y() * dim.z();
//    vector<double> &vals = *shell.cube->data();
    vector<double> *vals = new vector<double>(size);
    shell.vals = vals;
    vector<Vector3d> delta(size);
    vector<double> dr2(size);

    Vector3d atomPos = shell.set->m_atoms[shell.shell]->pos;
    Vector3d tmpPos = min;

    // Calculate the delta and dr2 vectors
    uint counter = 0;
    for (int i = 0; i < dim.x(); ++i) {
      for (int j = 0; j < dim.y(); ++j) {
        for (int k = 0; k < dim.z(); ++k) {
          tmpPos = Vector3d(min.x() + i * step,
                            min.y() + j * step,
                            min.z() + k * step);
          delta[counter] = tmpPos - atomPos;
          dr2[counter] = delta[counter].norm2();
          ++counter;
        }
      }
    }

    // The indexMO is the index into the MO coefficients
    unsigned int indexMO = (shell.state - 1) * shell.set->m_numMOs;

    foreach(Basis *basis, shell.set->m_shells[shell.shell]) {
      switch(basis->type) {
        case S:
          cubeS(shell.set, *vals, basis, dr2, indexMO);
          break;
        case P:
          cubeP(shell.set, *vals, basis, delta, dr2, indexMO);
          break;
        case D:
          cubeD(shell.set, *vals, basis, delta, dr2, indexMO);
          break;
        case D5:
          cubeD5(shell.set, *vals, basis, delta, dr2, indexMO);
          break;
        default:
        // Not handled - return a zero contribution
        qDebug() << "Error: Cannot process basis.";
      }
    }
    return shell.cube;
  }

  void BasisSet::reduceShells(Cube *rCube, Cube *iCube)
  {
    // This is the reduce function for the QtConcurrent map-reduce technique
    rCube->addData(*iCube->data());
    delete iCube;
    qDebug() << "Adding a cube to the result cube...";
  }

  inline void BasisSet::cubeS(BasisSet *set, vector<double> &vals, const Basis* basis,
                       const vector<double> &dr2, unsigned int indexMO)
  {
    // If the MO coefficient is very small skip it
    if (set->m_MOs[indexMO + basis->index] > -1e-15 && set->m_MOs[indexMO + basis->index] < 1e-15) {
      return;
    }
    // S type orbitals - the simplest of the calculations with one component
    for (unsigned int pos = 0; pos < vals.size(); ++pos) {
      double tmp = 0.0;
      for (unsigned int i = 0; i < basis->GTOs.size(); ++i) {
        tmp += set->m_c[basis->GTOs[i]->index] * exp(-basis->GTOs[i]->a * dr2[pos]);
      }
      // There is one MO coefficient per S shell basis
      vals[pos] += tmp * set->m_MOs[indexMO + basis->index];
    }
  }

  inline void BasisSet::cubeP(BasisSet *set, vector<double> &vals, const Basis* basis,
                       const vector<Vector3d> &delta,
                       const vector<double> &dr2, unsigned int indexMO)
  {
    // P type orbitals have three components and each component has a different
    // independent MO weighting. Many things can be cached to save time though
    if (set->m_MOs[indexMO + basis->index] > -1e-15
        && set->m_MOs[indexMO + basis->index] < 1e-15
        && set->m_MOs[indexMO + basis->index+1] > -1e-15
        && set->m_MOs[indexMO + basis->index+1] < 1e-15
        && set->m_MOs[indexMO + basis->index+2] > -1e-15
        && set->m_MOs[indexMO + basis->index+2] < 1e-15) {
      return;
    }
    for (unsigned int pos = 0; pos < vals.size(); ++pos) {
      double tmp = 0.0;
      // Calculate the prefactors for Px, Py and Pz
      double Px = set->m_MOs[indexMO + basis->index  ] * delta[pos].x();
      double Py = set->m_MOs[indexMO + basis->index+1] * delta[pos].y();
      double Pz = set->m_MOs[indexMO + basis->index+2] * delta[pos].z();

      // Now iterate through the P type GTOs and sum their contributions
      for (unsigned int i = 0; i < basis->GTOs.size(); ++i) {
        double tmpGTO = exp(-basis->GTOs[i]->a * dr2[pos]);
        tmp += set->m_c[basis->GTOs[i]->index  ] * Px * tmpGTO;
        tmp += set->m_c[basis->GTOs[i]->index+1] * Py * tmpGTO;
        tmp += set->m_c[basis->GTOs[i]->index+2] * Pz * tmpGTO;
      }
      vals[pos] += tmp;
    }
  }

  inline void BasisSet::cubeD(BasisSet *set, vector<double> &vals, const Basis* basis,
                       const vector<Vector3d> &delta,
                       const vector<double> &dr2, unsigned int indexMO)
  {
    // D type orbitals have five components and each component has a different
    // independent MO weighting. Many things can be cached to save time though
    for (unsigned int pos = 0; pos < vals.size(); ++pos) {
      double tmp = 0.0;
      // Calculate the prefactors
      double Dxx = set->m_MOs[indexMO + basis->index  ] * delta[pos].x()
                   * delta[pos].x();
      double Dyy = set->m_MOs[indexMO + basis->index+1] * delta[pos].y()
                   * delta[pos].y();
      double Dzz = set->m_MOs[indexMO + basis->index+2] * delta[pos].z()
                   * delta[pos].z();
      double Dxy = set->m_MOs[indexMO + basis->index+3] * delta[pos].x()
                   * delta[pos].y();
      double Dxz = set->m_MOs[indexMO + basis->index+4] * delta[pos].x()
                   * delta[pos].z();
      double Dyz = set->m_MOs[indexMO + basis->index+5] * delta[pos].y()
                   * delta[pos].z();

      // Now iterate through the D type GTOs and sum their contributions
      for (unsigned int i = 0; i < basis->GTOs.size(); ++i) {
        // Calculate the common factor
        double tmpGTO = exp(-basis->GTOs[i]->a * dr2[pos]);
        tmp += set->m_c[basis->GTOs[i]->index  ] * Dxx * tmpGTO; // Dxx
        tmp += set->m_c[basis->GTOs[i]->index+1] * Dyy * tmpGTO; // Dyy
        tmp += set->m_c[basis->GTOs[i]->index+2] * Dzz * tmpGTO; // Dzz
        tmp += set->m_c[basis->GTOs[i]->index+3] * Dxy * tmpGTO; // Dxy
        tmp += set->m_c[basis->GTOs[i]->index+4] * Dxz * tmpGTO; // Dxz
        tmp += set->m_c[basis->GTOs[i]->index+5] * Dyz * tmpGTO; // Dyz
      }
      vals[pos] += tmp;
    }
  }

  inline void BasisSet::cubeD5(BasisSet *set, vector<double> &vals, const Basis* basis,
                        const vector<Vector3d> &delta,
                        const vector<double> &dr2, unsigned int indexMO)
  {
    // D type orbitals have five components and each component has a different
    // MO weighting. Many things can be cached to save time
    for (unsigned int pos = 0; pos < vals.size(); ++pos) {
      double tmp = 0.0;
      // Calculate the prefactors
      double xx = delta[pos].x() * delta[pos].x();
      double yy = delta[pos].y() * delta[pos].y();
      double zz = delta[pos].z() * delta[pos].z();
      double xy = delta[pos].x() * delta[pos].y();
      double xz = delta[pos].x() * delta[pos].z();
      double yz = delta[pos].y() * delta[pos].z();

      double D0  = set->m_MOs[indexMO + basis->index  ] * (zz - dr2[pos]);
      double D1p = set->m_MOs[indexMO + basis->index+1] * xz;
      double D1n = set->m_MOs[indexMO + basis->index+2] * yz;
      double D2p = set->m_MOs[indexMO + basis->index+3] * (xx - yy);
      double D2n = set->m_MOs[indexMO + basis->index+4] * xy;

      // Not iterate through the GTOs
      for (unsigned int i = 0; i < basis->GTOs.size(); ++i) {
        double tmpGTO = exp(-basis->GTOs[i]->a * dr2[pos]);
        tmp += set->m_c[basis->GTOs[i]->index  ] * D0 * tmpGTO;  // D0
        tmp += set->m_c[basis->GTOs[i]->index+1] * D1p * tmpGTO; // D1p
        tmp += set->m_c[basis->GTOs[i]->index+2] * D1n * tmpGTO; // D1n
        tmp += set->m_c[basis->GTOs[i]->index+3] * D2p * tmpGTO; // D2p
        tmp += set->m_c[basis->GTOs[i]->index+4] * D2n * tmpGTO; // D2n
      }
      vals[pos] += tmp;
    }
  }

  int BasisSet::numMOs()
  {
    // Return the total number of MOs
    return m_MOs.size() / m_numMOs;
  }

  void BasisSet::addAtoms(Molecule* mol)
  {
    // Add our atoms to the molecule
    for (unsigned int i = 0; i < m_atoms.size(); ++i) {
      Atom* atom = mol->newAtom();
      atom->setPos(m_atoms.at(i)->pos * BOHR_TO_ANGSTROM);
      atom->setAtomicNumber(m_atoms.at(i)->num);
    }
  }

  void BasisSet::outputAll()
  {
    for (unsigned int i = 0; i < m_atoms.size(); ++i)
      qDebug() << "Atom" << i << m_atoms.at(i)->pos.z()
               << "num=" << m_atoms.at(i)->num;
    for (unsigned int i = 0; i < m_basis.size(); ++i)
    {
      qDebug() << "Basis set" << i << "type" << m_basis[i]->type;
      for (unsigned int j = 0; j < m_basis[i]->GTOs.size(); ++j)
      {
        qDebug() << i << ":" << m_basis[i]->GTOs[j]->c << ","
          << m_basis[i]->GTOs[j]->a;
      }
    }
    qDebug() << "Number of MOs" << numMOs();
  }

}
