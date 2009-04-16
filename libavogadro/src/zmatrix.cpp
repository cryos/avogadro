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

#include <avogadro/global.h>

#include <avogadro/molecule.h>
#include <avogadro/atom.h>
#include <avogadro/bond.h>

#include <Eigen/Core>
#include <Eigen/Geometry>

#include <QDebug>

namespace Avogadro{

  using Eigen::Vector3d;

  ZMatrix::ZMatrix(QObject *parent) : QObject(parent)
  {
    m_molecule = qobject_cast<Molecule *>(parent);
  }

  ZMatrix::~ZMatrix()
  {
  }

  void ZMatrix::addRow(int row)
  {
    qDebug() << "Adding new row" << row << m_items.size();
    if (row == -1) {
      m_items.push_back(zItem());
      m_items.last().atomIndex = m_molecule->addAtom()->id();
      emit rowAdded(m_items.size()-1);
    }
    else {
      m_items.insert(row, zItem());
      m_items[row].atomIndex = m_molecule->addAtom()->id();
      emit rowAdded(row);
    }
    // Set some reasonable defaults for new rows
    int size = m_items.size();
    if (size > 1) {
      Bond *b = m_molecule->addBond();
      switch (size) {
        case 2: // Always connected to the first atom - no choice
          m_items[1].indices[0] = 0;
          m_items[1].lengths[0] = 1.0;
          b->setAtoms(m_items[1].atomIndex, m_items[0].atomIndex);
          break;
        case 3:
          m_items[2].indices[0] = 0;
          m_items[2].lengths[0] = 1.0;
          m_items[2].indices[1] = 1;
          m_items[2].lengths[1] = 100.0;
          b->setAtoms(m_items[2].atomIndex, m_items[0].atomIndex);
          break;
        default: // Provide a default - user can change
          m_items[size-1].indices[0] = size - 4;
          m_items[size-1].lengths[0] = 1.0;
          m_items[size-1].indices[1] = size - 3;
          m_items[size-1].lengths[1] = 100;
          m_items[size-1].indices[2] = size - 2;
          m_items[size-1].lengths[2] = 120;
          b->setAtoms(m_items[size-1].atomIndex, m_items[size-4].atomIndex);
      }
    }
  }

  void ZMatrix::setBond(int atom1, int atom2)
  {
    Bond *b = m_molecule->bond(m_items[atom1].atomIndex,
                               m_items[m_items[atom1].indices[0]].atomIndex);
    b->setAtoms(m_items[atom1].atomIndex, m_items[atom2].atomIndex);
    m_items[atom1].indices[0] = atom2;
  }

  void ZMatrix::update()
  {
    // Start at the origin and calculate the coords relative to that
    for(int i = 0; i < m_items.size(); ++i) {
      Atom *a = m_molecule->atomById(m_items[i].atomIndex);
      a->setAtomicNumber(m_items[i].atomicNumber);
      if (i == 0) // First atom - origin
        a->setPos(Vector3d::Zero());
      else if (i == 1) // Second atom - just length
        a->setPos(Vector3d(m_items[i].lengths[0], 0.0, 0.0));
      else if (i == 2) { // Third atom - length and angle
        double length = m_items[i].lengths[0];
        double angle = m_items[i].lengths[1] * cDegToRad;
        double x = length * cos(angle);
        double y = length * sin(angle);
        a->setPos(Vector3d(x, y, 0.0));
      }
      else { // The general case where all three values are set
        double length = m_items[i].lengths[0];
        double angle = m_items[i].lengths[1] * cDegToRad;
        double dihedral = m_items[i].lengths[2] * cDegToRad;
        Atom *a1 = m_molecule->atomById(m_items[m_items[i].indices[0]].atomIndex);
        Atom *a2 = m_molecule->atomById(m_items[m_items[i].indices[1]].atomIndex);
        Atom *a3 = m_molecule->atomById(m_items[m_items[i].indices[2]].atomIndex);

        // Dihedral angle - perform rotation
        Vector3d v1(*a1->pos() - *a2->pos());
        Vector3d v2(*a1->pos() - *a3->pos());
        if (v1.norm() < 0.01 || v2.norm() < 0.01) {// Undefined
          a->setPos(a1->pos());
          continue;
        }
        // Find the two orthogonal axes for the dihedral angle rotation
        Vector3d axis1 = v1.cross(v2).normalized();
        Vector3d axis2 = v1.cross(axis1).normalized();
        axis1 *= -sin(dihedral);
        axis2 *= cos(dihedral);
        // Now rotate about the bond angle
        Vector3d v3 = (axis1 + axis2).normalized();
        v3 *= length * sin(angle);
        v1.normalize();
        v1 *= length * cos(angle);
        v2 = *a1->pos() + v3 - v1;
        // Now we have the position of the new atom
        a->setPos(v2);
      }
    }
  }


} // End namespace Avogadro

#include "zmatrix.moc"
