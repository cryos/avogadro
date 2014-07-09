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

#ifndef OQ_ATOM_H
#define OQ_ATOM_H

#include "config.h"

#include "molecule.h"

#include <Eigen/Core>

namespace OpenQube {

class Molecule;

class OPENQUBE_EXPORT Atom
{
public:
  /** Is the Atom valid?
   * @return True if it is a valid object, false if not.
   */
  bool isValid() const { return m_molecule != 0; }

  /** Get the atomic number. */
  short atomicNumber() const;

  /** Set the atomic number. */
  void setAtomicNumber(short atomicNumber);

  /** Convenience function
   * @return True is this atom is a hydrogen atom.
   */
  bool isHydrogen() const;

  /** Get the position of the specified atom. */
  Eigen::Vector3d pos() const;

  /** Set the position of the specified atom.
   * @param pos The position to set it to.
   */
  void setPos(const Eigen::Vector3d& pos);

protected:
  friend class Molecule;
  /** Default contructor to create an empty atom object.
   * @param parent The parent molecule, atoms must belong to a molecule.
   * @param index The index of the atom in the parent molecule.
   */
  Atom(Molecule *parent, size_t index);

  /** The index of this atom. */
  size_t m_index;
  /** The parent molecule of this atom. */
  Molecule *m_molecule;
};

inline Atom::Atom(Molecule *molecule, size_t index) : 
  m_index(index),
  m_molecule(molecule)
{
}

} // End namespace

#endif
