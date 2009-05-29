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

#ifndef NEIGHBORLIST_H
#define NEIGHBORLIST_H

#include <avogadro/molecule.h>

#include <Eigen/Core>

namespace Avogadro
{
  /**
   * @class NeighborList neighborlist.h <avogadro/neighborlist.h>
   * @brief A list container for finding spatial neighbors of atoms
   * @author Tim Vandermeersch
   *
   * NeighborList can be used for finding non-bonded interactions between
   * pairs of atoms.
   * Based on:
   * Near-neighbor calculations using a modified cell-linked list method
   * Mattson, W.; B. M. Rice (1999). "Near-neighbor calculations using a
   * modified cell-linked list method". Computer Physics Communications
   * 119: 135.
   *
   * http://dx.doi.org/10.1016/S0010-4655%2898%2900203-3
   *
   */
  class Atom;

  class A_EXPORT NeighborList
  {
    private:
      typedef std::vector<Atom*>::iterator atom_iter;

    public:
      /**
       * Constructor to include all atoms.
       * @param mol The molecule containing the atoms
       * @param rcut The cut-off distance.
       * @param boxSize The number of cells per rcut distance.
       */
      NeighborList(Molecule *mol, double rcut, bool periodic = false, int boxSize = 1);
      /**
       * Construcor.
       * @param atoms The atoms
       * @param rcut The cut-off distance.
       * @param periodic Use periodic boundary conditions.
       * @param The number of cells per rcut distance. 
       */
      NeighborList(const QList<Atom*> &atoms, double rcut, bool periodic = false, int boxSize = 1);

      /**
       * Update the cells. While minimizing or running MD simulations,
       * atoms move and can go from on cell into the next. This function
       * should be called every 10-20 iterations to make sure the cells
       * stay accurate.
       */
      void update();
      /**
       * Get the near-neighbor atoms for @p atom. The squared distance is
       * checked and is cached for later use (see r2() function).
       *
       * Note: Atoms in relative 1-2 and 1-3 positions are not returned.
       * The @p atom itself isn't added to the list.
       *
       * @param atom The atom for which to return the near-neighbors
       * @return The near-neighbors for @p atom
       */
      QList<Atom*> nbrs(Atom *atom, bool uniqueOnly = true);
      /**
       * Get the near-neighbor atoms around @p pos. The squared distance is
       * checked and is cached for later use (see r2() function).
       *
       * @param pos The position for which to return the near-neighbors
       * @return The near-neighbors for @p atom
       */
      QList<Atom*> nbrs(const Eigen::Vector3f *pos);
      /**
       * Get the cached squared distance from the atom last used to call
       * nbrs to the atom with @p index in the returned vector.
       * @param index The index for the atom in the vector of atoms returned by nbrs().
       * @return The cached squared distance.
       */
      inline double r2(unsigned int index) const
      {
        return m_r2.at(index);
      }

    private:
      inline unsigned int ghostIndex(int i, int j, int k) const
      {
        i += m_boxSize + 1;
        j += m_boxSize + 1;
        k += m_boxSize + 1;
        return i + j * m_ghostX + k * m_ghostXY;
      }

      inline unsigned int ghostIndex(const Eigen::Vector3i &index) const
      {
        return ghostIndex(index.x(), index.y(), index.z());
      }

      inline unsigned int cellIndex(int i, int j, int k) const
      {
        return i + j * m_dim.x() + k * m_xyDim;
      }

      inline unsigned int cellIndex(const Eigen::Vector3i &index) const
      {
        return index.x() + index.y() * m_dim.x() + index.z() * m_xyDim;
      }


      inline unsigned int cellIndex(const Eigen::Vector3d &pos) const
      {
        return cellIndex( int(floor( (pos.x() - m_min.x()) / m_edgeLength )),
                          int(floor( (pos.y() - m_min.y()) / m_edgeLength )),
                          int(floor( (pos.z() - m_min.z()) / m_edgeLength )) );
      }

      inline Eigen::Vector3i cellIndexes(const Eigen::Vector3d *pos) const
      {
        Eigen::Vector3i index;
        index.x() = int(floor( (pos->x() - m_min.x()) / m_edgeLength ));
        index.y() = int(floor( (pos->y() - m_min.y()) / m_edgeLength ));
        index.z() = int(floor( (pos->z() - m_min.z()) / m_edgeLength ));
        return index;
      }

      /**
       * @param i Index for the first atom (indexed from 1)
       * @param j Index for the first atom (indexed from 1)
       * @return True if atoms with index @p i and @p j are bonded (1-2)
       */
      inline bool IsOneTwo(unsigned int i, unsigned int j) const
      {
        std::vector<unsigned int>::const_iterator iter;
        for (iter = m_oneTwo.at(i).begin(); iter != m_oneTwo.at(i).end(); ++iter)
          if (*iter == j)
            return true;

        return false;
      }

      /**
       * @param i Index for the first atom (indexed from 1)
       * @param j Index for the first atom (indexed from 1)
       * @return True if atoms with index @p i and @p j are in a 1-3 position
       */
      inline bool IsOneThree(unsigned int i, unsigned int j) const
      {
        std::vector<unsigned int>::const_iterator iter;
        for (iter = m_oneThree.at(i).begin(); iter != m_oneThree.at(i).end(); ++iter)
          if (*iter == j)
            return true;

        return false;
      }

      /**
       * Initialize the 1-2 and 1-3 cache.
       */
      void initOneTwo();
      void initCells();
      void updateCells();
      void initOffsetMap();
      void initGhostMap(bool periodic = false);
      bool insideShpere(const Eigen::Vector3i &index);

      QList<Atom*>                        m_atoms;
      double                              m_rcut, m_rcut2;
      double                              m_edgeLength;
      int                                 m_boxSize;
      int                                 m_updateCounter;


      Eigen::Vector3d                     m_min, m_max;
      Eigen::Vector3i                     m_dim;
      int                                 m_xyDim;
      std::vector<std::vector<Atom*> >    m_cells;

      std::vector<Eigen::Vector3i>        m_offsetMap;
      std::vector<Eigen::Vector3i>        m_ghostMap;
      int                                 m_ghostX;
      int                                 m_ghostXY;

      std::vector<double>                 m_r2;

      std::vector<std::vector<unsigned int> > m_oneTwo;
      std::vector<std::vector<unsigned int> > m_oneThree;
  };

} // end namespace OpenBabel

//! \brief NeighborList class

#endif
