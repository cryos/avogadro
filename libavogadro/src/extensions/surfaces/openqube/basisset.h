/******************************************************************************

  This source file is part of the OpenQube project.

  Copyright 2008-2010 Marcus D. Hanwell
  Copyright 2010 David C. Lonie

  This source code is released under the New BSD License, (the "License").

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

******************************************************************************/

#ifndef BASISSET_H
#define BASISSET_H

#include "openqubeabi.h"

#include "molecule.h"

#include <QtCore/QObject>
#include <QtCore/QFutureWatcher>

namespace OpenQube
{

/**
 * @class BasisSet gaussianset.h <openqube/basissetloader.h>
 * @brief BasisSet contains basis set data, calculates cubes.
 * @author Marcus D. Hanwell
 *
 * This is the base class for basis sets, and has two derived classes -
 * GaussianSet and SlaterSet. It must be populated with data, and can then be
 * used to calculate values of the basis set in a cube.
 */

class Cube;

class OPENQUBE_EXPORT BasisSet : public QObject
{
  Q_OBJECT

public:
  /**
   * Constructor.
   */
  BasisSet() : m_electrons(0), m_valid(true) {}

  /**
   * Destructor.
   */
  virtual ~BasisSet() {}

  /**
   * Set the number of electrons in the BasisSet.
   * @param n The number of electrons in the BasisSet.
   */
  void setNumElectrons(unsigned int n) { m_electrons = n; }

  /**
   * @return The number of electrons in the molecule.
   */
  unsigned int numElectrons() { return m_electrons; }

  /**
   * Set the molecule for the basis set.
   */
  void setMolecule(const Molecule &molecule) { m_molecule = molecule; }

  /**
   * Get the molecule for the basis set.
   */
  Molecule molecule() const { return m_molecule; }

  /**
   * Get a reference to the molecule.
   */
  Molecule & moleculeRef() { return m_molecule; }
  const Molecule & moleculeRef() const { return m_molecule; }

  /**
   * @return The number of MOs in the BasisSet.
   */
  virtual unsigned int numMOs() = 0;

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
   * Set the number of electrons in the BasisSet.
   * @param valid True if the basis set is valid, false otherwise.
   */
  void setIsValid(bool valid) { m_valid = valid; }

  /**
   * @return True of the basis set is valid, false otherwise.
   * Default is true, if false then the basis set is likely unusable.
   */
  bool isValid() { return m_valid; }

  /**
   * Calculate the MO over the entire range of the supplied Cube.
   * @param cube The cube to write the values of the MO into.
   * @param mo The molecular orbital number to calculate.
   * @note This function starts a threaded calculation. Use watcher() to
   * monitor progress.
   * @sa blockingCalculateCubeMO
   * @return True if the calculation was successful.
   */
  virtual bool calculateCubeMO(Cube *cube, unsigned int mo = 1) = 0;

  /**
   * Calculate the MO over the entire range of the supplied Cube.
   * @param cube The cube to write the values of the MO into.
   * @param mo The molecular orbital number to calculate.
   * @sa calculateCubeMO
   * @return True if the calculation was successful.
   */
  virtual bool blockingCalculateCubeMO(Cube *cube, unsigned int mo = 1);

  /**
   * Calculate the electron density over the entire range of the supplied Cube.
   * @param cube The cube to write the values of the MO into.
   * @note This function starts a threaded calculation. Use watcher() to
   * monitor progress.
   * @sa blockingCalculateCubeDensity
   * @return True if the calculation was successful.
   */
  virtual bool calculateCubeDensity(Cube *cube) = 0;

  /**
   * Calculate the electron density over the entire range of the supplied Cube.
   * @param cube The cube to write the values of the MO into.
   * @sa calculateCubeDensity
   * @return True if the calculation was successful.
   */
  virtual bool blockingCalculateCubeDensity(Cube *cube);

  /**
   * When performing a calculation the QFutureWatcher is useful if you want
   * to update a progress bar.
   */
  virtual QFutureWatcher<void> & watcher()=0;

  /**
   * Create a deep copy of @a this and return a pointer to it.
   */
  virtual BasisSet * clone() = 0;

protected:
  /// Total number of electrons
  unsigned int m_electrons;

  /** Is the loaded basis set valid? Allows us to mark a basis set invalid if we
   * were not able to interpret part of it.
   */
  bool m_valid;

  /** The Molecule holds the atoms (and possibly bonds) read in from the output
   * file. Most basis sets have orbitals around these atoms, but this is not
   * necessarily the case.
   */
  Molecule m_molecule;

};

} // End namespace openqube

#endif
