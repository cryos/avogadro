/**********************************************************************
  Bond - Bond class derived from the base Primitive class

  Copyright (C) 2007 Donald Ephraim Curtis
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

 #include "bond.h"

 #include "molecule.h"
 #include "atom.h"

 #include <openbabel/mol.h>

 #include <QDebug>

 namespace Avogadro{

  class BondPrivate {
    public:
      BondPrivate() {}
  };

  Bond::Bond(QObject *parent) : Primitive(BondType, parent),
    m_beginAtomId(FALSE_ID), m_endAtomId(FALSE_ID), m_order(1),
    m_isAromatic(false), m_customLabel("")
  {
    m_molecule = static_cast<Molecule*>(parent);
    m_id = ULONG_MAX;
  }

  Bond::~Bond()
  {
  }

  void Bond::setBegin(Atom* atom)
  {
    if (m_beginAtomId != FALSE_ID) {
      Atom *a = m_molecule->atomById(m_beginAtomId);
      if (a) a->removeBond(this);
    }
    m_beginAtomId = atom->id();
    atom->addBond(this);
  }

  Atom * Bond::beginAtom() const
  {
    return m_molecule->atomById(m_beginAtomId);
  }

  void Bond::setEnd(Atom* atom)
  {
    if (m_endAtomId != FALSE_ID) {
      Atom *a = m_molecule->atomById(m_endAtomId);
      if (a) a->removeBond(this);
    }
    m_endAtomId = atom->id();
    atom->addBond(this);
  }

  Atom * Bond::endAtom() const
  {
    return m_molecule->atomById(m_endAtomId);
  }

  void Bond::setAtoms(unsigned long atom1, unsigned long atom2,
                      short order)
  {
    Atom *atom = m_molecule->atomById(atom1);
    if (atom) {
      m_beginAtomId = atom1;
      atom->addBond(m_id);
    }
    else {
      qDebug() << "Non-existent atom:" << atom1;
    }
    atom = m_molecule->atomById(atom2);
    if (atom) {
      m_endAtomId = atom2;
      atom->addBond(m_id);
    }
    else {
      qDebug() << "Non-existent atom:" << atom2;
    }
    m_order = order;
  }

  const Eigen::Vector3d * Bond::beginPos() const
  {
    return m_molecule->atomPos(m_beginAtomId);
  }

  const Eigen::Vector3d * Bond::midPos() const
  {
    m_midPos = (*(m_molecule->atomPos(m_beginAtomId)) + *(m_molecule->atomPos(m_endAtomId)) ) / 2.0;
    return &m_midPos;
  }

  const Eigen::Vector3d * Bond::endPos() const
  {
    return m_molecule->atomPos(m_endAtomId);
  }

  unsigned long Bond::otherAtom(unsigned long atomId) const
  {
    if (atomId == m_beginAtomId) {
      return m_endAtomId;
    }
    else {
      return m_beginAtomId;
    }
  }

  bool Bond::isAromatic() const
  {
    if (m_molecule && m_order) {
      m_molecule->calculateAromaticity();
      return m_isAromatic;
    }
    else {
      return false;
    }
  }

  void Bond::setAromaticity(bool isAromatic) const
  {
    m_isAromatic = isAromatic;
  }

  double Bond::length() const
  {
    return (*m_molecule->atomById(m_endAtomId)->pos()
            - *m_molecule->atomById(m_beginAtomId)->pos()).norm();
  }

  bool Bond::setOBBond(OpenBabel::OBBond *obbond)
  {
    m_order = obbond->GetBondOrder();
    return true;
  }

  Bond& Bond::operator=(const Bond& other)
  {
    m_beginAtomId = other.m_beginAtomId;
    m_endAtomId = other.m_endAtomId;
    m_order = other.m_order;
    return *this;
  }

} // End namespace Avogadro

#include "bond.moc"
