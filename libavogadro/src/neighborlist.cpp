/*********************************************************************
  NeighborList - NeighborList class

  Copyright (C) 2009 by Tim Vandermeersch

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
***********************************************************************/

#include <avogadro/neighborlist.h>
#include <avogadro/atom.h>

using namespace std;

namespace Avogadro
{

  NeighborList::NeighborList(Molecule* mol, double rcut, int boxSize) : m_mol(mol), m_rcut(rcut)
  {
    m_rcut2 = rcut*rcut;
    m_boxSize = boxSize;
    m_edgeLength = m_rcut / m_boxSize;
    m_updateCounter = 0;

    initOffsetMap();
    initOneTwo();
    initCells();
    initGhostMap();
  }

  QList<Atom*> NeighborList::nbrs(Atom *atom)
  {
    m_r2.clear();
    m_r2.reserve(m_mol->numAtoms());
    QList<Atom*> atoms;
    Eigen::Vector3i index(cellIndexes(atom->pos()));

    std::vector<Eigen::Vector3i>::const_iterator i;
    // Use the offset map to find neighboring cells
    for (i = m_offsetMap.begin(); i != m_offsetMap.end(); ++i) {
      // add the offset to the cell index for atom's cell
      Eigen::Vector3i offset = index + *i;
      // use the ghost map to handle indexes near border:
      // a) periodic boundary conditions --> wrap around
      // b) otherwise --> last empty cell
      unsigned int cell = cellIndex(m_ghostMap.at(ghostIndex(offset)));

      for (atom_iter j = m_cells[cell].begin(); j != m_cells[cell].end(); ++j) {
        // make sure to only return unique pairs
        if (atom->index() >= (*j)->index())
          continue;
        if (IsOneTwo(atom->index(), (*j)->index()))
          continue;
        if (IsOneThree(atom->index(), (*j)->index()))
          continue;

        const double R2 = ( *((*j)->pos()) - *(atom->pos()) ).squaredNorm();
        if (R2 > m_rcut2)
          continue;

        m_r2.push_back(R2);
        atoms.append(*j);
      }
    }

    return atoms;
  }
      
  QList<Atom*> NeighborList::nbrs(const Eigen::Vector3f *pos)
  {
    m_r2.clear();
    m_r2.reserve(m_mol->numAtoms());
    QList<Atom*> atoms;
    Eigen::Vector3d dpos(pos->cast<double>());
    Eigen::Vector3i index(cellIndexes(&dpos));

    std::vector<Eigen::Vector3i>::const_iterator i;
    // Use the offset map to find neighboring cells
    for (i = m_offsetMap.begin(); i != m_offsetMap.end(); ++i) {
      // add the offset to the cell index for atom's cell
      Eigen::Vector3i offset = index + *i;
      // use the ghost map to handle indexes near border:
      // a) periodic boundary conditions --> wrap around
      // b) otherwise --> last empty cell
      unsigned int cell = cellIndex(m_ghostMap.at(ghostIndex(offset)));

      for (atom_iter j = m_cells[cell].begin(); j != m_cells[cell].end(); ++j) {
        
        const double R2 = ( *((*j)->pos()) - dpos).squaredNorm();
        if (R2 > m_rcut2)
          continue;

        m_r2.push_back(R2);
        atoms.append(*j);
      }
    }

    return atoms;
  }

  void NeighborList::update()
  {
    m_updateCounter++;

    if (m_updateCounter > 10) {
      initCells();
      updateCells();
      m_updateCounter = 0;
    }
  }

  void NeighborList::initOneTwo()
  {
    m_oneTwo.resize(m_mol->numAtoms());
    m_oneThree.resize(m_mol->numAtoms());

    foreach (Atom *atom, m_mol->atoms()) {
      foreach (unsigned long id1, atom->neighbors()) {
        Atom *nbr1 = m_mol->atomById(id1);
        m_oneTwo[atom->index()].push_back(nbr1->index());
        m_oneTwo[nbr1->index()].push_back(atom->index());

        foreach (unsigned long id2, nbr1->neighbors()) {
          Atom *nbr2 = m_mol->atomById(id2);
          if (atom->index() == nbr2->index())
            continue;

          m_oneThree[atom->index()].push_back(nbr2->index());
          m_oneThree[nbr2->index()].push_back(atom->index());
        }
      }
    }
  }

  void NeighborList::initCells()
  {
    // find min & max
    foreach (Atom *atom, m_mol->atoms()) {
      Eigen::Vector3d pos = *(atom->pos());

      if (!atom->index()) {
        m_min = m_max = pos;
      } else {
        if (pos.x() > m_max.x())
          m_max.x() = pos.x();
        else if (pos.x() < m_min.x())
          m_min.x() = pos.x();

        if (pos.y() > m_max.y())
          m_max.y() = pos.y();
        else if (pos.y() < m_min.y())
          m_min.y() = pos.y();

        if (pos.z() > m_max.z())
          m_max.z() = pos.z();
        else if (pos.z() < m_min.z())
          m_min.z() = pos.z();
      }
    }

    // set the dimentions
    m_dim.x() = floor( (m_max.x() - m_min.x()) /  m_edgeLength) + 1;
    m_dim.y() = floor( (m_max.y() - m_min.y()) /  m_edgeLength) + 1;
    m_dim.z() = floor( (m_max.z() - m_min.z()) /  m_edgeLength) + 1;
    m_xyDim = m_dim.x() * m_dim.y();

    updateCells();
  }

  void NeighborList::updateCells()
  {
    // add atoms to their cells
    m_cells.clear();
    // the last cell is always empty and can be used for all ghost cells
    // in non-periodic boundary conditions.
    m_cells.resize(m_xyDim * m_dim.z() + 1);
    foreach (Atom *atom, m_mol->atoms()) {
      m_cells[cellIndex(*(atom->pos()))].push_back(&*atom);
    }
  }

  bool NeighborList::insideShpere(const Eigen::Vector3i &index)
  {
    int i = abs(index.x());
    if (i) i--;
    int j = abs(index.y());
    if (j) j--;
    int k = abs(index.z());
    if (k) k--;

    if (Eigen::Vector3i(i, j, k).squaredNorm() < m_rcut2)
      return true;

    return false;
  }

  void NeighborList::initOffsetMap()
  {
    int dim = 2 * m_boxSize + 1;
    m_offsetMap.clear();
    for (int i = 0; i < dim; ++i)
      for (int j = 0; j < dim; ++j)
        for (int k = 0; k < dim; ++k) {
          Eigen::Vector3i index(i - m_boxSize, j - m_boxSize, k - m_boxSize);
          if (insideShpere(index))
            m_offsetMap.push_back( index );
        }

  }

  void NeighborList::initGhostMap(bool periodic)
  {
    int xDim = 2 * m_boxSize + m_dim.x() + 2;
    int yDim = 2 * m_boxSize + m_dim.y() + 2;
    int zDim = 2 * m_boxSize + m_dim.z() + 2;

    m_ghostX = xDim;
    m_ghostXY = xDim * yDim;

    int start = - m_boxSize - 1;
    m_ghostMap.resize(xDim * yDim * zDim);
    for (int i = start; i < m_dim.x() - start; ++i)
      for (int j = start; j < m_dim.y() - start; ++j)
        for (int k = start; k < m_dim.z() - start; ++k) {
          unsigned int ghostCell = ghostIndex(i, j, k);

          int u = i, v = j, w = k;
          if (periodic) {
            // wrap around
            if (i < 0)
              u = m_dim.x() + i + 1;
            else if (i >= m_dim.x())
              u = i - m_dim.x();

            if (j < 0)
              v = m_dim.y() + j + 1;
            else if (j >= m_dim.y())
              v = j - m_dim.y();

            if (k < 0)
              w = m_dim.z() + k + 1;
            else if (k >= m_dim.z())
              w = k - m_dim.z();
          } else {
            if ( (i < 0) || (j < 0) || (k < 0) ||
                  (i >= m_dim.x()) || (j >= m_dim.y()) || (k >= m_dim.z()) )  {
              // point to last cell which is always empty
              u = m_cells.size() - 1;
              v = 0;
              w = 0;
            }
          }

          m_ghostMap[ghostCell] = Eigen::Vector3i(u, v, w);
        }

  }

} // end namespace OpenBabel

//! \file nbrlist.cpp
//! \brief NbrList class
