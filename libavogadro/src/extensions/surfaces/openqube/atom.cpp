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

#include "atom.h"

using Eigen::Vector3d;

namespace OpenQube {

short Atom::atomicNumber() const
{
  return m_molecule ? m_molecule->atomAtomicNumber(m_index) : -1;
}

bool Atom::isHydrogen() const
{
  return m_molecule ? m_molecule->atomAtomicNumber(m_index) == 1 : false;
}

void Atom::setAtomicNumber(short atomicNumber)
{
  if (m_molecule)
    m_molecule->setAtomAtomicNumber(m_index, atomicNumber);
}

Vector3d Atom::pos() const
{
  return m_molecule ? m_molecule->atomPos(m_index) : Vector3d::Zero();
}

void Atom::setPos(const Eigen::Vector3d& pos)
{
  if (m_molecule)
    m_molecule->setAtomPos(m_index, pos);
}

} // End namespace
