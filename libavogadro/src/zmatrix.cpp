/**********************************************************************
  ZMatrix - Class to store a z matrix

  Copyright (C) 2009 Marcus D. Hanwell

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.openmolecules.net/>

  Avogadro is free software; you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as published by
  the Free Software Foundation; either version 2.1 of the License, or
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

#include "zmatrix.h"

namespace Avogadro{

  ZMatrix::ZMatrix(QObject *parent) : QObject(parent)
  {
  }

  ZMatrix::~ZMatrix()
  {
  }

  void ZMatrix::addRow(int row)
  {
    if (row == -1) {
      m_items.push_back(zItem());
      emit rowAdded(m_items.size()-1);
    }
    else {
      m_items.insert(row, zItem());
      emit rowAdded(row);
    }
  }


} // End namespace Avogadro

#include "zmatrix.moc"
