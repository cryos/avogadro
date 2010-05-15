/**********************************************************************
  Atom - Atom class derived from the base Primitive class

  Copyright (C) 2007 Donald Ephraim Curtis
	Copyright (c) 2008-2009 Geoff Hutchison
	Copyright (c) 2008-2009 Marcus D. Hanwell

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

#ifndef ATOM_H
#define ATOM_H

#include <avogadro/primitive.h>
#include <QColor>
#include <QList>

namespace OpenBabel {
  class OBAtom;
}

namespace Avogadro {

  class Molecule;
  /**
   * @class Atom atom.h <avogadro/atom.h>
   * @brief Representation of an atom.
   * @author Marcus D. Hanwell
   *
   * The Atom class is a Primitive subclass that provides an Atom object. All
   * atoms must be owned by a Molecule. It should also be removed by the
   * Molecule that owns it.
   */
  class Bond;
  class Residue;
  class AtomPrivate;
  class A_EXPORT Atom : public Primitive
  {
  Q_OBJECT

  public:
    /**
     * Constructor.
     *
     * @param parent the parent object, which should be a molecule.
     */
    Atom(QObject *parent=0);

    /**
     * Destructor.
     */
    ~Atom();

    /** @name Set atomic information
     * These functions are used to set atomic information.
     * @{
     */

    /**
     * Sets the position of the atom.
     * @param vec Position of the atom.
     */
    void setPos(const Eigen::Vector3d &vec);

    /**
     * Sets the position of the atom.
     * @param vec Position of the atom.
     */
    void setPos(const Eigen::Vector3d *vec) { setPos(*vec); }

    /**
     * Set the atomic number of the atom.
     * @note This will emit an updated signal
     */
    void setAtomicNumber(int num);

    /**
     * Set the partial charge of the atom.
     * @note This is not calculated by the atom, instead call Molecule::calculatePartialCharges()
     */
    void setPartialCharge(double charge) const
    {
      m_partialCharge = charge;
    }

    /**
     * Set the formal charge of the atom.
     */
    void setFormalCharge(int charge);

    /**
     * Set the number of the atom in group of atoms of the same element.
     */
    void setGroupIndex(unsigned int m_index);

    /**
     * Set the force vector on the atom (e.g., used to display vibrations)
     */
    void setForceVector(const Eigen::Vector3d &force) { m_forceVector = force; }

    void setCustomLabel(const QString &label) {  m_customLabel = label; }

    void setCustomColor(const QColor &color) { m_customColor->setRgb(color.rgb()); }
    /** @} */


    /** @name Get atomic information
     * These functions are used to get atomic information.
     * @{
     */

    /**
      * @return The position of the atom.
      */
    const Eigen::Vector3d * pos() const;

    /**
     * @return Atomic number of the atom.
     * @note Replaces GetAtomicNum()
     */
    int atomicNumber() const { return m_atomicNumber; }

    /**
     * @return List of bond ids to the atom.
     */
    QList<unsigned long> bonds() const { return m_bonds; }

    /**
     * @return List of neighbor ids to the atom (atoms bonded to that atom).
     */
    QList<unsigned long> neighbors() const;

    /**
     * Use this function to get the bond between this Atom and another.
     * @param other The other Atom to test for bonding.
     * @return Pointer to the bond between the two atoms, or 0.
     */
    Bond * bond(const Atom *other) const;

    /**
     * The valence of the atom. FIXME - don't think this will cut it...
     */
    double valence() const { return static_cast<double>(m_bonds.size()); }

    /**
     * The index of the atom in group of atoms of the same element in Molecule
     */
    unsigned int groupIndex() const { return m_groupIndex; }
    
    /**
     * @return True if the atom is a hydrogen.
     */
    bool isHydrogen() const { return m_atomicNumber == 1; }

    /**
     * @return Partial charge of the atom.
     */
    double partialCharge() const;

    /**
     * @return Formal charge of the atom (default = 0)
     */
    int formalCharge() const;

    QString customLabel() const { return m_customLabel; }

    QColor* customColor() const {return m_customColor; }

    /**
     * @return The force vector on this atom (if any)
     */
    const Eigen::Vector3d forceVector() const { return m_forceVector; }

    /**
     * @return The Id of the Residue that the Atom is a part of.
     */
    unsigned long residueId() const;

    /**
     * @return A pointer to the Residue that the Atom is a part of.
     */
    Residue * residue() const;
    /** @} */


    /** @name OpenBabel conversion functions
     * These functions are used convert between Avogadro and OpenBabel atoms.
     * @{
     */

    /**
     * @return An OpenBabel::OBAtom copy of the atom.
     */
    OpenBabel::OBAtom OBAtom();
    const OpenBabel::OBAtom OBAtom() const;

    /**
     * Copies the data from an OpenBabel::OBAtom to the atom.
     */
    bool setOBAtom(OpenBabel::OBAtom *obatom);
    /** @} */

    /** @name Operators
     * Overloaded operators.
     * @{
     */
    Atom& operator=(const Atom& other);
    /** @} */

    friend class Molecule;
    friend class Bond;
    friend class Residue;

  protected:
    /**
     * Adds a reference to a bond to the atom.
     */
    void addBond(unsigned long bond);

    /**
     * Adds a reference to a bond to the atom.
     */
    void addBond(Bond* bond);

    /**
     * Removes the reference of the bond to the atom.
     */
    void removeBond(Bond* bond);

    /**
     * Removes the reference of the bond to the atom.
     */
    void removeBond(unsigned long bond);

    /**
     * Set the Residue that this Atom is a part of.
     */
    void setResidue(unsigned long id);

    /**
     * Set the Residue that this Atom is a part of.
     */
    void setResidue(const Residue *residue);

    AtomPrivate * const d_ptr;
    Molecule *m_molecule; /** Parent molecule - should always be valid. **/
    int m_atomicNumber;
    unsigned int m_groupIndex;
    unsigned long m_residue;
    QList<unsigned long> m_bonds;
    mutable double m_partialCharge;
    int m_formalCharge;
    Eigen::Vector3d m_forceVector;
    QString m_customLabel;
    QColor *m_customColor;
    Q_DECLARE_PRIVATE(Atom)
  };

} // End namespace Avoagdro

#endif
