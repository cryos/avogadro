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

 #include "qtaimbondcriticalpoint.h"

 #include "molecule.h"
 #include "qtaimnuclearcriticalpoint.h"

 #include <QDebug>

 namespace Avogadro{

  class QTAIMBondCriticalPointPrivate {
    public:
      QTAIMBondCriticalPointPrivate() {}
  };

  QTAIMBondCriticalPoint::QTAIMBondCriticalPoint(QObject *parent) : Primitive(BondType, parent),
    m_beginNuclearCriticalPointId(FALSE_ID), m_endNuclearCriticalPointId(FALSE_ID), m_order(1),
    m_isAromatic(false), m_customLabel(""), m_customColorName("Yellow"), m_customRadius(0.1)
  {
    m_molecule = static_cast<Molecule*>(parent);
    m_id = ULONG_MAX;
  }

  QTAIMBondCriticalPoint::~QTAIMBondCriticalPoint()
  {
  }

  const Eigen::Vector3d * QTAIMBondCriticalPoint::pos() const
  {
    return m_molecule->bondCriticalPointPos(m_id);
  }

  void QTAIMBondCriticalPoint::setPos(const Eigen::Vector3d &vec)
  {
    m_molecule->setBondCriticalPointPos(m_id, vec);
  }

  void QTAIMBondCriticalPoint::setBegin(QTAIMNuclearCriticalPoint* nuclearCriticalPoint)
  {
    if (m_beginNuclearCriticalPointId != FALSE_ID) {
      QTAIMNuclearCriticalPoint *ncp = m_molecule->nuclearCriticalPointById(m_beginNuclearCriticalPointId);
      if (ncp) ncp->removeBondCriticalPoint(this);
    }
    m_beginNuclearCriticalPointId = nuclearCriticalPoint->id();
    nuclearCriticalPoint->addBondCriticalPoint(this);
  }

  QTAIMNuclearCriticalPoint * QTAIMBondCriticalPoint::beginNuclearCriticalPoint() const
  {
    return m_molecule->nuclearCriticalPointById(m_beginNuclearCriticalPointId);
  }

  void QTAIMBondCriticalPoint::setEnd(QTAIMNuclearCriticalPoint* nuclearCriticalPoint)
  {
    if (m_endNuclearCriticalPointId != FALSE_ID) {
      QTAIMNuclearCriticalPoint *ncp = m_molecule->nuclearCriticalPointById(m_endNuclearCriticalPointId);
      if (ncp) ncp->removeBondCriticalPoint(this);
    }
    m_endNuclearCriticalPointId = nuclearCriticalPoint->id();
    nuclearCriticalPoint->addBondCriticalPoint(this);
  }

  QTAIMNuclearCriticalPoint * QTAIMBondCriticalPoint::endNuclearCriticalPoint() const
  {
    return m_molecule->nuclearCriticalPointById(m_endNuclearCriticalPointId);
  }

  void QTAIMBondCriticalPoint::setNuclearCriticalPoints(unsigned long nuclearCriticalPoint1, unsigned long nuclearCriticalPoint2,
                      short order)
  {
    QTAIMNuclearCriticalPoint *nuclearCriticalPoint = m_molecule->nuclearCriticalPointById(nuclearCriticalPoint1);
    if (nuclearCriticalPoint) {
      m_beginNuclearCriticalPointId = nuclearCriticalPoint1;
      nuclearCriticalPoint->addBondCriticalPoint(m_id);
    }
    else {
      qDebug() << "Non-existent nuclear critical point:" << nuclearCriticalPoint1;
    }
    nuclearCriticalPoint = m_molecule->nuclearCriticalPointById(nuclearCriticalPoint2);
    if (nuclearCriticalPoint) {
      m_endNuclearCriticalPointId = nuclearCriticalPoint2;
      nuclearCriticalPoint->addBondCriticalPoint(m_id);
    }
    else {
      qDebug() << "Non-existent nuclear critical point:" << nuclearCriticalPoint2;
    }
    m_order = order;
  }

  const Eigen::Vector3d * QTAIMBondCriticalPoint::beginPos() const
  {
    return m_molecule->nuclearCriticalPointPos(m_beginNuclearCriticalPointId);
  }

  const Eigen::Vector3d * QTAIMBondCriticalPoint::midPos() const
  {
    m_midPos = (*(m_molecule->nuclearCriticalPointPos(m_beginNuclearCriticalPointId)) + *(m_molecule->nuclearCriticalPointPos(m_endNuclearCriticalPointId)) ) / 2.0;
    return &m_midPos;
  }

  const Eigen::Vector3d * QTAIMBondCriticalPoint::endPos() const
  {
    return m_molecule->nuclearCriticalPointPos(m_endNuclearCriticalPointId);
  }

  unsigned long QTAIMBondCriticalPoint::otherNuclearCriticalPoint(unsigned long nuclearCriticalPointId) const
  {
    if (nuclearCriticalPointId == m_beginNuclearCriticalPointId) {
      return m_endNuclearCriticalPointId;
    }
    else {
      return m_beginNuclearCriticalPointId;
    }
  }

  bool QTAIMBondCriticalPoint::isAromatic() const
  {
    if (m_molecule && m_order) {
      m_molecule->calculateAromaticity();
      return m_isAromatic;
    }
    else {
      return false;
    }
  }

  void QTAIMBondCriticalPoint::setAromaticity(bool isAromatic) const
  {
    m_isAromatic = isAromatic;
  }

  double QTAIMBondCriticalPoint::length() const
  {
    return (*m_molecule->nuclearCriticalPointById(m_endNuclearCriticalPointId)->pos()
            - *m_molecule->nuclearCriticalPointById(m_beginNuclearCriticalPointId)->pos()).norm();
  }

  QTAIMBondCriticalPoint& QTAIMBondCriticalPoint::operator=(const QTAIMBondCriticalPoint& other)
  {
    m_beginNuclearCriticalPointId = other.m_beginNuclearCriticalPointId;
    m_endNuclearCriticalPointId = other.m_endNuclearCriticalPointId;
    m_order = other.m_order;
    return *this;
  }

} // End namespace Avogadro

#include "qtaimbondcriticalpoint.moc"
