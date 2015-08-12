/******************************************************************************

  This source file is part of the OpenQube project.

  Copyright 2011 Kitware, Inc.

  This source code is released under the New BSD License, (the "License").

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

******************************************************************************/

#ifndef OQ_MOLECULE_H
#define OQ_MOLECULE_H

#include "config.h"

#include "openqubeabi.h"

#include "atom.h"

#include <vector>
#include <Eigen/Core>

namespace OpenQube {

class Atom;

class OPENQUBE_EXPORT Molecule
{
public:
  /** Default contructor to create an empty molecule object. */
  Molecule();
  virtual ~Molecule();

  /** Add a new atom to the molecule, return a pointer to it.
   * @param pos Initial position, defaults to 0, 0, 0.
   * @param atomicNumber Atomic (proton) number, defaults to 0 (dummy atom).
   * @note The atom belongs to the molecule.
   */
  Atom addAtom(const Eigen::Vector3d &pos = Eigen::Vector3d::Zero(),
               short atomicNumber = 0);

  /** Get a pointer to the specified atom.
   * @param index The index of the atom in the molecule.
   * @return Pointer to the Atom, or 0 if the index is invalid.
   */
  const Atom atom(size_t index) const;
  Atom atom(size_t index);

  /** Get the atomic number for the supplied atom index.
   * @param index The index of the atom in the molecule.
   * @return The atomic number of the atom.
   */
  short atomAtomicNumber(size_t index) const;

  /** Set the atomic number for the supplied atom index.
   * @param index The index of the atom in the molecule.
   * @param atomicNumber The atomic number to set for the atom.
   */
  void setAtomAtomicNumber(size_t index, short atomicNumber);

  /** Get the position of the specified atom.
   * @param index The index of the atom you want the position of.
   * @return Position of the atom, or a Zero vector if the index is invalid.
   */
  Eigen::Vector3d atomPos(size_t index) const;

  /** Set the position of the specified atom.
   * @param index The index of the atom to set the position of.
   * @param pos The position to set it to.
   */
  void setAtomPos(size_t index, const Eigen::Vector3d& pos);

  /**
   * Clear all atoms from the molecule.
   */
  void clearAtoms();

  /**
   * @return The total number of atoms in the molecule.
   */
  size_t numAtoms() const { return m_atomicNumbers.size(); }

  /**
   * Convenience function: print out a summary of the molecule.
   */
  void print() const;

protected:
  /** Container of all atomic numbers for Atom objects in the molecule. */
  std::vector<short> m_atomicNumbers;
  /** Container of all atomic coordinates for the molecule. */
  std::vector<std::vector<Eigen::Vector3d> > m_atomPositions;
  /** The current conformer, defaults to 0 in a standard molecule. */
  size_t m_conformer;
};

} // End namespace

#endif
