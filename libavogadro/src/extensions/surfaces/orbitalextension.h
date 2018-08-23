/**********************************************************************
  OrbitalExtension - Molecular orbital explorer

  Copyright (C) 2010 David C. Lonie

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

#ifndef ORBITALEXTENSION_H
#define ORBITALEXTENSION_H

#include <avogadro/dockextension.h>

#include <QDockWidget>
#include <QCloseEvent>
#include <QVector>
#include <QMutex>
#include <QList>
#include <QTime>

class QProgressDialog;

namespace OpenQube
{
  class BasisSet;
  class Cube;
}

namespace Avogadro
{
  class Cube;
  class Mesh;
  class MeshGenerator;
  class VdWSurface;
  class SurfaceDialog;
  class OrbitalWidget;

  enum CalcState {
    NotStarted = 0,
    Running,
    Completed,
    Canceled
  };

  struct calcInfo {
    Mesh *posMesh;
    Mesh *negMesh;
    Cube *cube;
    unsigned int orbital;
    double resolution;
    double isovalue;
    double boxPadding;
    unsigned int priority;
    CalcState state;
  };

  class OrbitalDock : public QDockWidget
  {
  public:
    OrbitalDock( const QString & title, QWidget * parent = 0,
      Qt::WindowFlags flags = 0 ) : QDockWidget(title, parent, flags) {}

  protected:
    void closeEvent ( QCloseEvent * event )
    {
      if (widget())
        widget()->hide();
      event->accept();
    }
  };

  class OrbitalExtension : public DockExtension
  {
    Q_OBJECT
    AVOGADRO_EXTENSION("Orbitals", tr("Orbitals"),
                       tr("Calculate and visualize molecular orbitals"))

  public:
    OrbitalExtension(QObject* parent = 0);
    virtual ~OrbitalExtension();

    virtual QList<QAction *> actions() const;
    virtual QString menuPath(QAction* action) const;
    virtual QDockWidget * dockWidget();
    virtual void setMolecule(Molecule *molecule);

  private slots:
    /**
     * Load the appropriate basis set (if possible)
     */
    bool loadBasis();

    /**
     * Re-render an orbital at a higher resolution
     *
     * @param orbital The orbital to render
     * @param resolution The resolution of the cube
     */
    void calculateOrbitalFromWidget(unsigned int orbital,
                                    double resolution);

    /**
     * Calculate all molecular orbitals at low priority and low
     * resolution.
     */
    void precalculateOrbitals();

    /**
     * Add an orbital calculation to the queue. Lower priority values
     * run first. Do not set automatic calculations to priority zero,
     * this is reserved for user requested calculations and will run
     * first, displaying a progress dialog.
     *
     * @param orbital Orbital number
     * @param resolution Resolution of grid
     * @param isoval Isovalue for surface
     * @param boxPadding Box padding for orbital render cube
     * @param priority Priority. Default = 0 (user requested)
     */
    void addCalculationToQueue(unsigned int orbital,
                               double resolution,
                               double isoval,
                               double boxPadding,
                               unsigned int priority = 0);
    /**
     * Check that no calculations are currently running and start the
     * highest priority calculation.
     */
    void checkQueue();

    /**
     * Start or resume the calculation at the indicated index of the
     * queue.
     */
    void startCalculation(unsigned int queueIndex);

    void calculateCube();
    void calculateCubeDone();
    void calculatePosMesh();
    void calculatePosMeshDone();
    void calculateNegMesh();
    void calculateNegMeshDone();
    void calculationComplete();

    /**
     * Draw the indicated orbital on the GLWidget
     */
    void renderOrbital(unsigned int orbital);

    /**
     * Update the progress of the current calculation
     */
    void updateProgress(int current);

  private:

    QDockWidget *m_dock;
    OrbitalWidget *m_widget;
    QMutex *m_runningMutex;

    QList<calcInfo> m_queue;
    int m_currentRunningCalculation;
    MeshGenerator *m_meshGen;
    OpenQube::BasisSet *m_basis;
    QList<QAction *> m_actions;
    Molecule *m_molecule;
    OpenQube::Cube *m_qube;
    QTime m_time;
  };

  class OrbitalExtensionFactory : public QObject, public PluginFactory
  {
      Q_OBJECT
      Q_INTERFACES(Avogadro::PluginFactory)
      AVOGADRO_EXTENSION_FACTORY(OrbitalExtension)
  };

} // End namespace Avogadro

#endif
