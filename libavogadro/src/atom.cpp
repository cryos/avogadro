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
    AtomPrivate(): assignedFormalCharge(false) {}
    bool assignedFormalCharge;
  };

  Atom::Atom(QObject *parent) : Primitive(AtomType, parent),
                                d_ptr(new AtomPrivate),
                                m_atomicNumber(0),
                                m_residue(FALSE_ID), m_partialCharge(0.0),
                                m_formalCharge(0),
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
  }

  void Atom::addBond(Bond* bond)
  {
    if (bond)
      addBond(bond->id());
  }

  void Atom::addBond(unsigned long bond)
  {
    // Ensure that only unique bonds are added to the list
    if (m_bonds.indexOf(bond) == -1)
      m_bonds.push_back(bond);
    else
      // Should never happen - warn if it does...
      qDebug() << "Atom" << m_id << "tried to add duplicate bond" << bond;
  }

  void Atom::removeBond(Bond* bond)
  {
    if (bond)
      removeBond(bond->id());
  }

  void Atom::removeBond(unsigned long bond)
  {
    int index = m_bonds.indexOf(bond);
    if (index >= 0)
      m_bonds.removeAt(index);
  }

  QList<unsigned long> Atom::neighbors() const
  {
    if (m_molecule && m_bonds.size()) {
      QList<unsigned long> list;
      foreach(unsigned long id, m_bonds) {
        const Bond *bond = m_molecule->bondById(id);
        if (bond)
          list.push_back(bond->otherAtom(m_id));
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
    else
      return 0.0;
  }

   void Atom::setFormalCharge(int charge)
   {
     Q_D(Atom);
     d->assignedFormalCharge = true;
     m_formalCharge = charge;
   }

   int Atom::formalCharge() const
   {
     Q_D(const Atom);
     if (d->assignedFormalCharge)
       return m_formalCharge;

     // gotta guess it from bonding
     int valenceE = 0;
     int atomicNum = atomicNumber(); // save keystrokes
     if (atomicNum <= 2)
       valenceE = atomicNum;
     else if (atomicNum <= 10)
       valenceE = atomicNum - 2;
     else if (atomicNum <= 18)
       valenceE = atomicNum - 10;
     else if (atomicNum <= 20)
       valenceE = atomicNum - 18;
     else if (atomicNum > 30 && atomicNum <= 36)
       valenceE = atomicNum - 28;
     else if (atomicNum == 37 || atomicNum == 38)
       valenceE = atomicNum - 36;
     else if (atomicNum > 48 && atomicNum <= 54)
       valenceE = atomicNum - 46;
     else if (atomicNum == 55 || atomicNum == 56)
       valenceE = atomicNum - 54;
     else if (atomicNum > 80 && atomicNum <= 86)
       valenceE = atomicNum - 78;
     else if (atomicNum == 87 || atomicNum == 88)
       valenceE = atomicNum - 86;
     else
       return 0; // I don't quite know what to do for TM or other elements
	
     int formalcharge = 0;
     int totalBonds = 0;
     foreach(unsigned long id, m_bonds) {
       const Bond *bond = m_molecule->bondById(id);
       if (bond)
         totalBonds += bond->order();
     }

     int fullShell = 8;
     int loneE = 0;
     // Work out lone pairs: special cases for hypervalent S, P, Br, I (i.e., for VSEPR exercises)
     if (atomicNum == 16 || atomicNum == 34 || atomicNum == 52 || atomicNum == 84) { // Sulfur, Se, Te, ...
       if ((totalBonds - valenceE) % 2 == 0)
         loneE = valenceE - totalBonds;
       else if (totalBonds == 1)
         loneE = 6;
       else if (totalBonds == 3)
         loneE = 2;
       else if (totalBonds == 5)
         loneE = 0;
     }
     else if (atomicNum == 15 || atomicNum == 33 || atomicNum == 51 || atomicNum == 83) { // P, As, ...
       if (totalBonds == 1)
         loneE = 6;
       else if (totalBonds == 2)
         loneE = 4;
       else if (totalBonds == 3)
         loneE = 2;
       else
         loneE = 0;
     }
     else { // all other elements
       if (totalBonds < valenceE)
         loneE = fullShell - (2*totalBonds);
     }
		
     formalcharge = valenceE - (totalBonds + loneE);
	
     return formalcharge;
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
    obatom.SetFormalCharge(m_formalCharge);

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
    if (obatom->GetFormalCharge() != 0)
      m_formalCharge = obatom->GetFormalCharge();

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
    if (other.pos())
      m_molecule->setAtomPos(m_id, *other.pos());
    else
      qDebug() << "Atom position returned null.";
    m_atomicNumber = other.m_atomicNumber;
    m_formalCharge = other.m_formalCharge;
    return *this;
  }

} // End namespace Avogadro

#include "atom.moc"
