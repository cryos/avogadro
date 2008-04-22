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

#include <cmath>

using namespace Eigen;
using namespace std;

namespace Avogadro
{

  static const double BOHR_TO_ANGSTROM = 0.529177249;
  static const double ANGSTROM_TO_BOHR = 1.0 / 0.529177249;

  BasisSet::BasisSet() : m_numMOs(0)
  {
  }

  BasisSet::~BasisSet()
  {
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

  int BasisSet::addGTO(int basis, double c, double a, bool normalise)
  {
    // Add a new GTO - if normalise is true then normalise c
    if (normalise)
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
      c *= norm;
    }
    GTO *tmp = new GTO;
    tmp->c = c;
    tmp->a = a;
    m_basis[basis]->GTOs.push_back(tmp);
    return m_basis[basis]->GTOs.size() - 1;
  }

  void BasisSet::addMOs(const QList<double>& MOs)
  {
    // Append the MOs to our internal QList
    for (int i = 0; i < MOs.size(); ++i)
      m_MOs.push_back(MOs[i]);
  }

  void BasisSet::addMO(double MO)
  {
    m_MOs.push_back(MO);
  }

  double BasisSet::calculateMO(const Vector3d& pos, int state)
  {
    // First build up a list of deltas between atoms and the given position
    QList<Vector3d> deltas;
    QList<double> dr2;
    for (int i = 0; i < m_atoms.size(); ++i)
    {
      deltas << (pos - m_atoms[i]->pos);
      dr2 << deltas[i].x()*deltas[i].x() + deltas[i].y()*deltas[i].y()
             + deltas[i].z()*deltas[i].z();
    }

    // Reset the m_MO counter and start processing shells
    m_MO = state * m_numMOs;
    double tmp = 0.0;
    for (int i = 0; i < m_basis.size(); ++i)
      tmp += processShell(m_basis[i], deltas[m_basis[i]->atom],
        dr2[m_basis[i]->atom]);

    return tmp;
  }

  void BasisSet::addAtoms(Molecule* mol)
  {
    // Add our atoms to the molecule
    for (int i = 0; i < m_atoms.size(); ++i)
    {
      mol->BeginModify();
      Atom* atom = static_cast<Atom*>(mol->NewAtom());
      atom->setPos(m_atoms.at(i)->pos * BOHR_TO_ANGSTROM);
      atom->SetAtomicNum(m_atoms.at(i)->num);
      mol->EndModify();
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
      default:
        // Not handled - issue a warning and return
        qDebug() << "Unknown orbital type called - skipping...";
        return 0.0;
    }
  }

  double BasisSet::doS(const Basis* basis, double dr2)
  {
    // S type orbitals - the simplest of the calculations with one component
    double tmp = 0.0;
    for (int i = 0; i < basis->GTOs.size(); ++i)
      tmp += basis->GTOs[i]->c * exp(-basis->GTOs[i]->a * dr2);
    // There is one MO coefficient per S shell basis - advance by 1 and use
    return m_MOs[m_MO++] * tmp;
  }

  double BasisSet::doP(const Basis* basis, const Eigen::Vector3d& delta, double dr2)
  {
    // P type orbitals have three components and each component has a different
    // independent MO weighting. Many things can be cached to save time though
    double tmp = 0.0;
    // Calculate the prefactors for Px, Py and Pz
    double Px = m_MOs[m_MO++] * delta.x();
    double Py = m_MOs[m_MO++] * delta.y();
    double Pz = m_MOs[m_MO++] * delta.z();

    // Now iterate through the P type GTOs and sum their contributions
    for (int i = 0; i < basis->GTOs.size(); ++i)
    {
      // Calculate the common factor
      double tmpGTO = basis->GTOs.at(i)->c * exp(-basis->GTOs.at(i)->a * dr2);
      tmp += Px * tmpGTO; // Px
      tmp += Py * tmpGTO; // Py
      tmp += Pz * tmpGTO; // Pz
    }

    return tmp;
  }

  int BasisSet::numMOs()
  {
    // Return the total number of MOs
    return m_MOs.size() / m_numMOs;
  }

  void BasisSet::outputAll()
  {
    for (int i = 0; i < m_atoms.size(); ++i)
      qDebug() << "Atom" << i << m_atoms.at(i)->pos.z()
               << "num=" << m_atoms.at(i)->num;
    for (int i = 0; i < m_basis.size(); ++i)
    {
      qDebug() << "Basis set" << i << "type" << m_basis[i]->type;
      for (int j = 0; j < m_basis[i]->GTOs.size(); ++j)
      {
        qDebug() << i << ":" << m_basis[i]->GTOs[j]->c << ","
          << m_basis[i]->GTOs[j]->a;
      }
    }
    qDebug() << "Number of MOs" << numMOs();
  }

}
