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

#ifndef BASISSET_H
#define BASISSET_H

#include <QObject>
#include <QFuture>
#include <QFutureWatcher>

#include <Eigen/Core>
#include <vector>

// Basis sets...

namespace Avogadro
{
  class Molecule;
  class Cube;
  struct BasisShell;

  /**
   * Enumeration of the Gaussian type orbitals.
   */
  enum orbital { S, SP, P, D, D5, F, F7, UU };

  /**
   * @class BasisSet basisset.h
   * @brief BasisSet Class
   * @author Marcus D. Hanwell
   *
   * The BasisSet class has a transparent data structure for storing the basis
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

  class BasisSet : public QObject
  {
  Q_OBJECT

  public:
    /**
     * Constructor.
     */
    BasisSet();

    /**
     * Destructor.
     */
    ~BasisSet();

    /**
     * Function to add an atom to the BasisSet.
     * @param pos Position of the center of the QAtom.
     * @param num The atomic number of the QAtom.
     * @return The index of the added atom.
     */
    unsigned int addAtom(const Eigen::Vector3d& pos, int num = 0);

    /**
     * Add a basis to the basis set.
     * @param atom Index of the atom to add the Basis too.
     * @param type The type of the Basis being added.
     * @return The index of the added Basis.
     */
    unsigned int addBasis(unsigned int atom, orbital type);

    /**
     * Add a GTO to the supplied basis.
     * @param basis The index of the Basis to add the GTO to.
     * @param c The contraction coefficient of the GTO.
     * @param a The exponent of the GTO.
     * @return The index of the added GTO.
     */
    unsigned int addGTO(unsigned int basis, double c, double a);

    /**
     * Add MO coefficients to the BasisSet.
     * @param MOs Vector containing the MO coefficients for the BasisSet.
     */
    void addMOs(const std::vector<double>& MOs);

    /**
     * Add an individual MO coefficient.
     * @param MO The MO coefficient.
     */
    void addMO(double MO);

    /**
     * Set the SCF density matrix for the BasisSet.
     */
    bool setDensityMatrix(const Eigen::MatrixXd &m);

    /**
     * Set the number of electrons in the BasisSet.
     * @param n The number of electrons in the BasisSet.
     */
    void setElectrons(unsigned int n) { m_electrons = n; }

    /**
     * Add all of the atoms in the Molecule to the BasisSet.
     * @param mol Molecule to copy atoms across from.
     */
    void addAtoms(Molecule* mol);

    /**
     * Debug routine, outputs all of the data in the BasisSet.
     */
    void outputAll();

    /**
     * @return The number of MOs in the BasisSet.
     */
    int numMOs();

    /**
     * Check if the given MO number is the HOMO or not.
     * @param n The MO number.
     * @return True if the given MO number is the HOMO.
     */
    bool HOMO(unsigned int n)
    {
      if (n+1 == static_cast<unsigned int>(m_electrons / 2))
        return true;
      else
        return false;
    }

    /**
     * Check if the given MO number is the LUMO or not.
     * @param n The MO number.
     * @return True if the given MO number is the LUMO.
     */
    bool LUMO(unsigned int n)
    {
      if (n == static_cast<unsigned int>(m_electrons / 2))
        return true;
      else
        return false;
    }

    /**
     * Calculate the MO over the entire range of the supplied Cube.
     * @param cube The cube to write the values of the MO into.
     * @return True if the calculation was successful.
     */
    bool calculateCubeMO(Cube *cube, int state = 1);

    /**
     * Calculate the electron density over the entire range of the supplied Cube.
     * @param cube The cube to write the values of the MO into.
     * @return True if the calculation was successful.
     */
    bool calculateCubeDensity(Cube *cube);

    /**
     * When performing a calculation the QFutureWatcher is useful if you want
     * to update a progress bar.
     */
    QFutureWatcher<void> & watcher() { return m_watcher; }

  private Q_SLOTS:
     /**
     * Slot to set the cube data once Qt Concurrent is done
     */
     void calculationComplete();

  private:
    // New storage of the data
    std::vector<Eigen::Vector3d> m_atomPos;  // Atom position vectors
    std::vector<int> m_symmetry;             // Symmetry of the basis, S, P...
    std::vector<unsigned int> m_atomIndices; // Indices into the atomPos vector
    std::vector<unsigned int> m_moIndices;   // Indices into the MO/density matrix
    std::vector<unsigned int> m_gtoIndices;  // Indices into the GTO vector
    std::vector<unsigned int> m_cIndices;    // Indices into m_gtoCN
    std::vector<double> m_gtoA;              // The GTO exponent
    std::vector<double> m_gtoC;              // The GTO contraction coefficient
    std::vector<double> m_gtoCN;             // The GTO contraction coefficient (normalized)
    Eigen::MatrixXd m_moMatrix;              // MO coefficient matrix
    Eigen::MatrixXd m_density;               // Density matrix

    unsigned int m_numMOs; // The number of GTOs
    unsigned int m_electrons; // Total number of electrons
    unsigned int m_numAtoms;  // Total number of atoms in the basis set
    bool m_init; // Has the calculation been initialised?

    QFuture<void> m_future;
    QFutureWatcher<void> m_watcher;
    Cube *m_cube; // Cube to put the results into
    QVector<BasisShell> *m_basisShells;

    static bool isSmall(double val);

    void initCalculation();  // Perform initialisation before any calculations
    /// Re-entrant single point forms of the calculations
    static void processPoint(BasisShell &shell);
    static void processDensity(BasisShell &shell);
    static double pointS(BasisSet *set, unsigned int moIndex,
                      const double &dr2, unsigned int indexMO);
    static double pointP(BasisSet *set, unsigned int moIndex,
                      const Eigen::Vector3d &delta,
                      const double &dr2, unsigned int indexMO);
    static double pointD(BasisSet *set, unsigned int moIndex,
                      const Eigen::Vector3d &delta,
                      const double &dr2, unsigned int indexMO);
    static double pointD5(BasisSet *set, unsigned int moIndex,
                      const Eigen::Vector3d &delta,
                      const double &dr2, unsigned int indexMO);
    // Calculate the basis for the density
    static void pointS(BasisSet *set, const double &dr2, int basis,
                         Eigen::MatrixXd &out);
    static void pointP(BasisSet *set, const Eigen::Vector3d &delta,
                         const double &dr2, int basis, Eigen::MatrixXd &out);
    static void pointD(BasisSet *set, const Eigen::Vector3d &delta,
                         const double &dr2, int basis, Eigen::MatrixXd &out);
  };

} // End namespace Avogadro

#endif
