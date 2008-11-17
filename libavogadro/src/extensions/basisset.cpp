/**********************************************************************
  BasisSet - encapsulation of basis sets

  Copyright (C) 2008 Marcus D. Hanwell
  Copyright (C) 2008 Albert De Fusco

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.sourceforge.net/>

  This library is free software; you can redistribute it and/or modify
  it under the terms of the GNU Library General Public License as
  published by the Free Software Foundation; either version 2 of the
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

#include <cmath>

using namespace Eigen;
using namespace std;

namespace Avogadro
{

  using std::vector;

  static const double BOHR_TO_ANGSTROM = 0.529177249;
  static const double ANGSTROM_TO_BOHR = 1.0 / 0.529177249;

  BasisSet::BasisSet() : m_MO(0), m_cPos(0), m_numMOs(0), m_electrons(0)
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

  int BasisSet::addAtom(const Vector3d& pos, int num)
  {
    qDebug() << "New atom added" << num;
    QAtom *tmp = new QAtom;
    tmp->pos = pos;
    tmp->num = num;
    m_atoms.push_back(tmp);
    return m_atoms.size() - 1;
  }

  int BasisSet::addBasis(int atom, orbital type)
  {
    Basis *tmp = new Basis;
    tmp->atom = atom;
    tmp->type = type;
    m_basis.push_back(tmp);
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
    return m_basis.size() - 1;
  }

  int BasisSet::addGTO(int basis, double c, double a)
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
    return m_basis[basis]->GTOs.size() - 1;
  }

  void BasisSet::addMOs(const vector<double>& MOs)
  {
    // Append the MOs to our internal vector
    for (unsigned int i = 0; i < MOs.size(); ++i)
      m_MOs.push_back(MOs[i]);
  }

  void BasisSet::addMO(double MO)
  {
    m_MOs.push_back(MO);
  }

  double BasisSet::calculateMO(const Vector3d& pos, unsigned int state)
  {
    // First build up a list of deltas between atoms and the given position
    if (state < 1 || state > m_MOs.size())
      return 0.0;

    static bool init = false;
    if (!init)
      initCalculation();

    vector<Vector3d> deltas;
    vector<double> dr2;
    for (unsigned int i = 0; i < m_atoms.size(); ++i)
    {
      deltas.push_back(pos - m_atoms.at(i)->pos);
      dr2.push_back(deltas.at(i).x()*deltas.at(i).x()
                 + deltas.at(i).y()*deltas.at(i).y()
                 + deltas.at(i).z()*deltas.at(i).z());
    }

    // Reset the m_MO counter and start processing shells
    m_MO = (state - 1) * m_numMOs;
    m_cPos = 0;

    double tmp = 0.0;
    for (unsigned int i = 0; i < m_basis.size(); ++i)
      tmp += processShell(m_basis.at(i), deltas.at(m_basis[i]->atom),
        dr2.at(m_basis[i]->atom));

    return tmp;
  }

  void BasisSet::initCalculation()
  {
    // This currently just involves normalising all contraction coefficients
    m_c.clear();
    GTO* gto;
    Basis* basis;
    for (unsigned int i = 0; i < m_basis.size(); ++i)
    {
      basis = m_basis.at(i);
      switch (basis->type)
      {
        case S:
          for (unsigned int j = 0; j < basis->GTOs.size(); ++j)
          {
            gto = basis->GTOs.at(j);
            m_c.push_back(gto->c * pow(2.0 * gto->a / M_PI, 0.75));
          }
          break;
        case P:
          for (unsigned int j = 0; j < basis->GTOs.size(); ++j)
          {
            gto = basis->GTOs.at(j);
            m_c.push_back(gto->c * pow(128.0 * pow(gto->a, 5.0)
                          / (M_PI*M_PI*M_PI), 0.25));
            m_c.push_back(m_c.at(m_c.size()-1));
            m_c.push_back(m_c.at(m_c.size()-1));
          }
          break;
        case D:
          // Cartesian - 6 d components
          // Order in xx, yy, zz, xy, xz, yz
          for (unsigned int j = 0; j < basis->GTOs.size(); ++j)
          {
            gto = basis->GTOs.at(j);
            m_c.push_back(gto->c * pow(2048.0 * pow(gto->a, 7.0)
                          / (9.0 * M_PI*M_PI*M_PI), 0.25));
            m_c.push_back(m_c.at(m_c.size()-1));
            m_c.push_back(m_c.at(m_c.size()-1));

            m_c.push_back(gto->c * pow(2048.0 * pow(gto->a, 7.0)
                          / (M_PI*M_PI*M_PI), 0.25));
            m_c.push_back(m_c.at(m_c.size()-1));
            m_c.push_back(m_c.at(m_c.size()-1));
          }
          break;
        case D5:
          // Spherical - 5 d components
          // Order in d0, d+1, d-1, d+2, d-2
          // Form d(z^2-r^2), dxz, dyz, d(x^2-y^2), dxy
          for (unsigned int j = 0; j < basis->GTOs.size(); ++j)
          {
            gto = basis->GTOs.at(j);
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
          }
          break;
        default:
          ;
      }
    }
  }

  double BasisSet::processShell(const Basis* basis, const Vector3d& delta,
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
    for (unsigned int i = 0; i < basis->GTOs.size(); ++i)
      tmp += m_c.at(m_cPos++) * exp(-basis->GTOs.at(i)->a * dr2);
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
