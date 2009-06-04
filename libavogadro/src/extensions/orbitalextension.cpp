/**********************************************************************
  OrbitalExtension - Extension for generating cubes and meshes

  Copyright (C) 2008-2009 Marcus D. Hanwell

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

#include "gaussianfchk.h"
#include "slaterset.h"
#include "molpro.h"
#include "mopacaux.h"
#include "vdwsurface.h"

#include <vector>
#include <avogadro/toolgroup.h>
#include <avogadro/molecule.h>
#include <avogadro/atom.h>
#include <avogadro/cube.h>
#include <avogadro/mesh.h>
#include <avogadro/meshgenerator.h>
#include <avogadro/engine.h>
#include <avogadro/neighborlist.h>

#include <Eigen/Core>

#include <QProgressDialog>
#include <QCoreApplication>
#include <QFileInfo>
#include <QMessageBox>
#include <QTime>
#include <QDir>
#include <QDebug>

using Eigen::Vector3f;
using Eigen::Vector3d;
using Eigen::Vector3i;

namespace Avogadro
{
  OrbitalExtension::OrbitalExtension(QObject* parent) : Extension(parent),
    m_glwidget(0), m_orbitalDialog(0), m_molecule(0), m_basis(0), m_slater(0),
    m_progress(0), m_timer(0), m_mesh1(0), m_mesh2(0), m_meshGen1(0),
    m_meshGen2(0), m_VdWsurface(0)
  {
    QAction* action = new QAction(this);
    action->setText(tr("Create Surfaces..."));
    m_actions.append(action);
  }

  OrbitalExtension::~OrbitalExtension()
  {
    delete m_basis;
    m_basis = 0;
    delete m_slater;
    m_slater = 0;
    delete m_meshGen1;
    m_meshGen1 = 0;
    delete m_meshGen2;
    m_meshGen2 = 0;
    delete m_timer;
    m_timer = 0;
    delete m_VdWsurface;
    m_VdWsurface = 0;
  }

  QList<QAction *> OrbitalExtension::actions() const
  {
    return m_actions;
  }

  QString OrbitalExtension::menuPath(QAction*) const
  {
    return tr("E&xtensions");
  }

  QUndoCommand* OrbitalExtension::performAction(QAction *, GLWidget *widget)
  {
    m_glwidget = widget;
    if (!m_orbitalDialog) {
      m_orbitalDialog = new OrbitalDialog(static_cast<QWidget *>(parent()));
      m_orbitalDialog->setGLWidget(widget);
      m_orbitalDialog->setMolecule(m_molecule);
      connect(m_orbitalDialog, SIGNAL(calculateMO(int)),
              this, SLOT(calculateMO(int)));
      connect(m_orbitalDialog, SIGNAL(calculateAll()),
              this, SLOT(calculateAll()));
      connect(m_orbitalDialog, SIGNAL(calculateDensity()),
              this, SLOT(calculateDensity()));
      connect(m_orbitalDialog, SIGNAL(calculateMesh(int, double, int)),
              this, SLOT(generateMesh(int, double, int)));
      connect(m_orbitalDialog, SIGNAL(calculateVdWCube()),
              this, SLOT(calculateVdWCube()));
      connect(m_orbitalDialog, SIGNAL(calculateVdWMesh(int, double)),
              this, SLOT(generateVdWMesh(int, double)));
      setDefaultCube();
      loadBasis();
      m_orbitalDialog->show();
    }
    else {
      m_orbitalDialog->setGLWidget(widget);
      setDefaultCube();
      loadBasis();
      m_orbitalDialog->show();
    }
    return 0;
  }

  void OrbitalExtension::setMolecule(Molecule *molecule)
  {
    m_molecule = molecule;
    delete m_slater;
    m_slater = 0;
    delete m_basis;
    m_basis = 0;
    delete m_VdWsurface;
    m_VdWsurface = 0;
    m_mesh1 = 0;
    m_mesh2 = 0;

    if (m_orbitalDialog)
      m_orbitalDialog->setMolecule(molecule);
  }

  void OrbitalExtension::setDefaultCube()
  {
    // Now to set the default cube...
    Cube cube;
    double step = 0.18;
    cube.setLimits(m_molecule, step, 2.5);
    Vector3d min = cube.min();// / BOHR_TO_ANGSTROM;
    Vector3i dim = cube.dimensions();
    // Set these values on the form - they can then be altered by the user
    m_orbitalDialog->setCube(min, dim.x(), dim.y(), dim.z(), step);
  }

  bool OrbitalExtension::loadBasis()
  {
    if (m_molecule->fileName().isEmpty()) {
      return false;
    }
    else if (m_loadedFileName == m_molecule->fileName()) {
      return true;
    }
    else if (QFileInfo(m_molecule->fileName()).baseName()
             == QFileInfo(m_loadedFileName).baseName()) {
      return true;
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
        if (m_slater) {
          delete m_slater;
          m_slater = 0;
        }
        if (m_basis) {
          delete m_basis;
          m_basis = 0;
        }
        m_basis = new BasisSet;
        GaussianFchk fchk(fullFileName, m_basis);

        m_orbitalDialog->setMOs(m_basis->numMOs());
        for (int i = 0; i < m_basis->numMOs(); ++i) {
          if (m_basis->HOMO(i)) m_orbitalDialog->setHOMO(i);
          else if (m_basis->LUMO(i)) m_orbitalDialog->setLUMO(i);
        }
        return true;
      }
      else if (completeSuffix.contains("aux", Qt::CaseInsensitive)) {
        if (m_slater) {
          delete m_slater;
          m_slater = 0;
        }
        if (m_basis) {
          delete m_basis;
          m_basis = 0;
        }
        m_slater = new SlaterSet;
        MopacAux aux(fullFileName, m_slater);

        // Set the number of MOs
        m_orbitalDialog->setMOs(m_slater->numMOs());
        for (unsigned int i = 0; i < m_slater->numMOs(); ++i) {
          if (m_slater->HOMO(i)) m_orbitalDialog->setHOMO(i);
          else if (m_slater->LUMO(i)) m_orbitalDialog->setLUMO(i);
        }
        return true;
      }
      else if (completeSuffix.contains("mpo", Qt::CaseInsensitive)) {
        if (m_slater) {
          delete m_slater;
          m_slater = 0;
        }
        if (m_basis) {
          delete m_basis;
          m_basis = 0;
        }
        m_basis = new BasisSet;
        Molpro mpo(fullFileName, m_basis);
        qDebug() << "numMOs: " << m_basis->numMOs();
        m_orbitalDialog->setMOs(m_basis->numMOs());
        for (int i = 0; i < m_basis->numMOs(); ++i) {
          if (m_basis->HOMO(i)) m_orbitalDialog->setHOMO(i);
          else if (m_basis->LUMO(i)) m_orbitalDialog->setLUMO(i);
        }
        return true;
      }

    }

    // We didn't find an appropriate filetype
    qDebug() << "baseName:" << parentInfo.completeSuffix();
    m_orbitalDialog->setCurrentTab(0);
    return false;
  }

  void OrbitalExtension::calculateMO(int mo, const Vector3d &origin,
                                     const Vector3i &steps, double stepSize)
  {
    const double BOHR_TO_ANGSTROM = 0.529177249;
    // Slater type orbital
    if (m_slater) {
      // We have a slater type orbital....
      qDebug() << "Adding a slater type orbital...." << mo;
      Cube *cube = m_molecule->addCube();
      cube->setName(tr("MO %L1", "Molecular Orbital").arg(mo));
      cube->setLimits(origin * BOHR_TO_ANGSTROM, steps,
                      stepSize * BOHR_TO_ANGSTROM);
      if (!m_timer) {
        m_timer = new QTime;
        m_timer->start();
      }
      m_slater->calculateCubeMO(cube, mo);

      // Set up a progress dialog
      if (!m_progress) {
        m_progress = new QProgressDialog(m_orbitalDialog);
        m_progress->setCancelButtonText(tr("Abort Calculation"));
        m_progress->setWindowModality(Qt::NonModal);
      }

      // Set up the progress bar
      m_progress->setWindowTitle(tr("Calculating MO %L1", "Molecular Orbital").arg(mo));
      m_progress->setRange(m_slater->watcher().progressMinimum(),
                           m_slater->watcher().progressMinimum());
      m_progress->setValue(m_slater->watcher().progressValue());
      m_progress->show();

      // Connect the signals and slots
      connect(&m_slater->watcher(), SIGNAL(progressValueChanged(int)),
              m_progress, SLOT(setValue(int)));
      connect(&m_slater->watcher(), SIGNAL(progressRangeChanged(int, int)),
              m_progress, SLOT(setRange(int, int)));
      connect(m_progress, SIGNAL(canceled()),
              this, SLOT(slaterCanceled()));
      connect(&m_slater->watcher(), SIGNAL(finished()),
              this, SLOT(slaterDone()));
      m_orbitalDialog->enableCalculation(false);
    }
    else if (m_basis) {
      // Calculate each point in the cube in parallel
      Cube *cube = m_molecule->addCube();
      cube->setName(tr("MO %L1", "Molecular Orbital").arg(mo));
      cube->setLimits(origin * BOHR_TO_ANGSTROM, steps,
                      stepSize * BOHR_TO_ANGSTROM);
      if (!m_timer) {
        m_timer = new QTime;
        m_timer->start();
      }
      m_basis->calculateCubeMO(cube, mo);

     // Set up a progress dialog
      if (!m_progress) {
        m_progress = new QProgressDialog(m_orbitalDialog);
        m_progress->setCancelButtonText(tr("Abort Calculation"));
        m_progress->setWindowModality(Qt::NonModal);
      }

      // Set up the progress bar
      m_progress->setWindowTitle(tr("Calculating MO %L1", "Molecular Orbital").arg(mo));
      m_progress->setRange(m_basis->watcher().progressMinimum(),
                           m_basis->watcher().progressMinimum());
      m_progress->setValue(m_basis->watcher().progressValue());
      m_progress->show();

      // Connect signals and slots
      connect(&m_basis->watcher(), SIGNAL(progressValueChanged(int)),
              m_progress, SLOT(setValue(int)));
      connect(&m_basis->watcher(), SIGNAL(progressRangeChanged(int, int)),
              m_progress, SLOT(setRange(int, int)));
      connect(m_progress, SIGNAL(canceled()),
              this, SLOT(calculation2Canceled()));
      connect(&m_basis->watcher(), SIGNAL(finished()),
              this, SLOT(calculation2Done()));
      m_orbitalDialog->enableCalculation(false);
    }
  }

  void OrbitalExtension::calculateESP(Mesh *mesh)
  {
    //                                          //
    //     |    red    green     blue           //
    // 1.0 |...--+       +       +--...         //
    //     |      \     / \     /               //
    //     |       \   /   \   /                //
    //     |        \ /     \ /                 //
    //     |         X       X                  //
    //     |        / \     / \                 //
    //     |       /   \   /   \                //
    //     |      /     \ /     \               //
    // 0.0 +...--+-------+-------+--...-->      //
    //           a      0.0      b      energy
    //
    //  a = 20 * energy
    //  b = 20 * energy
    //
    // Calculate the ESP mapped onto the vertices of the Mesh supplied
    if (!m_molecule)
      return;

    NeighborList *nbrList = new NeighborList(m_molecule, 7.0, false, 2);

    std::vector<QColor> colors;
    for(unsigned int i=0; i < mesh->vertices().size(); ++i) {
      const Vector3f *v = mesh->vertex(i);

      GLfloat red, green, blue;
      double energy = 0.0;

      QList<Atom*> nbrAtoms = nbrList->nbrs(v);
      foreach(Atom *a, nbrAtoms) {
        Vector3f dist = a->pos()->cast<float>() - v->cast<float>();
        energy += a->partialCharge() / dist.squaredNorm();
      }

      // Chemistry convention: red = negative, blue = positive
      QColor color;
      if (energy < 0.0) {
        red = -20.0*energy;
        if (red >= 1.0) {
          color.setRgbF(1.0, 0.0, 0.0, 1.0);
        }
        else {
          green = 1.0 - red;
          color.setRgbF(red, green, 0.0, 1.0);
        }
      }
      else if (energy > 0.0) {
        blue = 20.0*energy;
        if (blue >= 1.0) {
          color.setRgbF(0.0, 0.0, 1.0, 1.0);
        }
        else {
          green = 1.0 - blue;
          color.setRgbF(0.0, green, blue, 1.0);
        }
      }
      else
        color.setRgbF(0.0, 1.0, 0.0, 1.0);

      colors.push_back(color);
    }
    mesh->setColors(colors);
  }

  void OrbitalExtension::calculateMO(int n)
  {
    if (!m_basis && !m_slater)
      return;

    const double BOHR_TO_ANGSTROM = 0.529177249;
    const double ANGSTROM_TO_BOHR = 1.0/BOHR_TO_ANGSTROM;
    // Calculate MO n and add the cube to the molecule...
    ++n; // MOs are 1 based, not 0 based...
    qDebug() << "Calculating MO" << n;
    double step = m_orbitalDialog->stepSize() * ANGSTROM_TO_BOHR;
    Vector3d origin = ANGSTROM_TO_BOHR * m_orbitalDialog->origin();
    Vector3i nSteps = m_orbitalDialog->steps();
    // Debug output
    qDebug() << "Origin = " << origin.x() << origin.y() << origin.z()
             << "\nStep = " << step << ", nz = " << nSteps.z();

    m_currentMO = 0; // Only calculating one MO
    calculateMO(n, origin, nSteps, step);
  }

  void OrbitalExtension::calculateAll()
  {
    if (!m_basis && !m_slater)
      return;

    const double BOHR_TO_ANGSTROM = 0.529177249;
    const double ANGSTROM_TO_BOHR = 1.0/BOHR_TO_ANGSTROM;
    qDebug() << "Calculating all MOs";
    m_stepSize = m_orbitalDialog->stepSize() * ANGSTROM_TO_BOHR;
    m_origin = ANGSTROM_TO_BOHR * m_orbitalDialog->origin();
    m_steps = m_orbitalDialog->steps();

    // Call the calculation, starting at MO 1
    m_currentMO = 1;
    calculateMO(m_currentMO, m_origin, m_steps, m_stepSize);
  }

  void OrbitalExtension::calculateDensity()
  {
    if (!m_basis && !m_slater)
      return;

    const double BOHR_TO_ANGSTROM = 0.529177249;
    const double ANGSTROM_TO_BOHR = 1.0/BOHR_TO_ANGSTROM;
    qDebug() << "Calculating electron density...";
    double stepSize = m_orbitalDialog->stepSize() * ANGSTROM_TO_BOHR;
    Vector3d origin = ANGSTROM_TO_BOHR * m_orbitalDialog->origin();
    Vector3i steps = m_orbitalDialog->steps();

    // Call the calculation, starting at MO 1
    m_currentMO = 0;
    if (m_slater) {
      // We have a slater type orbital
      Cube *cube = m_molecule->addCube();
      cube->setName(QString(tr("Electron Density")));
      cube->setLimits(origin * BOHR_TO_ANGSTROM, steps,
                      stepSize * BOHR_TO_ANGSTROM);
      cube->setProperty("electronDensity", true);
      if (!m_timer) {
        m_timer = new QTime;
        m_timer->start();
      }
      m_slater->calculateCubeDensity(cube);

      // Set up a progress dialog
      if (!m_progress) {
        m_progress = new QProgressDialog(m_orbitalDialog);
        m_progress->setCancelButtonText(tr("Abort Calculation"));
        m_progress->setWindowModality(Qt::NonModal);
      }

      // Set up the progress bar
      m_progress->setWindowTitle(tr("Calculating Electron Density"));
      m_progress->setRange(m_slater->watcher().progressMinimum(),
                           m_slater->watcher().progressMinimum());
      m_progress->setValue(m_slater->watcher().progressValue());
      m_progress->show();

      // Connect the signals and slots
      connect(&m_slater->watcher(), SIGNAL(progressValueChanged(int)),
              m_progress, SLOT(setValue(int)));
      connect(&m_slater->watcher(), SIGNAL(progressRangeChanged(int, int)),
              m_progress, SLOT(setRange(int, int)));
      connect(m_progress, SIGNAL(canceled()),
              this, SLOT(slaterCanceled()));
      connect(&m_slater->watcher(), SIGNAL(finished()),
              this, SLOT(slaterDone()));
      m_orbitalDialog->enableCalculation(false);
    }
    else if (m_basis) {
      // Gaussian type orbital
      Cube *cube = m_molecule->addCube();
      cube->setName(QString(tr("Electron Density")));
      cube->setLimits(origin * BOHR_TO_ANGSTROM, steps,
                      stepSize * BOHR_TO_ANGSTROM);
      cube->setProperty("electronDensity", true);
      if (!m_timer) {
        m_timer = new QTime;
        m_timer->start();
      }
      m_basis->calculateCubeDensity(cube);

      // Set up a progress dialog
      if (!m_progress) {
        m_progress = new QProgressDialog(m_orbitalDialog);
        m_progress->setCancelButtonText(tr("Abort Calculation"));
        m_progress->setWindowModality(Qt::NonModal);
      }

      // Set up the progress bar
      m_progress->setWindowTitle(tr("Calculating Electron Density"));
      m_progress->setRange(m_basis->watcher().progressMinimum(),
                           m_basis->watcher().progressMinimum());
      m_progress->setValue(m_basis->watcher().progressValue());
      m_progress->show();

      // Connect the signals and slots
      connect(&m_basis->watcher(), SIGNAL(progressValueChanged(int)),
              m_progress, SLOT(setValue(int)));
      connect(&m_basis->watcher(), SIGNAL(progressRangeChanged(int, int)),
              m_progress, SLOT(setRange(int, int)));
      connect(m_progress, SIGNAL(canceled()),
              this, SLOT(calculation2Canceled()));
      connect(&m_basis->watcher(), SIGNAL(finished()),
              this, SLOT(calculation2Done()));
      m_orbitalDialog->enableCalculation(false);
    }
  }

  void OrbitalExtension::calculationDone()
  {
    disconnect(&m_basis->watcher(), SIGNAL(progressValueChanged(int)),
               m_progress, SLOT(setValue(int)));
    disconnect(&m_basis->watcher(), SIGNAL(progressRangeChanged(int, int)),
            m_progress, SLOT(setRange(int, int)));
    disconnect(m_progress, SIGNAL(canceled()),
            this, SLOT(calculationCanceled()));
    disconnect(&m_basis->watcher(), SIGNAL(finished()),
            this, SLOT(calculationDone()));

    qDebug() << "Whole cube calculation done in" << m_timer->elapsed() / 1000.0
             << "seconds";
    delete m_timer;
    m_timer = 0;
    m_molecule->update();
    m_orbitalDialog->enableCalculation(true);
  }

  void OrbitalExtension::calculation2Done()
  {
    // Calculation complete
    if (!m_currentMO) {
      disconnect(&m_basis->watcher(), 0, m_progress, 0);
      disconnect(m_progress, SIGNAL(canceled()),
                 this, SLOT(calculation2Canceled()));
      disconnect(&m_basis->watcher(), 0, this, 0);

      qDebug() << "Single points calculation done in" << m_timer->elapsed() / 1000.0
               << "seconds";
      delete m_timer;
      m_timer = 0;
      m_molecule->update();
      m_orbitalDialog->enableCalculation(true);
    }
    else if (static_cast<unsigned int>(m_basis->numMOs()) == m_currentMO) {
      // All MOs have been calculated
      disconnect(&m_basis->watcher(), 0, m_progress,0);
      disconnect(m_progress, SIGNAL(canceled()),
                 this, SLOT(calculation2Canceled()));
      disconnect(&m_basis->watcher(), 0, this, 0);

      qDebug() << "All cube MOs calculated in" << m_timer->elapsed() / 1000.0
               << "seconds";
      delete m_timer;
      m_timer = 0;
      m_molecule->update();
      m_orbitalDialog->enableCalculation(true);
      m_currentMO = 0;
    }
    else { // More work to do
      disconnect(&m_basis->watcher(), 0, m_progress, 0);
      disconnect(m_progress, SIGNAL(canceled()),
                 this, SLOT(calculation2Canceled()));
      disconnect(&m_basis->watcher(), 0, this, 0);
      calculateMO(++m_currentMO, m_origin, m_steps, m_stepSize);
    }
  }

  void OrbitalExtension::slaterDone()
  {
    if (!m_currentMO) {
      disconnect(&m_slater->watcher(), SIGNAL(progressValueChanged(int)),
                 m_progress, SLOT(setValue(int)));
      disconnect(&m_slater->watcher(), SIGNAL(progressRangeChanged(int, int)),
                 m_progress, SLOT(setRange(int, int)));
      disconnect(m_progress, SIGNAL(canceled()),
                 this, SLOT(slaterCanceled()));
      disconnect(&m_slater->watcher(), SIGNAL(finished()),
                 this, SLOT(slaterDone()));

      qDebug() << "Single points calculation done in" << m_timer->elapsed() / 1000.0
               << "seconds";
      delete m_timer;
      m_timer = 0;
      m_molecule->update();
      m_orbitalDialog->enableCalculation(true);
    }
    else if (m_slater->numMOs() == m_currentMO) {
      disconnect(&m_slater->watcher(), SIGNAL(progressValueChanged(int)),
                 m_progress, SLOT(setValue(int)));
      disconnect(&m_slater->watcher(), SIGNAL(progressRangeChanged(int, int)),
                 m_progress, SLOT(setRange(int, int)));
    disconnect(m_progress, SIGNAL(canceled()),
            this, SLOT(slaterCanceled()));
      disconnect(&m_slater->watcher(), SIGNAL(finished()),
                 this, SLOT(slaterDone()));

      qDebug() << "All cube MOs calculated in" << m_timer->elapsed() / 1000.0
               << "seconds";
      delete m_timer;
      m_timer = 0;
      m_molecule->update();
      m_orbitalDialog->enableCalculation(true);
      m_currentMO = 0;
    }
    else {
      disconnect(&m_slater->watcher(), SIGNAL(progressValueChanged(int)),
                 m_progress, SLOT(setValue(int)));
      disconnect(&m_slater->watcher(), SIGNAL(progressRangeChanged(int, int)),
                 m_progress, SLOT(setRange(int, int)));
    disconnect(m_progress, SIGNAL(canceled()),
            this, SLOT(slaterCanceled()));
      disconnect(&m_slater->watcher(), SIGNAL(finished()),
                 this, SLOT(slaterDone()));
      calculateMO(++m_currentMO, m_origin, m_steps, m_stepSize);
    }
  }

  void OrbitalExtension::calculationCanceled()
  {
    disconnect(&m_basis->watcher(), SIGNAL(progressValueChanged(int)),
               m_progress, SLOT(setValue(int)));
    disconnect(&m_basis->watcher(), SIGNAL(progressRangeChanged(int, int)),
            m_progress, SLOT(setRange(int, int)));
    connect(m_progress, SIGNAL(canceled()),
            this, SLOT(calculationCanceled()));
    disconnect(&m_basis->watcher(), SIGNAL(finished()),
            this, SLOT(calculationDone()));
    m_basis->watcher().cancel();
    qDebug() << "Canceled...";
    m_progress->deleteLater();
    m_orbitalDialog->enableCalculation(true);
  }

  void OrbitalExtension::calculation2Canceled()
  {
    disconnect(&m_basis->watcher(), 0, m_progress, 0);
    connect(m_progress, SIGNAL(canceled()),
            this, SLOT(calculation2Canceled()));
    disconnect(&m_basis->watcher(), 0, this, 0);
    m_basis->watcher().cancel();
    qDebug() << "Canceled...";
    m_orbitalDialog->enableCalculation(true);
    m_currentMO = 0;
  }

  void OrbitalExtension::slaterCanceled()
  {
    disconnect(&m_slater->watcher(), SIGNAL(progressValueChanged(int)),
               m_progress, SLOT(setValue(int)));
    disconnect(&m_slater->watcher(), SIGNAL(progressRangeChanged(int, int)),
            m_progress, SLOT(setRange(int, int)));
    connect(m_progress, SIGNAL(canceled()),
            this, SLOT(slaterCanceled()));
    disconnect(&m_slater->watcher(), SIGNAL(finished()),
            this, SLOT(slaterDone()));
    m_slater->watcher().cancel();
    qDebug() << "Canceled...";
    m_orbitalDialog->enableCalculation(true);
    m_currentMO = 0;
  }

  void OrbitalExtension::generateMesh(int iCube, double isoValue, int)
  {
    if (!m_molecule->cube(iCube))
      return;

    Cube *cube = m_molecule->cube(iCube);
    m_mesh1 = m_molecule->addMesh();
    m_mesh1->setName(cube->name());
    m_mesh1->setIsoValue(isoValue);
    m_mesh1->setCube(cube->id());
    m_mesh2 = m_molecule->addMesh();
    m_mesh2->setName(cube->name());
    m_mesh2->setIsoValue(-isoValue);
    m_mesh2->setCube(cube->id());
    m_mesh1->setOtherMesh(m_mesh2->id());
    m_mesh2->setOtherMesh(m_mesh1->id());
    if (!m_meshGen1) {
      m_meshGen1 = new MeshGenerator;
      connect(m_meshGen1, SIGNAL(finished()), this, SLOT(meshGenerated()));
    }
    if (!m_meshGen2) {
      m_meshGen2 = new MeshGenerator;
      connect(m_meshGen2, SIGNAL(finished()), this, SLOT(meshGenerated()));
    }
    m_meshGen1->initialize(cube, m_mesh1, isoValue);
    m_meshGen1->start();
    m_meshGen2->initialize(cube, m_mesh2, -isoValue, true);
    m_meshGen2->start();
  }

  void OrbitalExtension::meshGenerated()
  {
    Engine *engine = m_orbitalDialog->currentEngine();
    if (engine) {
      QSettings settings;
      engine->writeSettings(settings);
      settings.setValue("mesh1Id", static_cast<int>(m_mesh1->id()));
      settings.setValue("mesh2Id", static_cast<int>(m_mesh2->id()));
      // If there is a color by and it is 1 then do ESP estimation
      if (m_orbitalDialog->colorBy() == 1) {
        qDebug() << "Calculating approximate ESP mapping...";
        calculateESP(m_mesh1);
        calculateESP(m_mesh2);
        settings.setValue("colorMode", 1);
      }
      else
        settings.setValue("colorMode", 0);
      engine->readSettings(settings);
      engine->setEnabled(true);
      m_molecule->update();
    }
  }

  void OrbitalExtension::calculateVdWCube()
  {
    if (!m_VdWsurface)
      m_VdWsurface = new VdWSurface;

    // Only do the calculation if there is a molecule and it has some atoms
    if (m_molecule) {
      if (m_molecule->numAtoms())
        m_VdWsurface->setAtoms(m_molecule);
      else
        return;
    }
    else
      return;

    qDebug() << "Calculating VdW cube...";
    double stepSize = m_orbitalDialog->stepSize();
    Vector3d origin = m_orbitalDialog->origin();
    Vector3i steps = m_orbitalDialog->steps();
    Cube *cube = m_molecule->addCube();
    cube->setName(QString(tr("VdW Cube")));
    cube->setLimits(origin, steps, stepSize);

    m_VdWsurface->calculateCube(cube);

    // Set up a progress dialog
    if (!m_progress) {
      m_progress = new QProgressDialog(m_orbitalDialog);
      m_progress->setCancelButtonText(tr("Abort Calculation"));
      m_progress->setWindowModality(Qt::NonModal);
    }

    // Set up the progress bar
    m_progress->setWindowTitle(tr("Calculating VdW Cube"));
    m_progress->setRange(m_VdWsurface->watcher().progressMinimum(),
                         m_VdWsurface->watcher().progressMinimum());
    m_progress->setValue(m_VdWsurface->watcher().progressValue());
    m_progress->show();

    connect(&m_VdWsurface->watcher(), SIGNAL(progressValueChanged(int)),
            m_progress, SLOT(setValue(int)));
    connect(&m_VdWsurface->watcher(), SIGNAL(progressRangeChanged(int, int)),
            m_progress, SLOT(setRange(int, int)));
    connect(m_progress, SIGNAL(canceled()),
            this, SLOT(calculateVdWCanceled()));
    connect(&m_VdWsurface->watcher(), SIGNAL(finished()),
            this, SLOT(calculateVdWDone()));
  }

  void OrbitalExtension::calculateVdWDone()
  {
    disconnect(&m_VdWsurface->watcher(), 0, m_progress, 0);
    disconnect(m_progress, 0, this, 0);
  }

  void OrbitalExtension::calculateVdWCanceled()
  {
    disconnect(&m_VdWsurface->watcher(), 0, m_progress, 0);
    disconnect(m_progress, 0, this, 0);
    m_VdWsurface->watcher().cancel();
    qDebug() << "Canceled...";
  }

  void OrbitalExtension::generateVdWMesh(int iCube, double isoValue)
  {
    qDebug() << "Calculate VdW Mesh called" << isoValue << iCube;
    if (!m_molecule->cube(iCube))
      return;

    Cube *cube = m_molecule->cube(iCube);
    m_mesh1 = m_molecule->addMesh();
    m_mesh1->setName(cube->name());
    m_mesh1->setIsoValue(isoValue);
    m_mesh1->setCube(cube->id());

    if (!m_meshGen1) {
      m_meshGen1 = new MeshGenerator;
      connect(m_meshGen1, SIGNAL(finished()), this, SLOT(VdWMeshGenerated()));
    }
    else {
      disconnect(m_meshGen1, 0, this, 0);
      connect(m_meshGen1, SIGNAL(finished()), this, SLOT(VdWMeshGenerated()));
    }
    m_meshGen1->initialize(cube, m_mesh1, isoValue,
                           !cube->property("electronDensity").isValid());
    m_meshGen1->start();
    qDebug() << "Calculate Mesh called" << isoValue;
  }

  void OrbitalExtension::VdWMeshGenerated()
  {
    Engine *engine = m_orbitalDialog->currentEngine();
    if (engine) {
      QSettings settings;
      engine->writeSettings(settings);
      // If there is a color by and it is 1 then do ESP estimation
      if (m_orbitalDialog->colorBy() == 1) {
        qDebug() << "Calculating approximate ESP mapping...";
        calculateESP(m_mesh1);
        settings.setValue("colorMode", 1);
      }
      else
        settings.setValue("colorMode", 0);

      settings.setValue("meshId", static_cast<int>(m_mesh1->id()));
      engine->readSettings(settings);
      engine->setEnabled(true);
      m_molecule->update();
    }
    else {
      qDebug() << "Engine is null - no engines of this type loaded.";
    }
  }

} // End namespace Avogadro

Q_EXPORT_PLUGIN2(orbitalextension, Avogadro::OrbitalExtensionFactory)

