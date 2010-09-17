/**********************************************************************
  Bond - Bond class derived from the base Primitive class

  Copyright (C) 2007 Donald Ephraim Curtis
	Copyright (c) 2008-2009 Marcus D. Hanwell
	Copyright (c) 2009 Tim Vandermeersch
	Copyright (c) 2009 Geoff Hutchison

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

#ifndef QTAIMBONDCRITICALPOINT_H
#define QTAIMBONDCRITICALPOINT_H

#include <avogadro/primitive.h>

namespace Avogadro {

  /**
   * @class QTAIMBondCriticalPoint qtaimbondcriticalpoint.h <avogadro/qtaimbondcriticalpoint.h>
   * @brief Representation of a QTAIM Bond Critical Point (BCP). Based on the Bond Class.
   * @author Eric C. Brown
   *
   * The QTAIMBondCriticalPoint class is a Primitive subclass that provides a QTAIMBondCriticalPoint object.
   */
  class QTAIMNuclearCriticalPoint;
  class Molecule;
  class QTAIMBondCriticalPointPrivate;
  class A_EXPORT QTAIMBondCriticalPoint : public Primitive
  {
  Q_OBJECT

  public:
    /**
     * Constructor
     *
     * @param parent the object parent.
     */
    QTAIMBondCriticalPoint(QObject *parent=0);

    ~QTAIMBondCriticalPoint();

    /** @name Set bonding information
     * These functions are used to set bonding information.
     * @{
     */

    /**
     * Sets the position of the BCP.
     * @param vec Position of the BCP.
     */
    void setPos(const Eigen::Vector3d &vec);

    /**
     * Sets the position of the BCP.
     * @param vec Position of the BCP.
     */
    void setPos(const Eigen::Vector3d *vec) { setPos(*vec); }

    /**
     * Set the unique ID of the first NCP associated with the BCP.
     */
    void setBegin(QTAIMNuclearCriticalPoint* nuclearCriticalPoint);

    /**
     * Set the unique ID of the second NCP associated with the BCP.
     */
    void setEnd(QTAIMNuclearCriticalPoint* nuclearCriticalPoint);

    /**
     * Set the unique ID of both NCPs in the BCP.
     * @param ncp1 First NCP associated with the bond.
     * @param ncp2 Second NCP associated with the bond.
     * @param order Bond order (defaults to 1).
     */
    void setNuclearCriticalPoints(unsigned long nuclearCriticalPoint1, unsigned long nuclearCriticalPoint2,
                  short order = 1);

    /**
     * Set the order of the BCP.
     */
    void setOrder(short order) { m_order = order; }

    /**
     * Set the aromaticity of the BCP.
     */
    void setAromaticity(bool isAromatic) const;

    /**
     * Set the custom label for the BCP
     */
    void setCustomLabel(const QString &label) {  m_customLabel = label; }

    /**
     * Set the custom color for the BCP using color name
     */
    void setCustomColorName(const QString &name) { m_customColorName = name; }

    /**
     * Set the custom radius for the BCP
     */
    void setCustomRadius(const double radius) { m_customRadius = radius; }

    /**
     * Set the bond path
     */
    void setBondPath(const QList<Eigen::Vector3d> bondPath) { m_bondPath = bondPath ; }

    /**
     * Set the laplacian at BCP
     */
    void setLaplacian(const qreal laplacian) { m_laplacian = laplacian; }
    void setEllipticity(const qreal ellipticity) { m_ellipticity = ellipticity; }


    /** @} */

    /** @} */

    /** @name Get bonding information
     * These functions are used to get bonding information.
     * @{
     */

    /**
      * @return The position of the BCP.
      */
    const Eigen::Vector3d * pos() const;

    /**
     * @return the unique ID of the first NCP associated with the BCP.
     */
    unsigned long beginNuclearCriticalPointId() const { return m_beginNuclearCriticalPointId; }

    /**
     * @return Pointer to the first NCP associated with the bond.
     */
    QTAIMNuclearCriticalPoint * beginNuclearCriticalPoint() const;

    /**
     * @return the unique ID of the second NCP associated with the BCP.
     */
    unsigned long endNuclearCriticalPointId() const { return m_endNuclearCriticalPointId; }

    /**
     * @return Pointer to the second NCP associated with the BCP.
     */
    QTAIMNuclearCriticalPoint * endNuclearCriticalPoint() const;

    /**
     * @return The position of the start of the Bond Path.
     */
    const Eigen::Vector3d * beginPos() const;

    /**
     * @return The position of the mid-point of the BCP (actual position of BCP).
     */
    const Eigen::Vector3d * midPos() const;

    /**
     * @return The position of the end of the Bond Path.
     */
    const Eigen::Vector3d * endPos() const;

    /**
     * Get the unique id of the other NCP associated with the BCP.
     * @param ncpId The unique id of the NCP.
     * @return The unique if of the other NCP associated with the BCP.
     * @note This function does not perform checks to ensure the supplied
     * NCP is actually associated with the BCP.
     */
    unsigned long otherNuclearCriticalPoint(unsigned long nuclearCriticalPointId) const;

    /**
     * @return the order of the BCP - 1 = single, 2 = double etc.
     */
    short order() const { return m_order; }

    /**
     * @return True if the BCP is aromatic.
     */
    bool isAromatic() const;

    /**
     * @return the length of the Bond Path.
     */
    double length() const;

    /**
     * @return the custom label of the BCP
     */
    QString customLabel() const { return m_customLabel; }

    /**
     * @return the custom color of the BCP
     */
    QString customColorName() const {return m_customColorName; }

    /**
     * @return the custom radius of the Bond Path
     */
    double customRadius() const { return m_customRadius; }

    QList<Eigen::Vector3d> bondPath() const { return m_bondPath; }
    qreal laplacian() const { return m_laplacian; }
    qreal ellipticity() const { return m_ellipticity; }

    /** @} */

    /** @name OpenBabel conversion functions
     * These functions are used convert between Avogadro and OpenBabel bonds.
     * @{
     */

    /** @name Operators
     * Overloaded operators.
     * @{
     */
    QTAIMBondCriticalPoint& operator=(const QTAIMBondCriticalPoint& other);
    /** @} */

    friend class Molecule;

  private:
    unsigned long m_beginNuclearCriticalPointId, m_endNuclearCriticalPointId;
    short m_order;
    mutable bool m_isAromatic;
    mutable Eigen::Vector3d m_midPos;
    Molecule *m_molecule;
    QString m_customLabel;
    QString m_customColorName;
    double m_customRadius;
    QList<Eigen::Vector3d> m_bondPath;
    qreal m_laplacian;
    qreal m_ellipticity;

    /* shared d_ptr with Primitive */
    Q_DECLARE_PRIVATE(QTAIMBondCriticalPoint)
  };

} // End namespace Avogadro

#endif
