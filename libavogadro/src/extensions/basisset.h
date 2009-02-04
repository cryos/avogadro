/**********************************************************************
  BasisSet - encapsulation of basis sets

  Copyright (C) 2008 Marcus D. Hanwell
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
   * Simple structure to encapsulate a GTO, where c is the contraction
   * coefficient and a is th exponent.
   */
  struct GTO
  {
    double c;
    double a;
    unsigned int index;
  };

  /**
   * Simple structure containing a Basis, where atom is the atom id, the index
   * is the position index in the MO vector, type is orbital type and GTOs is a
   * vector containing the GTOs for the Basis.
   */
  struct Basis
  {
    unsigned int atom;
    unsigned int index;
    orbital type; // The orbital type, i.e. S, P, D etc.
    std::vector<GTO *> GTOs;
  };

  /**
   * Simple, minimalistic Quantum Atom containing the position of the atom
   * center and the atomic number.
   */
  struct QAtom
  {
    Eigen::Vector3d pos;
    int num;
  };

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

    /**
     * Calculate the value at the supplied point in the supplied MO.
     * @param pos The position to calculate the MO at.
     * @param state The MO number to use.
     */
    double calculateMO(const Eigen::Vector3d& pos, unsigned int state = 1);

    /**
     * Calculate the MO over the entire range of the supplied Cube.
     * @param cube The cube to write the values of the MO into.
     * @return True if the calculation was successful.
     */
    bool calculateCubeMO(Cube *cube, unsigned int state = 1);

    /**
     * Calculate the MO over the entire range of the supplied Cube.
     * @param cube The cube to write the values of the MO into.
     * @return True if the calculation was successful.
     */
    bool calculateCubeMO2(Cube *cube, unsigned int state = 1);

    /**
     * When performing a calculation the QFutureWatcher is useful if you want
     * to update a progress bar.
     */
    QFutureWatcher< std::vector<double> > & watcher() { return m_watcher; }

    /**
     * When performing a calculation the QFutureWatcher is useful if you want
     * to update a progress bar.
     */
    QFutureWatcher<void> & watcher2() { return m_watcher2; }

  private Q_SLOTS:
    /**
     * Slot to set the cube data once Qt Concurrent is done
     */
     void calculationComplete();

     /**
     * Slot to set the cube data once Qt Concurrent is done
     */
     void calculationComplete2();

  Q_SIGNALS:

  private:
    std::vector<GTO *> m_GTOs;
    std::vector<Basis *> m_basis;
    std::vector< std::vector<Basis *> > m_shells;
    std::vector<QAtom *> m_atoms;
    std::vector<double> m_MOs; // These are the LCAO contributions
    std::vector<double> m_c;   // These are the normalised contraction coeffs

    unsigned int m_MO;  // The current MO
    unsigned int m_cPos; // The current c position
    unsigned int m_numMOs; // The number of GTOs
    unsigned int m_numCs; // Number of contraction coefficients
    unsigned int m_electrons; // Total number of electrons
    unsigned int m_numAtoms;  // Total number of atoms in the basis set
    bool m_init; // Has the calculation been initialised?

    QFuture< std::vector<double> > m_future;
    QFuture<void> m_future2;
    QFutureWatcher< std::vector<double> > m_watcher;
    QFutureWatcher<void> m_watcher2;
    Cube *m_cube; // Cube to put the results into
    QVector<BasisShell> *m_basisShells;

    static bool isSmall(double val);

    void initCalculation();  // Perform initialisation when necessary
    double processBasis(const Basis* basis, const Eigen::Vector3d& delta,
      double dr2);
    double doS(const Basis* basis, double dr2);
    double doP(const Basis* basis, const Eigen::Vector3d& delta, double dr2);
    double doD(const Basis* basis, const Eigen::Vector3d& delta, double dr2);
    double doD5(const Basis* basis, const Eigen::Vector3d& delta, double dr2);

    /// These are the re-entrant, entire cube forms of the calculations
    static std::vector<double> processShell(const BasisShell &shell);
    static void reduceShells(std::vector<double> &result, const std::vector<double> &add);
    static void cubeS(BasisSet *set, std::vector<double> &vals, const Basis* basis,
                      const std::vector<double> &dr2, unsigned int indexMO);
    static void cubeP(BasisSet *set, std::vector<double> &vals, const Basis* basis,
                      const std::vector<Eigen::Vector3d> &delta,
                      const std::vector<double> &dr2, unsigned int indexMO);
    static void cubeD(BasisSet *set, std::vector<double> &vals, const Basis* basis,
                      const std::vector<Eigen::Vector3d> &delta,
                      const std::vector<double> &dr2, unsigned int indexMO);
    static void cubeD5(BasisSet *set, std::vector<double> &vals, const Basis* basis,
                      const std::vector<Eigen::Vector3d> &delta,
                      const std::vector<double> &dr2, unsigned int indexMO);

    /// Re-entrant single point forms of the calculations
    static void processPoint(BasisShell &shell);
    static double pointS(BasisSet *set, const Basis* basis,
                      const double &dr2, unsigned int indexMO);
    static double pointP(BasisSet *set, const Basis* basis,
                      const Eigen::Vector3d &delta,
                      const double &dr2, unsigned int indexMO);
    static double pointD(BasisSet *set, const Basis* basis,
                      const Eigen::Vector3d &delta,
                      const double &dr2, unsigned int indexMO);
    static double pointD5(BasisSet *set, const Basis* basis,
                      const Eigen::Vector3d &delta,
                      const double &dr2, unsigned int indexMO);
  };

} // End namespace Avogadro

#endif
