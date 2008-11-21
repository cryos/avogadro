/**********************************************************************
  Atom - Atom class derived from the base Primitive class

  Copyright (C) 2007 Donald Ephraim Curtis
  Copyright (C) 2008 Marcus D. Hanwell

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.sourceforge.net/>

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

#include "atom.h"

#include "bond.h"

#include <openbabel/mol.h>

 namespace Avogadro{

  class AtomPrivate {
    public:
      AtomPrivate() {}
  };

  Atom::Atom(QObject *parent) : Primitive(AtomType, parent), m_pos(0., 0., 0.),
    m_atomicNum(0)
  {
  }

  void Atom::addBond(Bond* bond)
  {
    m_bonds.push_back(bond->id());
    // Update the neighbors list
    if (bond->beginAtomId() == id())
      m_neighbors.push_back(bond->endAtomId());
    else
      m_neighbors.push_back(bond->beginAtomId());
  }

  void Atom::deleteBond(Bond* bond)
  {
    int index = m_bonds.indexOf(bond->id());
    if (index >= 0)
      m_bonds.removeAt(index);

    // Update the neighbors list too
    if (bond->beginAtomId() == id())
      m_neighbors.removeAt(m_neighbors.indexOf(bond->endAtomId()));
    else
      m_neighbors.removeAt(m_neighbors.indexOf(bond->beginAtomId()));
  }

  OpenBabel::OBAtom Atom::OBAtom()
  {
    // Need to copy all relevant data over to the OBAtom
    OpenBabel::OBAtom obatom;
    obatom.SetVector(m_pos.x(), m_pos.y(), m_pos.z());
    obatom.SetAtomicNum(m_atomicNum);

    return obatom;
  }

  bool Atom::setOBAtom(OpenBabel::OBAtom *obatom)
  {
    // Copy all needed OBAtom data to our atom
    m_pos = Eigen::Vector3d(obatom->x(), obatom->y(), obatom->z());
    m_atomicNum = obatom->GetAtomicNum();
    m_partialCharge = obatom->GetPartialCharge();
    update();
    return true;
  }

  Atom& Atom::operator=(const Atom& other)
  {
    // Virtually everything here is invariant apart from the index and possibly id
    m_pos = other.m_pos;
    m_atomicNum = other.m_atomicNum;
    m_bonds = other.m_bonds;
    return *this;
  }

} // End namespace Avogadro

#include "atom.moc"
