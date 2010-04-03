/**********************************************************************
  VdWSurface - Class to calculate Van der Waals cubes

  Copyright (C) 2008 Marcus D. Hanwell
  Copyright (C) 2008 Tim Vandermeersch

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.openmolecules.net/>

  This library is free software; you can redistribute it and/or modify
  it under the terms of the GNU Library General Public License as
  published by the Free Software Foundation; either version 2.1 of the
  License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.
 **********************************************************************/

#ifndef VDWSURFACE_H
#define VDWSURFACE_H

#include <QObject>
#include <QFuture>
#include <QFutureWatcher>

#include <Eigen/Core>
#include <vector>

/**
 * @class VdWSurface vdwsurface.h
 * @brief VdWSurface Class
 * @author Marcus D. Hanwell
 *
 * This is a simple class that uses QtConcurrent::map to calculate a cube of the
 * given dimensions. It should use the number of cores available on the system.
 * Ideally some additional binning would be added to further increase speed
 * gains when calculating large cubes with large numbers of atoms.
 */

namespace Avogadro
{

  class Molecule;
  class Cube;
  struct VdWStruct;

  class VdWSurface : public QObject
  {
  Q_OBJECT

  public:
    /**
     * Constructor.
     */
    VdWSurface();

    /**
     * Destructor.
     */
    ~VdWSurface();

    /**
     * Set the atoms in the Molecule to the Van der Waals surface.
     * @param mol Molecule to copy atoms across from.
     */
    void setAtoms(Molecule* mol);

    /**
     * Calculate the VdW cube over the entire range of the supplied Cube.
     * @param cube The cube to write the values VdW surface to.
     * @return True if the calculation was successful.
     */
    void calculateCube(Cube *cube);

    /**
     * When performing a calculation the QFutureWatcher is useful if you want
     * to update a progress bar.
     */
    QFutureWatcher<void> & watcher() { return m_watcher; }

  private Q_SLOTS:
    /**
     * Slot to set the cube data once Qt Concurrent is done
     */
     void calculationComplete();

  Q_SIGNALS:

  private:
    std::vector<Eigen::Vector3d> m_atomPos;
    std::vector<double> m_atomRadius;

    QFuture<void> m_future;
    QFutureWatcher<void> m_watcher;
    Cube *m_cube; // Cube to put the results into
    QVector<VdWStruct> m_VdWvector;

    /// Re-entrant single point forms of the calculations
    static void processPoint(VdWStruct &vdw);
  };

} // End namespace Avogadro

#endif
