/**********************************************************************
  Bond - Bond class derived from the base Primitive class

  Copyright (C) 2007 Donald Ephraim Curtis
	Copyright (c) 2008-2009 Marcus D. Hanwell
	Copyright (c) 2009 Tim Vandermeersch
	Copyright (c) 2009 Geoff Hutchison

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.cc/>

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

namespace OpenBabel {
  class OBBond;
}

namespace Avogadro {

  /**
   * @class Bond bond.h <avogadro/bond.h>
   * @brief Representation of a chemical bond.
   * @author Marcus D. Hanwell
   *
   * The Bond class is a Primitive subclass that provides a Bond object.
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

    /** @name Set bonding information
     * These functions are used to set bonding information.
     * @{
     */

    /**
     * Set the unique ID of the first atom in the bond.
     */
    void setBegin(Atom* atom);

    /**
     * Set the unique ID of the second atom in the bond.
     */
    void setEnd(Atom* atom);

    /**
     * Set the unique ID of both atoms in the bond.
     * @param atom1 First atom in the bond.
     * @param atom2 Second atom in the bond.
     * @param order Bond order (defaults to 1).
     */
    void setAtoms(unsigned long atom1, unsigned long atom2,
                  short order = 1);

    /**
     * Set the order of the bond.
     */
    void setOrder(short order) { m_order = order; }

    /**
     * Set the aromaticity of the bond.
     */
    void setAromaticity(bool isAromatic) const;

    /**
     * Set the custom label for the bond
     */
    void setCustomLabel(const QString &label) {  m_customLabel = label; }
    /** @} */

    /** @name Get bonding information
     * These functions are used to get bonding information.
     * @{
     */

    /**
     * @return the unique ID of the first atom in the bond.
     */
    unsigned long beginAtomId() const { return m_beginAtomId; }

    /**
     * @return Pointer to the first atom in the bond.
     */
    Atom * beginAtom() const;

    /**
     * @return the unique ID of the second atom in the bond.
     */
    unsigned long endAtomId() const { return m_endAtomId; }

    /**
     * @return Pointer to the second atom in the bond.
     */
    Atom * endAtom() const;

    /**
     * @return The position of the start of the Bond.
     */
    const Eigen::Vector3d * beginPos() const;

    /**
     * @return The position of the mid-point of the Bond.
     */
    const Eigen::Vector3d * midPos() const;

    /**
     * @return The position of the end of the Bond.
     */
    const Eigen::Vector3d * endPos() const;

    /**
     * Get the unique id of the other atom in the bond.
     * @param atomId The unique id of the Atom.
     * @return The unique if of the other Atom in the bond.
     * @note This function does not perform checks to ensure the supplied
     * Atom is actually in the bond.
     */
    unsigned long otherAtom(unsigned long atomId) const;

    /**
     * @return the order of the bond - 1 = single, 2 = double etc.
     */
    short order() const { return m_order; }

    /**
     * @return True if the bond is aromatic.
     */
    bool isAromatic() const;

    /**
     * @return the length of the bond.
     */
    double length() const;

    /**
     * @return the custom label of the bond
     */
    QString customLabel() const { return m_customLabel; }
    /** @} */

    /** @name OpenBabel conversion functions
     * These functions are used convert between Avogadro and OpenBabel bonds.
     * @{
     */

    /**
     * Copy the data from an OpenBabel::OBBond to create a similar bond.
     */
    bool setOBBond(OpenBabel::OBBond *obbond);
    /** @} */

    /** @name Operators
     * Overloaded operators.
     * @{
     */
    Bond& operator=(const Bond& other);
    /** @} */

    friend class Molecule;

  private:
    unsigned long m_beginAtomId, m_endAtomId;
    short m_order;
    mutable bool m_isAromatic;
    mutable Eigen::Vector3d m_midPos;
    Molecule *m_molecule;
    QString m_customLabel;
    /* shared d_ptr with Primitive */
    Q_DECLARE_PRIVATE(Bond)
  };

} // End namespace Avogadro

#endif
