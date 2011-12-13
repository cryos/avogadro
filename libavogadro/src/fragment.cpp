/**********************************************************************
  Fragment - Fragment class derived from the base Primitive class

  Copyright (C) 2008 Marcus D. Hanwell

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.openmolecules.net/>

  Avogadro is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  Avogadro is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.
 **********************************************************************/

#include "fragment.h"

#include "molecule.h"

namespace Avogadro {

  Fragment::Fragment(QObject *parent) : Primitive(FragmentType, parent)
  {
    m_molecule = static_cast<Molecule*>(parent);
  }

  Fragment::Fragment(Type type, QObject *parent) : Primitive(type, parent)
  {
    m_molecule = static_cast<Molecule*>(parent);
  }

  Fragment::~Fragment()
  { }

  void Fragment::addAtom(unsigned long id)
  {
    if (!m_atoms.contains(id)) {
      m_atoms.push_back(id);
    }
  }

  void Fragment::removeAtom(unsigned long id)
  {
    int index = m_atoms.indexOf(id);
    if (index > -1) {
      m_atoms.removeAt(index);
    }
  }

  QList<unsigned long> Fragment::atoms()
  {
    return m_atoms;
  }

  void Fragment::addBond(unsigned long id)
  {
    if (!m_bonds.contains(id)) {
      m_bonds.push_back(id);
    }
  }

  void Fragment::removeBond(unsigned long id)
  {
    int index = m_bonds.indexOf(id);
    if (index > -1) {
      m_bonds.removeAt(index);
    }
  }

  QList<unsigned long> Fragment::bonds()
  {
    return m_bonds;
  }

} // End namespace Avogadro
