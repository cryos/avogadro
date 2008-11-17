/**********************************************************************
  Bond - Bond class derived from the base Primitive class

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

 #include "bond.h"

 #include <openbabel/mol.h>

 namespace Avogadro{

  class BondPrivate {
    public:
      BondPrivate() {}
  };

  Bond::Bond(QObject *parent) : Primitive(BondType, parent), m_beginAtomId(0),
    m_endAtomId(0), m_order(1)
  {
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
