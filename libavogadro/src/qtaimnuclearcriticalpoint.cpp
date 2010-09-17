/**********************************************************************
  Atom - Atom class derived from the base Primitive class

  Copyright (C) 2007 Donald Ephraim Curtis
  Copyright (C) 2008 Marcus D. Hanwell
  Copyright (c) 2010 Konstantin Tokarev

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

#include "qtaimnuclearcriticalpoint.h"
#include "molecule.h"
#include "qtaimbondcriticalpoint.h"
#include "residue.h"

#include <QVariant>
#include <QDebug>

using Eigen::Vector3d;

 namespace Avogadro {

   class QTAIMNuclearCriticalPointPrivate {
   public:
     QTAIMNuclearCriticalPointPrivate(): assignedFormalCharge(false) {}
     bool assignedFormalCharge;
   };

   QTAIMNuclearCriticalPoint::QTAIMNuclearCriticalPoint(QObject *parent) : Primitive(QTAIMNuclearCriticalPointType, parent),
                                 d_ptr(new QTAIMNuclearCriticalPointPrivate),
                                 m_atomicNumber(0),
                                 m_residue(FALSE_ID), m_partialCharge(0.0),
                                 m_formalCharge(0),
                                 m_forceVector(0.0, 0.0, 0.0),
                                 m_customLabel(""), m_customColorName("Purple"),
                                 m_customRadius(0.1)
   {
     if (!parent) {
       qDebug() << "I am an orphaned NCP! I feel so invalid...";
     }
     m_molecule = static_cast<Molecule*>(parent);
   }

   QTAIMNuclearCriticalPoint::~QTAIMNuclearCriticalPoint()
   {
   }

   const Eigen::Vector3d * QTAIMNuclearCriticalPoint::pos() const
   {
     return m_molecule->nuclearCriticalPointPos(m_id);
   }

   void QTAIMNuclearCriticalPoint::setPos(const Eigen::Vector3d &vec)
   {
     m_molecule->setNuclearCriticalPointPos(m_id, vec);
   }

   void QTAIMNuclearCriticalPoint::setAtomicNumber(int num)
   {
     m_atomicNumber = num;
     update(); // signal that the element has changed, to update residues
   }

   void QTAIMNuclearCriticalPoint::addBondCriticalPoint(QTAIMBondCriticalPoint* bondCriticalPoint)
   {
     if (bondCriticalPoint)
       addBondCriticalPoint(bondCriticalPoint->id());
   }

   void QTAIMNuclearCriticalPoint::addBondCriticalPoint(unsigned long bondCriticalPoint)
   {
     // Ensure that only unique BCPs are added to the list
     if (m_bondCriticalPoints.indexOf(bondCriticalPoint) == -1)
       m_bondCriticalPoints.push_back(bondCriticalPoint);
     else
       // Should never happen - warn if it does...
       qDebug() << "QTAIMNuclearCriticalPoint" << m_id << "tried to add duplicate BCP" << bondCriticalPoint;
   }

   void QTAIMNuclearCriticalPoint::removeBondCriticalPoint(QTAIMBondCriticalPoint* bondCriticalPoint)
   {
     if (bondCriticalPoint)
       removeBondCriticalPoint(bondCriticalPoint->id());
   }

   void QTAIMNuclearCriticalPoint::removeBondCriticalPoint(unsigned long bondCriticalPoint)
   {
     int index = m_bondCriticalPoints.indexOf(bondCriticalPoint);
     if (index >= 0)
       m_bondCriticalPoints.removeAt(index);
   }

   QList<unsigned long> QTAIMNuclearCriticalPoint::neighbors() const
   {
     if (m_molecule && m_bondCriticalPoints.size()) {
       QList<unsigned long> list;
       foreach(unsigned long id, m_bondCriticalPoints) {
         const QTAIMBondCriticalPoint *bondCriticalPoint = m_molecule->bondCriticalPointById(id);
         if (bondCriticalPoint)
           list.push_back(bondCriticalPoint->otherNuclearCriticalPoint(m_id));
       }
       return list;
     }
     return QList<unsigned long>();
   }

   QTAIMBondCriticalPoint * QTAIMNuclearCriticalPoint::bondCriticalPoint(const QTAIMNuclearCriticalPoint *other) const
   {
     return m_molecule->bondCriticalPoint(this, other);
   }

   double QTAIMNuclearCriticalPoint::partialCharge() const
   {
     if (m_molecule && m_atomicNumber) {
       m_molecule->calculatePartialCharges();
       return m_partialCharge;
     }
     else
       return 0.0;
   }

   void QTAIMNuclearCriticalPoint::setFormalCharge(int charge)
   {
     Q_D(QTAIMNuclearCriticalPoint);
     d->assignedFormalCharge = true;
     m_formalCharge = charge;
   }

   int QTAIMNuclearCriticalPoint::formalCharge() const
   {
     Q_D(const QTAIMNuclearCriticalPoint);
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
     foreach(unsigned long id, m_bondCriticalPoints) {
       const QTAIMBondCriticalPoint *bondCriticalPoint = m_molecule->bondCriticalPointById(id);
       if (bondCriticalPoint)
         totalBonds += bondCriticalPoint->order();
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

   void QTAIMNuclearCriticalPoint::setResidue(unsigned long id)
   {
     m_residue = id;
   }

   void QTAIMNuclearCriticalPoint::setResidue(const Residue *residue)
   {
     m_residue = residue->id();
   }

   void QTAIMNuclearCriticalPoint::setGroupIndex(unsigned int index)
   {
	 m_groupIndex = index;
   }

   unsigned long QTAIMNuclearCriticalPoint::residueId() const
   {
     return m_residue;
   }

   Residue * QTAIMNuclearCriticalPoint::residue() const
   {
     return m_molecule->residueById(m_residue);
   }

   QTAIMNuclearCriticalPoint& QTAIMNuclearCriticalPoint::operator=(const QTAIMNuclearCriticalPoint& other)
   {
     // Virtually everything here is invariant apart from the index and possibly id
     if (other.pos())
       m_molecule->setNuclearCriticalPointPos(m_id, *other.pos());
     else
       qDebug() << "NCP position returned null.";
     m_atomicNumber = other.m_atomicNumber;
     m_formalCharge = other.m_formalCharge;
     m_customLabel = other.m_customLabel;
     m_customColorName = other.m_customColorName;
     m_customRadius = other.m_customRadius;
     return *this;
   }

 } // End namespace Avogadro

#include "qtaimnuclearcriticalpoint.moc"
