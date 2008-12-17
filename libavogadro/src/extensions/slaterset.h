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

#ifndef SLATERSET_H
#define SLATERSET_H

#include <QObject>
#include <QFuture>
#include <QFutureWatcher>

#include <Eigen/Core>
#include <vector>

/**
 * @class SlaterSet slaterset.h
 * @brief SlaterSet Class
 * @author Marcus D. Hanwell
 *
 * The SlaterSet class has a transparent data structure for storing the basis
 * sets output by many quantum mechanical codes. It has a certain hierarchy
 * where shells are built up from n primitives, in this case Gaussian Type
 * Orbitals (GTOs). Each shell has a type (S, P, D, F, etc) and is composed of
 * one or more GTOs. Each GTO has a contraction coefficient, c, and an exponent,
 * a.
 *
 * When calculating Molecular Orbitals (MOs) each orthogonal shell has an
 * independent coefficient. That is the S type orbitals have one coefficient,
 * the P type orbitals have three coefficients (Px, Py and Pz), the D type
 * orbitals have five (or six if cartesian types) coefficients, and so on.
 */

namespace Avogadro
{

  class Molecule;
  class Cube;
  struct SlaterShell;

  class SlaterSet : public QObject
  {
  Q_OBJECT

  public:
    /**
     * Constructor.
     */
    SlaterSet();

    /**
     * Destructor.
     */
    ~SlaterSet();

    /**
     * Enumeration of the Slater orbital types.
     */
    enum slater { S, PX, PY, PZ, X2, XZ, Z2, YZ, XY, UU };

    /**
     * Function to add an atom to the SlaterSet.
     * @param pos Position of the center of the QAtom.
     * @return The index of the added atom.
     */
    bool addAtoms(const std::vector<Eigen::Vector3d> &pos);

    /**
     * Add a basis to the basis set.
     * @param i Index of the atom to add the Basis too.
     * @return The index of the added Basis.
     */
    bool addSlaterIndices(const std::vector<int> &i);

    /**
     * Add the symmetry types for the orbitals.
     * @param t Vector containing the types of symmetry using the slater enum.
     */
    bool addSlaterTypes(const std::vector<int> &t);

    /**
     * Add a GTO to the supplied basis.
     * @param zetas The exponents of the STOs
     * @return True if successful.
     */
    bool addZetas(const std::vector<double> &zetas);

    /**
     * The PQNs for the orbitals.
     */
    bool addPQNs(const std::vector<double> &pqns);

    /**
     * The number of electrons in the molecule.
     */
    bool setNumElectrons(double electrons);

    /**
     * The overlap matrix.
     * @param m Matrix containing the overlap matrix for the basis.
     */
    bool addOverlapMatrix(const Eigen::MatrixXd &m);

    /**
     * Add Eigen Vectors to the SlaterSet.
     * @param MOs Matrix of the eigen vectors for the SlaterSet.
     */
    bool addEigenVectors(const Eigen::MatrixXd &e);

    /**
     * Add the density matrix to the SlaterSet.
     * @param d Density matrix for the SlaterSet.
     */
    bool addDensityMatrix(const Eigen::MatrixXd &d);

    /**
     * @return The number of MOs in the BasisSet.
     */
    unsigned int numMOs();

    /**
     * Check if the given MO number is the HOMO or not.
     * @param n The MO number.
     * @return True if the given MO number is the HOMO.
     */
    bool HOMO(unsigned int n)
    {
      if (n+1 == static_cast<unsigned int>(m_electrons / 2)) return true;
      else return false;
    }

    /**
     * Check if the given MO number is the LUMO or not.
     * @param n The MO number.
     * @return True if the given MO number is the LUMO.
     */
    bool LUMO(unsigned int n)
    {
      if (n == static_cast<unsigned int>(m_electrons / 2)) return true;
      else return false;
    }

    void outputAll();

    bool calculateCubeMO(Cube *cube, unsigned int state = 1);

    bool calculateCubeDensity(Cube *cube);

    QFutureWatcher<void> & watcher() { return m_watcher; }

  private Q_SLOTS:
    /**
     * Slot to set the cube data once Qt Concurrent is done
     */
     void calculationComplete();

  private:
    std::vector<Eigen::Vector3d> m_atomPos;
    std::vector<int> m_slaterIndices;
    std::vector<int> m_slaterTypes;
    std::vector<double> m_zetas;
    std::vector<double> m_pqns;
    unsigned int m_electrons;
    std::vector<double> m_factors;
    Eigen::MatrixXd m_overlap;
    Eigen::MatrixXd m_eigenVectors;
    Eigen::MatrixXd m_density;
    Eigen::MatrixXd m_normalized;
    bool m_initialized;

    QFuture<void> m_future;
    QFutureWatcher<void> m_watcher;
    Cube *m_cube; // Cube to put the results into
    QVector<SlaterShell> m_slaterShells;

    bool initialize();

    static bool isSmall(double val);
    unsigned int factorial(unsigned int n);

    static void processPoint(SlaterShell &shell);
    static void processDensity(SlaterShell &shell);
    static double pointSlater(SlaterSet *set, const Eigen::Vector3d &delta,
                      const double &dr2, unsigned int slater, unsigned int indexMO);
  };

} // End namespace Avogadro

#endif
