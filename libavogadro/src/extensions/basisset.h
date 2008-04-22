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

#ifndef __BASISSET_H
#define __BASISSET_H

#include <avogadro/primitive.h>
#include <eigen/vector.h>
#include <QList>
#include <QDebug>

// Basis sets...

/**
 * This class implements a transparent data structure for storing the basis sets
 * output by many quantum mechanical codes. It has a certain hierarchy where
 * shells are built up from n primitives, in this case Gaussian Type Orbitals
 * (GTOs). Each shell has a type (S, P, D, F) and is composed of one or more
 * GTOs. Each GTO has a contraction coefficient, c, and an exponent, a.
 * When calculatating Molecular Orbitals (MOs) each orthogonal shell has an
 * independent coefficient. That is the S type orbitals have one coefficient,
 * the P type orbitals have three coefficients (Px, Py and Pz), the D type
 * orbitals have five (or six) coefficients and so on.
 */

namespace Avogadro
{

  // Orbital types that are handled
  enum orbital { S, SP, P, D, D5, F, F7 };

  struct GTO
  {
    double c;
    double a;
  };

  struct Basis
  {
    int atom; // The ID of the atom the basis belongs to
    orbital type; // The orbital type, i.e. S, P, D etc.
    QList<GTO *> GTOs;
  };

  struct QAtom
  {
    Eigen::Vector3d pos;
    int num;
  };

  class BasisSet
  {
  public:
    BasisSet();
    ~BasisSet();
    int addAtom(const Eigen::Vector3d& pos, int num = 0); // Just want a postion - return the index
    int addBasis(int atom, orbital type); // The basis type return the index
    int addGTO(int basis, double c, double a, bool normalise = false); // Add the GTO
    void addMOs(const QList<double>& MOs); // Add MO coefficients
    void addMO(double MO); // Add the MO coefficient
    void setElectrons(int n) { m_electrons = n; }

    void addAtoms(Molecule* mol);
    void outputAll();

    int numMOs();

    bool HOMO(int n)
    {
      if (n+1 == static_cast<int>(m_electrons / 2)) return true;
      else return false;
    }
    bool LUMO(int n)
    {
      if (n == static_cast<int>(m_electrons / 2)) return true;
      else return false;
    }

    double calculateMO(const Eigen::Vector3d& pos, int state = 0);
    // Evaluate the MO at pos

  private:
    QList<GTO *> m_GTOs;
    QList<Basis *> m_basis;
    QList<QAtom *> m_atoms;
    QList<double> m_MOs;

    int m_MO;
    int m_numMOs;
    int m_electrons;

    double processShell(const Basis* basis, const Eigen::Vector3d& delta,
      double dr2);
    double doS(const Basis* basis, double dr2);
    double doP(const Basis* basis, const Eigen::Vector3d& delta, double dr2);
  };

} // End namespace Avogadro

#endif
