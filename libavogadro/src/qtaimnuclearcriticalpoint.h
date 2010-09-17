/**********************************************************************
  Atom - Atom class derived from the base Primitive class

  Copyright (C) 2007 Donald Ephraim Curtis
  Copyright (c) 2008-2009 Geoff Hutchison
  Copyright (c) 2008-2009 Marcus D. Hanwell
  Copyright (c) 2010 Konstantin Tokarev

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

#ifndef QTAIMNUCLEARCRITICALPOINT_H
#define QTAIMNUCLEARCRITICALPOINT_H

#include <avogadro/primitive.h>
#include <QtCore/QList>

namespace Avogadro {

  class Molecule;
  /**
   * @class QTAIMNuclearCriticalPoint qtaimnuclearcriticalpoint.h <avogadro/qtaimnuclearcriticalpoint.h>
   * @brief Representation of a QTAIM Nuclear Critical Point.
   * @author Eric C. Brown (Based on the Atom Class)
   *
   * The QTAIMNuclearCriticalPoint class is a Primitive subclass that provides a QTAIM Nuclear Critical Point object. All
   * NCPs must be owned by a Molecule. It should also be removed by the
   * Molecule that owns it.
   */
  class QTAIMBondCriticalPoint;
  class Residue;
  class QTAIMNuclearCriticalPointPrivate;
  class A_EXPORT QTAIMNuclearCriticalPoint : public Primitive
  {
  Q_OBJECT

  public:
    /**
     * Constructor.
     *
     * @param parent the parent object, which should be a molecule.
     */
    QTAIMNuclearCriticalPoint(QObject *parent=0);

    /**
     * Destructor.
     */
    ~QTAIMNuclearCriticalPoint();

    /** @name Set atomic information
     * These functions are used to set atomic information.
     * @{
     */

    /**
     * Sets the position of the NCP.
     * @param vec Position of the NCP.
     */
    void setPos(const Eigen::Vector3d &vec);

    /**
     * Sets the position of the NCP.
     * @param vec Position of the NCP.
     */
    void setPos(const Eigen::Vector3d *vec) { setPos(*vec); }

    /**
     * Set the atomic number of the NCP.
     * @note This will emit an updated signal
     */
    void setAtomicNumber(int num);

    /**
     * Set the partial charge of the NCP.
     * @note This is not calculated by the atom, instead call QTAIMIntegrator::electronDensityInAtomicBasin()
     */
    void setPartialCharge(double charge) const
    {
      m_partialCharge = charge;
    }

    /**
     * Set the formal charge of the NCP.
     */
    void setFormalCharge(int charge);

    /**
     * Set the number of the NCP in group of NCPs of the same element.
     */
    void setGroupIndex(unsigned int m_index);

    /**
     * Set the force vector on the NCP
     */
    void setForceVector(const Eigen::Vector3d &force) { m_forceVector = force; }

    /**
     * Set the custom label for the NCP
     */
    void setCustomLabel(const QString &label) {  m_customLabel = label; }

    /**
     * Set the custom color for the NCP using color name
     */
    void setCustomColorName(const QString &name) { m_customColorName = name; }

    /**
     * Set the custom radius for the NCP
     */
    void setCustomRadius(const double radius) { m_customRadius = radius; }
    /** @} */


    /** @name Get atomic information
     * These functions are used to get atomic information.
     * @{
     */

    /**
      * @return The position of the NCP.
      */
    const Eigen::Vector3d * pos() const;

    /**
     * @return Atomic number of the NCP.
     */
    int atomicNumber() const { return m_atomicNumber; }

    /**
     * @return List of QTAIMBondCriticalPoint ids to the NCP.
     */
    QList<unsigned long> bondCriticalPoints() const { return m_bondCriticalPoints; }

    /**
     * @return List of neighbor ids to the NCP (NCPs connected to that NCP).
     */
    QList<unsigned long> neighbors() const;

    /**
     * Use this function to get the bond critical point between this NCP and another.
     * @param other The other NCP to test for bonding.
     * @return Pointer to the bond critical point between the two NCPs, or 0.
     */
    QTAIMBondCriticalPoint * bondCriticalPoint(const QTAIMNuclearCriticalPoint *other) const;

    /**
     * The valence of the NCP.
     */
    double valence() const { return static_cast<double>(m_bondCriticalPoints.size()); }

    /**
     * The index of the NCP in group of NCPs of the same element in Molecule
     */
    unsigned int groupIndex() const { return m_groupIndex; }
    
    /**
     * @return True if the NCP is associated with hydrogen.
     */
    bool isHydrogen() const { return m_atomicNumber == 1; }

    /**
     * @return Partial charge of the NCP.
     */
    double partialCharge() const;

    /**
     * @return Formal charge of the NCP (default = 0)
     */
    int formalCharge() const;

    QString customLabel() const { return m_customLabel; }

    QString customColorName() const {return m_customColorName; }

    double customRadius() const { return m_customRadius; }

    /**
     * @return The force vector on this NCP (if any)
     */
    const Eigen::Vector3d forceVector() const { return m_forceVector; }

    /**
     * @return The Id of the Residue that the NCP is a part of.
     */
    unsigned long residueId() const;

    /**
     * @return A pointer to the Residue that the NCP is a part of.
     */
    Residue * residue() const;
    /** @} */

    /** @name Operators
     * Overloaded operators.
     * @{
     */
    QTAIMNuclearCriticalPoint& operator=(const QTAIMNuclearCriticalPoint& other);
    /** @} */

    friend class Molecule;
    friend class QTAIMBondCriticalPoint;
    friend class Residue;

  protected:
    /**
     * Adds a reference to a BCP to the NCP.
     */
    void addBondCriticalPoint(unsigned long bondCriticalPoint);

    /**
     * Adds a reference to a BCP to the NCP.
     */
    void addBondCriticalPoint(QTAIMBondCriticalPoint* bondCriticalPoint);

    /**
     * Removes the reference of the BCP to the NCP.
     */
    void removeBondCriticalPoint(QTAIMBondCriticalPoint* bondCriticalPoint);

    /**
     * Removes the reference of the BCP to the NCP.
     */
    void removeBondCriticalPoint(unsigned long bondCriticalPoint);

    /**
     * Set the Residue that this NCP is a part of.
     */
    void setResidue(unsigned long id);

    /**
     * Set the Residue that this NCP is a part of.
     */
    void setResidue(const Residue *residue);

    QTAIMNuclearCriticalPointPrivate * const d_ptr;
    Molecule *m_molecule; /** Parent molecule - should always be valid. **/
    int m_atomicNumber;
    unsigned int m_groupIndex;
    unsigned long m_residue;
    QList<unsigned long> m_bondCriticalPoints;
    mutable double m_partialCharge;
    int m_formalCharge;
    Eigen::Vector3d m_forceVector;
    QString m_customLabel;
    QString m_customColorName;
    double m_customRadius;
    Q_DECLARE_PRIVATE(QTAIMNuclearCriticalPoint)
  };

} // End namespace Avogadro

#endif // QTAIMNUCLEARCRITICALPOINT_H
