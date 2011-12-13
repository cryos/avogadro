/**********************************************************************
  Residue - Residue class derived from the base Primitive class

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

#include "residue.h"
#include "molecule.h"
#include "atom.h"

#include <QDebug>

namespace Avogadro {

  Residue::Residue(QObject *parent): Fragment(ResidueType, parent),
    m_chainNumber(0)
  {
  }

  Residue::~Residue()
  {
  }

  void Residue::addAtom(unsigned long id)
  {
    if (!m_molecule) {
      qWarning() << Q_FUNC_INFO << "Parent molecule was not set!";
      return;
    }
    if(!m_molecule->atomById(id)){
      qWarning() << Q_FUNC_INFO << QString("Atom with id=%1 was not found in the parent molecule!").arg(id);
      return;
    }
    if (!m_atoms.contains(id))
      m_atoms.push_back(id);
    m_molecule->atomById(id)->setResidue(m_id);
    connect(m_molecule->atomById(id), SIGNAL(updated()), this, SLOT(updateAtom()));
  }

  void Residue::removeAtom(unsigned long id)
  {
    if (!m_molecule) {
      qWarning() << Q_FUNC_INFO << "Parent molecule was not set!";
      return;
    }
    if(!m_molecule->atomById(id)){
      qWarning() << Q_FUNC_INFO << QString("Atom with id=%1 was not found in the parent molecule!").arg(id);
      return;
    }
    int index = m_atoms.indexOf(id);
    if (index != -1 ) {
      m_atoms.removeAt(index);
    }
    m_molecule->atomById(id)->setResidue(FALSE_ID);
    disconnect(m_molecule->atomById(id), SIGNAL(updated()), this, SLOT(updateAtom()));
  }

  void Residue::setNumber(const QString& number)
  {
    m_number = number;
  }

  QString Residue::number()
  {
    return m_number;
  }

  void Residue::setChainNumber(unsigned int number)
  {
    m_chainNumber = number;
  }

  unsigned int Residue::chainNumber()
  {
    return m_chainNumber;
  }

  void Residue::setChainID(char id)
  {
    m_chainID = id;
  }

  char Residue::chainID()
  {
    return m_chainID;
  }


  bool Residue::setAtomId(unsigned long id, QString atomId)
  {
    int index = m_atoms.indexOf(id);
    if (index != -1 ) {
      if (m_atomId.size() == index) {
        m_atomId.push_back(atomId.trimmed());
        return true;
      }
      else if (index < m_atomId.size()) {
        m_atomId[index] = atomId.trimmed();
        return true;
      }
      else {
        return false;
      }
    }
    return false;
  }

  bool Residue::setAtomIds(const QList<QString> &atomIds)
  {
    if (atomIds.size() == m_atoms.size()) {
      m_atomId.clear();
      m_atomId = atomIds;
      return true;
    }
    return false;
  }

  QString Residue::atomId(unsigned long id)
  {
    int index = m_atoms.indexOf(id);
    if (index != -1 ) {
      if (m_atomId.size() < index + 1) {
        return "";
      }
      else {
        return m_atomId.at(index);
      }
    }
    return "";
  }

  const QList<QString> & Residue::atomIds() const
  {
    return m_atomId;
  }

  void Residue::updateAtom()
  {
    // We can't trust our atom ids anymore, so we'll let Open Babel guess them.
    m_atomId.clear();
  }

} // End namespace Avogadro
