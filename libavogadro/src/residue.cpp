/**********************************************************************
  Residue - Residue class derived from the base Primitive class

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

#include "residue.h"

#include <QDebug>

namespace Avogadro {

  Residue::Residue(QObject *parent): Fragment(ResidueType, parent),
    m_chainNumber(0)
  {
  }

  Residue::~Residue()
  {
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

  bool Residue::setAtomId(unsigned long int id, QString atomId)
  {
    int index = m_atoms.indexOf(id);
    if (index != -1) {
      if (m_atomId.size() == index) {
        m_atomId.push_back(atomId);
        return true;
      }
      else if (m_atomId.size() < index) {
        m_atomId[index] = atomId;
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

  QString Residue::atomId(unsigned long int id)
  {
    int index = m_atoms.indexOf(id);
    if (index != -1) {
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

} // End namespace Avogadro

 #include "residue.moc"

