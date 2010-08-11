/**********************************************************************
  OrbitalExtension - Extension for visualizing molecular orbitals

  Copyright (C) 2010 David C. Lonie
  Copyright (C) 2009 Marcus D. Hanwell

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

#include "orbitalextension.h"

#include "orbitalwidget.h"

#include "basisset.h"
#include "gaussianset.h"
#include "slaterset.h"
#include "gaussianfchk.h"
#include "molpro.h"
#include "mopacaux.h"
#include "molden.h"

#include <avogadro/molecule.h>
#include <avogadro/cube.h>
#include <avogadro/mesh.h>
#include <avogadro/meshgenerator.h>
#include <avogadro/engine.h>
#include <avogadro/glwidget.h>

#include <QDir>
#include <QFileInfo>
#include <QMessageBox>
#include <QTime>

namespace Avogadro
{

  OrbitalExtension::OrbitalExtension(QObject* parent) :
    DockExtension(parent),
    m_dock(0),
    m_widget(0),
    m_runningMutex(new QMutex),
    m_currentRunningCalculation(-1),
    m_meshGen(0),
    m_basis(0),
    m_molecule(0)
  {
    QAction* action = new QAction(this);
    action->setText(tr("Molecular Orbitals..."));
    m_actions.append(action);
  }

  OrbitalExtension::~OrbitalExtension()
  {
    delete m_runningMutex;
  }

  QList<QAction *> OrbitalExtension::actions() const
  {
    return m_actions;
  }

  QString OrbitalExtension::menuPath(QAction*) const
  {
    return tr("E&xtensions");
  }

  QDockWidget * OrbitalExtension::dockWidget()
  {
    if (!m_dock) {
      m_dock = new OrbitalDock( tr("Orbitals"),
                                qobject_cast<QWidget *>(parent()) );
      m_dock->setObjectName("orbitalDock");
      if (!m_widget) {
        m_widget = new OrbitalWidget(m_dock);
        connect(m_widget, SIGNAL(orbitalSelected(unsigned int)),
                this, SLOT(renderOrbital(unsigned int)));
        connect(m_widget, SIGNAL(renderRequested(unsigned int, double)),
                this, SLOT(calculateOrbitalFromWidget(unsigned int, double)));
        connect(m_widget, SIGNAL(calculateAll()),
                this, SLOT(precalculateOrbitals()));
      }
    }

    m_dock->setWidget(m_widget);
    m_dock->setVisible(true);
    return m_dock;
  }

  void OrbitalExtension::setMolecule(Molecule *molecule)
  {
    m_molecule = molecule;

    // Stuff we manage that will not be valid any longer
    m_queue.clear();
    m_currentRunningCalculation = -1;

    if (m_basis) {
      delete m_basis;
      m_basis = 0;
    }

    loadBasis();

    // Show dock
    if (m_dock &&
        molecule &&
        m_basis) {
      m_widget->setEnabled(true);
      if (!m_dock->toggleViewAction()->isChecked())
        m_dock->toggleViewAction()->activate(QAction::Trigger);
    }

    // Send MO data to table
    if (m_basis) {
      QList<Orbital> list;
      unsigned int homo = ceil( m_basis->numElectrons() / 2.0 );
      unsigned int lumo = homo + 1;
      unsigned int count = homo - 1;
      bool leqHOMO = true; // orbital <= homo
      for (unsigned int i = 0; i < m_basis->numMOs(); i++) {
        QString num = "";
        if (i+1 != homo && i+1 != lumo) {
          num = (leqHOMO) ? "-" : "+";
          num += QString::number(count);
        }

        QString desc = QString("%1")
          // (HOMO|LUMO)(+|-)[0-9]+
          .arg( (leqHOMO)
                ? tr("HOMO", "Highest Occupied MO") + num
                : tr("LUMO", "Lowest Unoccupied MO") + num
                );
        qDebug() << desc;

        Orbital orb;
        orb.energy = 0; // TODO
        orb.index = i;
        orb.description = desc;
        orb.queueEntry = 0;
        orb.min = 0;
        orb.max = 0;
        orb.current = 0;

        list.append(orb);
        if (i+1 < homo) count--;
        else if (i+1 == homo) leqHOMO = false;
        else if (i+1 >= lumo) count++;
      }
      m_widget->fillTable(list);
    }

    m_time.start();
    precalculateOrbitals();
  }

  void OrbitalExtension::calculateOrbitalFromWidget(unsigned int orbital,
                                                    double resolution)
  {
    addCalculationToQueue(orbital, resolution, m_widget->isovalue(), 0);
    checkQueue();
  }


  void OrbitalExtension::precalculateOrbitals()
  {
    if (m_basis) {
      // Determine HOMO
      unsigned int homo = ceil( m_basis->numElectrons() / 2.0 );

      // Initialize prioritizer at HOMO's index
      int priority = homo;

      // Loop through all MOs, submitting calculations with increasing
      // priority until HOMO is reached, submit both HOMO and LUMO at
      // priority=1, then start increasing for orbitals above LUMO.
      // E.g,
      // .... HOMO-2 HOMO-1 HOMO LUMO LUMO+1 LUMO+2 ... << orbitals
      // ....   3      2     1    1     2      3    ... << priorities
      for (unsigned int i = 0; i < m_basis->numMOs(); i++) {
        addCalculationToQueue(i+1,  // orbital
                              OrbitalWidget::OrbitalQualityToDouble(m_widget->defaultQuality()),
                              m_widget->isovalue(),
                              priority);

        // Update priority. Stays the same when i = homo.
        if ( i + 1 < homo ) priority--;
        else if ( i + 1 > homo) priority++;
      }
    }
    checkQueue();
  }

  void OrbitalExtension::addCalculationToQueue(unsigned int orbital,
                                               double resolution,
                                               double isovalue,
                                               unsigned int priority)
  {
    // Create new queue entry
    calcInfo newCalc;
    newCalc.orbital = orbital;
    newCalc.resolution = resolution;
    newCalc.isovalue = isovalue;
    newCalc.priority = priority;
    newCalc.state = NotStarted;

    // Add new calculation
    m_queue.append(newCalc);

    // Set progress to show 0%
    m_widget->calculationQueued(newCalc.orbital);

    qDebug() << "New calculation added:" << newCalc.orbital
             << "Priority:" << newCalc.priority;
  }

  void OrbitalExtension::startCalculation(unsigned int queueIndex)
  {
    // This will launch calculateMesh when finished.
    m_currentRunningCalculation = queueIndex;

    calcInfo *info = &m_queue[m_currentRunningCalculation];

    qDebug() << info->orbital << " startCalculation() called";

    switch (info->state) {
    case NotStarted: // Start calculation
      calculateCube();
      break;
    case Running: // Nothing below should happen...
      qWarning() << "startCalculation called on a running calc...";
      break;
    case Completed:
      qWarning() << "startCalculation called on a completed calc...";
      break;
    case Canceled:
      qWarning() << "startCalculation called on a canceled calc...";
      break;
    }
  }

  void OrbitalExtension::calculateCube()
  {
    calcInfo *info = &m_queue[m_currentRunningCalculation];

    info->state = Running;

    // Check if the cube we want already exists
    for (int i = 0; i < m_queue.size(); i++) {
      calcInfo *cI = &m_queue[i];
      if (cI->state == Completed &&
          cI->orbital == info->orbital &&
          cI->resolution == info->resolution) {
        info->cube = cI->cube;
        qDebug() << "Reusing cube from calculation " << i << ":\n"
                 << "\tOrbital " << cI->orbital << "\n"
                 << "\tResolution " << cI->resolution;
        calculatePosMesh();
        return;
      }
    }

    // Create new cube
    Cube *cube = m_molecule->addCube();
    info->cube = cube;
    cube->setLimits(m_molecule, info->resolution, 2.5);

    m_basis->calculateCubeMO(cube, info->orbital);
    connect(&m_basis->watcher(), SIGNAL(finished()),
            this, SLOT(calculateCubeDone()));

    m_widget->initializeProgress(info->orbital,
                                 m_basis->watcher().progressMinimum(),
                                 m_basis->watcher().progressMaximum(),
                                 1, 3);

    connect(&m_basis->watcher(), SIGNAL(progressValueChanged(int)),
            this, SLOT(updateProgress(int)));

    qDebug() << info->orbital << " Cube calculation started.";
  }

  void OrbitalExtension::calculateCubeDone()
  {
    calcInfo *info = &m_queue[m_currentRunningCalculation];

    qDebug() << info->orbital << " Cube calculation finished.";

    disconnect(&m_basis->watcher(), 0,
               this, 0);

    calculatePosMesh();
  }

  void OrbitalExtension::calculatePosMesh()
  {
    calcInfo *info = &m_queue[m_currentRunningCalculation];

    info->state = Running;

    // Check if the mesh we want already exists
    for (int i = 0; i < m_queue.size(); i++) {
      calcInfo *cI = &m_queue[i];
      if (cI->state == Completed &&
          cI->orbital == info->orbital &&
          cI->resolution == info->resolution &&
          cI->isovalue == info->isovalue) {
        info->posMesh = cI->posMesh;
        qDebug() << "Reusing posMesh from calculation " << i << ":\n"
                 << "\tOrbital " << cI->orbital << "\n"
                 << "\tResolution " << cI->resolution << "\n"
                 << "\tIsovalue " << cI->isovalue;
        m_widget->nextProgressStage(info->orbital, 0, 100);
        calculateNegMesh();
        return;
      }
    }

    Cube *cube = info->cube;

    Mesh *mesh = m_molecule->addMesh();
    mesh->setName(cube->name());
    mesh->setIsoValue(info->isovalue);
    mesh->setCube(cube->id());
    info->posMesh = mesh;

    if (m_meshGen) {
      m_meshGen->disconnect();
      delete m_meshGen;
    }
    m_meshGen = new MeshGenerator;

    connect(m_meshGen, SIGNAL(finished()),
            this, SLOT(calculatePosMeshDone()));

    m_meshGen->initialize(cube, mesh, info->isovalue);

    m_widget->nextProgressStage(info->orbital,
                                m_meshGen->progressMinimum(),
                                m_meshGen->progressMaximum());

    m_meshGen->start();

    connect(m_meshGen, SIGNAL(progressValueChanged(int)),
            this, SLOT(updateProgress(int)));

    qDebug() << info->orbital << " posMesh calculation started.";
  }

  void OrbitalExtension::calculatePosMeshDone()
  {
    calcInfo *info = &m_queue[m_currentRunningCalculation];

    disconnect(m_meshGen, 0,
               this, 0);

    qDebug() << info->orbital << " posMesh calculation finished.";

    calculateNegMesh();
  }

  void OrbitalExtension::calculateNegMesh()
  {
    calcInfo *info = &m_queue[m_currentRunningCalculation];

    info->state = Running;

    // Check if the mesh we want already exists
    for (int i = 0; i < m_queue.size(); i++) {
      calcInfo *cI = &m_queue[i];
      if (cI->state == Completed &&
          cI->orbital == info->orbital &&
          cI->resolution == info->resolution &&
          cI->isovalue == info->isovalue) {
        info->negMesh = cI->negMesh;
        qDebug() << "Reusing posMesh from calculation " << i << ":\n"
                 << "\tOrbital " << cI->orbital << "\n"
                 << "\tResolution " << cI->resolution << "\n"
                 << "\tIsovalue " << cI->isovalue;
        m_widget->nextProgressStage(info->orbital, 0, 100);
        calculationComplete();
        return;
      }
    }

    Cube *cube = info->cube;

    Mesh *mesh = m_molecule->addMesh();
    info->negMesh = mesh;
    mesh->setName(cube->name());
    mesh->setIsoValue(0.0 - info->isovalue);
    mesh->setCube(cube->id());

    if (m_meshGen) {
      m_meshGen->disconnect();
      delete m_meshGen;
    }
    m_meshGen = new MeshGenerator;

    connect(m_meshGen, SIGNAL(finished()),
            this, SLOT(calculateNegMeshDone()));

    m_meshGen->initialize(cube, mesh, 0.0 - info->isovalue,
                          true); // Reverse the surface

    m_widget->nextProgressStage(info->orbital,
                                m_meshGen->progressMinimum(),
                                m_meshGen->progressMaximum());

    m_meshGen->start();

    connect(m_meshGen, SIGNAL(progressValueChanged(int)),
            this, SLOT(updateProgress(int)));

    qDebug() << info->orbital << " negMesh calculation started.";
  }

  void OrbitalExtension::calculateNegMeshDone()
  {
    calcInfo *info = &m_queue[m_currentRunningCalculation];

    disconnect(m_meshGen, 0,
               this, 0);

    qDebug() << info->orbital << " negMesh calculation finished.";
    calculationComplete();
  }

  void OrbitalExtension::calculationComplete()
  {
    calcInfo *info = &m_queue[m_currentRunningCalculation];

    m_widget->calculationComplete(info->orbital);

    info->state = Completed;
    m_currentRunningCalculation = -1;
    m_runningMutex->unlock();

    // Show orbital is calculation was user requested
    if (info->priority == 0)
      m_widget->selectOrbital(info->orbital);

    qDebug() << info->orbital << " all calculations complete.";
    checkQueue();
  }

  void OrbitalExtension::renderOrbital(unsigned int orbital)
  {
    qDebug() << "Attempting to render orbital " << orbital;
    // TODO Actually select the engine. For now, just use the first
    // surface engine
    Engine *engine = 0;

    foreach (Engine *e, GLWidget::current()->engines()) {
      if (e->identifier() == "Surfaces") {
        engine = e;
      }
    }
    if (!engine)
      return; // prevent a crash if the surface engine isn't loaded

    // Find the most recent calc matching the selected orbital:
    calcInfo calc;
    int index = -1;
    for (int i = 0; i < m_queue.size(); i++) {
      calc = m_queue[i];
      if ( calc.orbital == orbital &&
           calc.state == Completed ) {
        index = i;
      }
    }

    if (index == -1) {
      qDebug() << "Orbital not found, or still calculating. Cannot render.";
      return;
    }

    if (engine) {
      QSettings settings;
      engine->writeSettings(settings);
      settings.setValue("colorMode", 1);
      settings.setValue("mesh1Id",static_cast<int>(m_queue[index].posMesh->id()));
      settings.setValue("mesh2Id",static_cast<int>(m_queue[index].negMesh->id()));
      engine->readSettings(settings);
      engine->setEnabled(true);
      // Trigger a repaint with the new mesh
      /// FIXME Should be using m_molecule->update() to trigger a repaint in
      /// all open displays, this currently causes crashes - need to track
      /// down the cause.
      GLWidget::current()->update();
    }
    else
      qDebug() << "Engine is null - no engines of this type loaded.";
  }

  void OrbitalExtension::checkQueue()
  {
    if (!m_runningMutex->tryLock())
      return;

    // Create a hash: keys=priority, values=indices

    QHash<int, int> hash;
    CalcState state;

    for (int i = 0; i < m_queue.size(); i++) {
      state = m_queue.at(i).state;

      // If there is already a running job, return.
      if ( state == Running ) {
        return;
      }

      if ( state == NotStarted ) {
        hash.insert(m_queue[i].priority, i);
      }
    }

    // Do nothing if all calcs are finished.
    if (hash.size() == 0) {
      m_runningMutex->unlock();
      qDebug("Finished queue. Time elapsed: %10.4f s", (m_time.elapsed() / 1000.0f));
      return;
    }

    QList<int> priorities = hash.keys();
    qSort(priorities);
    startCalculation(hash.value(priorities.first()));
  }

  bool OrbitalExtension::loadBasis()
  {
    if (m_molecule->fileName().isEmpty()) {
      return false;
    }

    // Everything looks good, a new basis set needs to be loaded
    // Check for files in this directory -- first the file itself
    // and then any other similar files

    QFileInfo parentInfo(m_molecule->fileName());
    // Look for files with the same basename, but different extensions
    QDir parentDir = parentInfo.dir();
    QStringList nameFilters;
    nameFilters << parentInfo.baseName() + ".*";

    QStringList matchingFiles = parentDir.entryList(nameFilters,
                                                    QDir::Readable | QDir::Files);
    matchingFiles.prepend(parentInfo.fileName());

    // TODO: Add a warning dialog to make sure that opening up a new file is OK
    // (i.e., that we found the right checkpoint file)
    foreach(const QString &fileName, matchingFiles) {
      QString fullFileName = parentInfo.path() + '/' + fileName;
      QFileInfo info(fullFileName);
      QString completeSuffix = info.completeSuffix();

      if (completeSuffix.contains("fchk", Qt::CaseInsensitive)
          || completeSuffix.contains("fch", Qt::CaseInsensitive)
          || completeSuffix.contains("fck", Qt::CaseInsensitive)) {
        if (m_basis) {
          delete m_basis;
          m_basis = 0;
        }
        GaussianSet *gaussian = new GaussianSet;
        GaussianFchk fchk(fullFileName, gaussian);

        m_basis = gaussian;
        return true;
      }
      else if (completeSuffix.contains("aux", Qt::CaseInsensitive)) {
        if (m_basis) {
          delete m_basis;
          m_basis = 0;
        }
        SlaterSet *slater = new SlaterSet;
        MopacAux aux(fullFileName, slater);

        m_basis = slater;
        return true;
      }
      else if (completeSuffix.contains("mpo", Qt::CaseInsensitive)) {
        if (m_basis) {
          delete m_basis;
          m_basis = 0;
        }
        GaussianSet *gaussian = new GaussianSet;
        Molpro mpo(fullFileName, gaussian);

        m_basis = gaussian;
        return true;
      }
      else if (completeSuffix.contains("molden", Qt::CaseInsensitive)
          || completeSuffix.contains("mold", Qt::CaseInsensitive)
          || completeSuffix.contains("molf", Qt::CaseInsensitive)) {
        if (m_basis) {
          delete m_basis;
          m_basis = 0;
        }
        GaussianSet *gaussian = new GaussianSet;
        MoldenFile fchk(fullFileName, gaussian);

        m_basis = gaussian;
        return true;
      }
    }

    return false;
  }

  void OrbitalExtension::updateProgress(int current)
  {
    calcInfo *info = &m_queue[m_currentRunningCalculation];
    int orbital = info->orbital;
    m_widget->updateProgress(orbital, current);
  }

} // End namespace Avogadro

Q_EXPORT_PLUGIN2(orbitalextension, Avogadro::OrbitalExtensionFactory)
