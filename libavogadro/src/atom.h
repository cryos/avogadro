/**********************************************************************
  Atom - Atom class derived from the base Primitive class

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

#ifndef ATOM_H
#define ATOM_H

#include <avogadro/primitive.h>

namespace OpenBabel {
  class OBAtom;
}

namespace Avogadro {

  /**
   * @class Atom primitive.h <avogadro/primitive.h>
   * @brief Atom Class
   * @author Donald Ephraim Curtis
   *
   * The Atom class is a Primitive subclass that provides a wrapper around
   * OpenBabel::OBAtom.  This class is provided to give more control of
   * the OpenBabel::OBAtom class through slots/signals provided by the
   * Primitive superclass.
   */
  class Bond;
  class AtomPrivate;
  class A_EXPORT Atom : public Primitive
  {
    Q_OBJECT

    public:
      /**
       * Constructor
       *
       * @param parent the object parent.
       */
      Atom(QObject *parent=0);

      /**
        * Returns the position of the atom.
        * @return The position of the atom.
        */
      inline const Eigen::Vector3d &pos () const
      {
        return m_pos;
      }

      /**
       * Sets the position of the atom.
       * @param vec Position of the atom.
       */
      inline void setPos(const Eigen::Vector3d &vec)
      {
        m_pos = vec;
      }

      /**
       * @return Atomic number of the atom.
       * @note Replaces GetAtomicNum()
       */
      inline int atomicNumber() const { return m_atomicNum; }

      /**
       * Set the atomic number of the atom.
       */
      inline void setAtomicNumber(int num) { m_atomicNum = num; }

      /**
       * Adds a reference to a bond to the atom.
       */
      void addBond(Bond* bond);

      /**
       * Deletes the reference of the bond to the atom.
       */
      void deleteBond(Bond* bond);

      /**
       * @return List of bond ids to the atom.
       */
      QList<unsigned long int> bonds() { return m_bonds; }

      /**
       * @return List of neighbor ids to the atom (atoms bonded to that atom).
       */
      QList<unsigned long int> neighbors() { return m_neighbors; }

      /**
       * The valence of the atom. FIXME - don't think this will cut it...
       */
      double valence() { return static_cast<double>(m_neighbors.size()); }

      /**
       * @return True if the atom is a hydrogen.
       */
      bool isHydrogen() const { return m_atomicNum == 1; }

      /**
       * Set the partial charge of the atom.
       * @note This is not calculated at the moment!
       */
      inline void setPartialCharge(double charge)
      {
        m_partialCharge = charge;
      }

      /**
       * @return Partial charge of the atom.
       */
      inline double partialCharge() const
      {
        return m_partialCharge;
      }

      /// Our OpenBabel conversion functions
      /**
       * @return An OpenBabel::OBAtom copy of the atom.
       */
      OpenBabel::OBAtom OBAtom();

      /**
       * Copies an OpenBabel::OBAtom to the atom.
       */
      bool setOBAtom(OpenBabel::OBAtom *obatom);

      Atom& operator=(const Atom& other);

    private:
      /* shared d_ptr with Primitive */
      Eigen::Vector3d m_pos;
      int m_atomicNum;
      QList<unsigned long int> m_bonds, m_neighbors;
      double m_partialCharge;
      Q_DECLARE_PRIVATE(Atom)
  };

} // End namespace Avoagdro

#endif
