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

#include "molecule.h"
#include "atom.h"

#include <QtCore/QDebug>

using Eigen::Vector3d;

namespace OpenQube {

Molecule::Molecule() : m_atomPositions(1), m_conformer(0)
{
}

Molecule::~Molecule()
{
}

Atom Molecule::addAtom(const Eigen::Vector3d &pos, short atomicNumber)
{
  // Increment the vectors containing atomic properties.
  m_atomicNumbers.push_back(atomicNumber);
  for (size_t i = 0; i < m_atomPositions.size(); ++i)
    m_atomPositions[i].push_back(pos);
  return Atom(this, m_atomicNumbers.size() - 1);
}

const Atom Molecule::atom(size_t index) const
{
  // The const_cast is needed to create the temporary. As we are returning
  // a const Atom object and Atom is const safe this should be fine.
  if (index < m_atomicNumbers.size())
    return Atom(const_cast<Molecule *>(this), index);
  else
    return Atom(0, 0);
}

Atom Molecule::atom(size_t index)
{
  if (index < m_atomicNumbers.size())
    return Atom(this, index);
  else
    return Atom(0, 0);
}

short Molecule::atomAtomicNumber(size_t index) const
{
  if (index < m_atomicNumbers.size())
    return m_atomicNumbers[index];
  else
    return 0;
}

void Molecule::setAtomAtomicNumber(size_t index, short atomicNumber)
{
  if (index < m_atomicNumbers.size())
    m_atomicNumbers[index] = atomicNumber;
}

Vector3d Molecule::atomPos(size_t atomIndex) const
{
  if (atomIndex < m_atomPositions[m_conformer].size())
    return m_atomPositions[m_conformer][atomIndex];
  else
    return Vector3d::Zero();
}

void Molecule::setAtomPos(size_t atomIndex, const Eigen::Vector3d& pos)
{
  if (atomIndex < m_atomPositions[m_conformer].size())
    m_atomPositions[m_conformer][atomIndex] = pos;
}

void Molecule::clearAtoms()
{
  m_atomicNumbers.clear();
  m_atomPositions.resize(1);
  m_atomPositions[0].clear();
}

void Molecule::print() const
{
  qDebug() << "Number of atoms:" << m_atomicNumbers.size();
  for (size_t i = 0; i < m_atomicNumbers.size(); ++i) {
    qDebug() << "Atom" << i << ":" << m_atomicNumbers[i]
             << "->" << m_atomPositions[0][i].x() << m_atomPositions[0][i].y()
             << m_atomPositions[0][i].z();
  }
}

} // End namespace
