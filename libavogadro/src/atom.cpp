/**********************************************************************
  Atom - Atom class derived from the base Primitive class

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

#include "atom.h"

#include "molecule.h"
#include "bond.h"
#include "residue.h"

#include <openbabel/atom.h>
#include <openbabel/generic.h>

#include <QVariant>
#include <QDebug>

using Eigen::Vector3d;

 namespace Avogadro{

  class AtomPrivate {
    public:
      AtomPrivate() {}
  };

  Atom::Atom(QObject *parent) : Primitive(AtomType, parent), m_atomicNumber(0),
                                m_residue(FALSE_ID), m_partialCharge(0.0),
                                m_forceVector(0.0, 0.0, 0.0)
  {
    if (!parent) {
      qDebug() << "I am an orphaned atom! I feel so invalid...";
    }
    m_molecule = static_cast<Molecule*>(parent);
  }

  Atom::~Atom()
  {
  }

  const Eigen::Vector3d * Atom::pos() const
  {
    return m_molecule->atomPos(m_id);
  }

  void Atom::setPos(const Eigen::Vector3d &vec)
  {
    m_molecule->setAtomPos(m_id, vec);
    update();
  }

  void Atom::addBond(Bond* bond)
  {
    if (bond) {
      addBond(bond->id());
    }
  }

  void Atom::addBond(unsigned long bond)
  {
    // Ensure that only unique bonds are added to the list
    if (m_bonds.indexOf(bond) == -1) {
      m_bonds.push_back(bond);
    }
    else {
      // Should never happen - warn if it does...
      qDebug() << "Atom" << m_id << "tried to add duplicate bond" << bond;
    }
  }

  void Atom::removeBond(Bond* bond)
  {
    if (bond) {
      removeBond(bond->id());
    }
  }

  void Atom::removeBond(unsigned long bond)
  {
    int index = m_bonds.indexOf(bond);
    if (index >= 0) {
      m_bonds.removeAt(index);
    }
  }

  QList<unsigned long> Atom::neighbors() const
  {
    if (m_molecule && m_bonds.size()) {
      QList<unsigned long> list;
      foreach(unsigned long id, m_bonds) {
        const Bond *bond = m_molecule->bondById(id);
        if (bond) {
          list.push_back(bond->otherAtom(m_id));
        }
      }
      return list;
    }
    return QList<unsigned long>();
  }

  Bond * Atom::bond(const Atom *other) const
  {
    return m_molecule->bond(this, other);
  }

  double Atom::partialCharge() const
  {
    if (m_molecule && m_atomicNumber) {
      m_molecule->calculatePartialCharges();
      return m_partialCharge;
    }
    else {
      return 0.0;
    }
  }

  void Atom::setResidue(unsigned long id)
  {
    m_residue = id;
  }

  void Atom::setResidue(const Residue *residue)
  {
    m_residue = residue->id();
  }

  unsigned long Atom::residueId() const
  {
    return m_residue;
  }

  Residue * Atom::residue() const
  {
    return m_molecule->residueById(m_residue);
  }

  OpenBabel::OBAtom Atom::OBAtom()
  {
    // Need to copy all relevant data over to the OBAtom
    OpenBabel::OBAtom obatom;
    const Vector3d *v = m_molecule->atomPos(m_id);
    obatom.SetVector(v->x(), v->y(), v->z());
    obatom.SetAtomicNum(m_atomicNumber);

    // Add dynamic properties as OBPairData
    OpenBabel::OBPairData *obproperty;
    foreach(const QByteArray &propertyName, dynamicPropertyNames()) {
      obproperty = new OpenBabel::OBPairData;
      obproperty->SetAttribute(propertyName.data());
      obproperty->SetValue(property(propertyName).toByteArray().data());
      obatom.SetData(obproperty);
    }

    return obatom;
  }

  bool Atom::setOBAtom(OpenBabel::OBAtom *obatom)
  {
    // Copy all needed OBAtom data to our atom
    m_molecule->setAtomPos(m_id, Vector3d(obatom->x(), obatom->y(), obatom->z()));
    m_atomicNumber = obatom->GetAtomicNum();
//    m_partialCharge = obatom->GetPartialCharge();

    // And add any generic data as QObject properties
    std::vector<OpenBabel::OBGenericData*> data;
    OpenBabel::OBDataIterator j;
    OpenBabel::OBPairData *property;

    data = obatom->GetAllData(OpenBabel::OBGenericDataType::PairData);
    for (j = data.begin(); j != data.end(); ++j) {
      property = static_cast<OpenBabel::OBPairData *>(*j);
      setProperty(property->GetAttribute().c_str(), property->GetValue().c_str());
    }

    return true;
  }

  Atom& Atom::operator=(const Atom& other)
  {
    // Virtually everything here is invariant apart from the index and possibly id
    if (other.pos()) {
      m_molecule->setAtomPos(m_id, *other.pos());
    }
    else {
      qDebug() << "Atom position returned null.";
    }
    m_atomicNumber = other.m_atomicNumber;
//    m_bonds = other.m_bonds;
    return *this;
  }

} // End namespace Avogadro

#include "atom.moc"
