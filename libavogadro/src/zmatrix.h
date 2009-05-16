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

#ifndef ZMATRIX_H
#define ZMATRIX_H

#include "global.h"

#include <QObject>
#include <QList>

namespace Avogadro {

  class Molecule;

  /**
   * @class ZMatrix zmatrix.h <avogadro/zmatrix.h>
   * @brief Z-matrix internal coordinates for a molecule.
   * @author Marcus D. Hanwell
   *
   * The ZMatrix class is a Primitive subclass that provides a ZMatrix object.
   * All z matrices must be owned by a Molecule. It should also be removed by
   * the Molecule that owns it.
   */

  class A_EXPORT ZMatrix : public QObject
  {
    Q_OBJECT

  public:
    /**
     * Constructor. Parent should always be a Molecule.
     */
    explicit ZMatrix(QObject *parent = 0);

    /**
     * Destructor.
     */
    ~ZMatrix();

    /**
     * Add a new line to the z-matrix.
     * @param row Row to add, default to adding a new row at the end.
     */
    void addRow(int row = -1);

    /**
     * @return The number of rows in the z-matrix.
     */
    int rows();

    /**
     * Set the atom that the z matrix item is bonded to.
     * @param atom1 The first atom in the bond
     * @param atom2 The second atom in the bond
     */
    void setBond(int atom1, int atom2);

    /**
     * Update the atoms of the z matrix according to the z matrix
     */
    void update();

//  private:
    /**
     * Struct storing a z-matrix
     */
    struct zItem
    {
      unsigned int atomIndex; // Unique ID of the atom
      short atomicNumber;     // The atomic number of the entry
      double lengths[3];      // Length, angle, dihedral
      short indices[3];       // z-matrix reference to other atoms in the matrix
    };

    QList<zItem> m_items;
    Molecule *m_molecule;

  Q_SIGNALS:
    void rowAdded(int row);

  };

  inline int ZMatrix::rows()
  {
    return m_items.size();
  }

} // End namespace Avogadro

#endif // ZMATRIX_H
