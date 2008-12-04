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

#ifndef BOND_H
#define BOND_H

#include <avogadro/primitive.h>
#include <avogadro/atom.h>

namespace OpenBabel {
  class OBBond;
}

namespace Avogadro {

  /**
   * @class Bond bond.h <avogadro/bond.h>
   * @brief Bond Class
   * @author Marcus D. Hanwell
   *
   * The Bond class is a Primitive subclass that provides a bond object.
   */
  class Atom;
  class Molecule;
  class BondPrivate;
  class A_EXPORT Bond : public Primitive
  {
    Q_OBJECT

    public:
      /**
       * Constructor
       *
       * @param parent the object parent.
       */
      Bond(QObject *parent=0);

      ~Bond();

      /**
       * Set the unique ID of the first atom in the bond.
       */
      void setBegin(Atom* atom);

      /**
       * @return the unique ID of the first atom in the bond.
       */
      inline unsigned long int beginAtomId() const { return m_beginAtomId; }

      /**
       * Set the unique ID of the second atom in the bond.
       */
      void setEnd(Atom* atom);

      /**
       * @return the unique ID of the second atom in the bond.
       */
      inline unsigned long int endAtomId() const { return m_endAtomId; }

      /**
       * Set the unique ID of both atoms in the bond
       * @param atom1 First atom in the bond.
       * @param atom2 Second atom in the bond.
       * @param order Bond order (defaults to 1).
       */
      void setAtoms(unsigned long int atom1, unsigned long int atom2,
                    short order = 1);

      /**
       * Get the unique id of the other atom in the bond.
       * @param atomId The unique id of the Atom.
       * @return The unique if of the other Atom in the bond.
       * @note This function does not perform checks to ensure the supplied
       * Atom is actually in the bond.
       */
      unsigned long int otherAtom(unsigned long int atomId) const;

      /**
       * @return the order of the bond - 1 = single, 2 = double etc.
       */
      inline short order() const { return m_order; }

      /**
       * Set the order of the bond.
       */
      inline void setOrder(short order) { m_order = order; }

      /**
       * @return the length of the bond.
       */
      double length() const;

      /**
       * Copy an OpenBabel::OBBond's data to create a similar bond.
       */
      bool setOBBond(OpenBabel::OBBond *obbond);

      Bond& operator=(const Bond& other);

    private:
      unsigned long int m_beginAtomId, m_endAtomId;
      short m_order;
      Molecule *m_molecule;
      /* shared d_ptr with Primitive */
      Q_DECLARE_PRIVATE(Bond)
  };

} // End namespace Avogadro

#endif
