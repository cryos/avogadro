/**********************************************************************
  OrbitalExtension - Extension for generating orbital cubes

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

#include "orbitalextension.h"

#include "gaussianfchk.h"
#include "slaterset.h"
#include "mopacaux.h"

#include <vector>
#include <avogadro/toolgroup.h>
#include <avogadro/molecule.h>
#include <avogadro/atom.h>
#include <avogadro/cube.h>
#include <Eigen/Core>

#include <QProgressDialog>
#include <QCoreApplication>
#include <QFileInfo>
#include <QMessageBox>
#include <QTime>
#include <QDebug>

using namespace std;
using namespace OpenBabel;

namespace Avogadro
{

  using Eigen::Vector3d;
  using Eigen::Vector3i;

  OrbitalExtension::OrbitalExtension(QObject* parent) : Extension(parent),
    m_glwidget(0), m_orbitalDialog(0), m_molecule(0), m_basis(0), m_slater(0),
    m_progress(0), m_timer(0)
  {
    QAction* action = new QAction(this);
    action->setText(tr("Import Molecular Orbitals..."));
    m_actions.append(action);
  }

  OrbitalExtension::~OrbitalExtension()
  {
    if (m_orbitalDialog) {
      delete m_orbitalDialog;
      m_orbitalDialog = 0;
    }
    if (m_basis) {
      delete m_basis;
      m_basis = 0;
    }
  }

  QList<QAction *> OrbitalExtension::actions() const
  {
    return m_actions;
  }

  QString OrbitalExtension::menuPath(QAction*) const
  {
    return tr("&Extensions");
  }

  QUndoCommand* OrbitalExtension::performAction(QAction *, GLWidget *widget)
  {
    m_glwidget = widget;
    if (!m_orbitalDialog)
    {
      m_orbitalDialog = new OrbitalDialog();
      connect(m_orbitalDialog, SIGNAL(calculateMO(int)),
              this, SLOT(calculateMO(int)));
      connect(m_orbitalDialog, SIGNAL(calculateAll()),
              this, SLOT(calculateAll()));
      connect(m_orbitalDialog, SIGNAL(calculateDensity()),
              this, SLOT(calculateDensity()));
      if (loadBasis()) {
        m_orbitalDialog->show();
      }
      else {
        m_orbitalDialog->show();
      }
    }
    else {
      if (loadBasis()) {
        m_orbitalDialog->show();
      }
      else {
        QMessageBox::warning(m_orbitalDialog, tr("File type not supported"),
                             tr("Either no file is loaded, or the loaded file type is not supported. Currently Gaussian checkpoints (.fchk/.fch) are supported."));
      }
    }
    return 0;
  }

  void OrbitalExtension::setMolecule(Molecule *molecule)
  {
    m_molecule = molecule;
  }

  bool OrbitalExtension::loadBasis()
  {
    if (m_molecule->fileName().isEmpty()) {
      return false;
    }
    else if (m_loadedFileName == m_molecule->fileName()) {
      return true;
    }

    // Everything looks good, a new basis set needs to be loaded
    QFileInfo info(m_molecule->fileName());
    if (info.completeSuffix() == "fchk" || info.completeSuffix() == "fch") {
      if (m_basis)
        delete m_basis;
      m_basis = new BasisSet;
      GaussianFchk fchk(m_molecule->fileName(), m_basis);

      m_orbitalDialog->setMOs(m_basis->numMOs());
      for (int i = 0; i < m_basis->numMOs(); ++i) {
        if (m_basis->HOMO(i)) m_orbitalDialog->setHOMO(i);
        else if (m_basis->LUMO(i)) m_orbitalDialog->setLUMO(i);
      }

      // Now to set the default cube...
      Cube cube;
      double step = 0.18;
      cube.setLimits(m_molecule, step, 2.5);
      Vector3d min = cube.min();// / BOHR_TO_ANGSTROM;
      Vector3i dim = cube.dimensions();
      // Set these values on the form - they can then be altered by the user
      m_orbitalDialog->setCube(min, dim.x(), dim.y(), dim.z(), step);
      return true;
    }
    else if (info.completeSuffix() == "mopout" || info.completeSuffix() == "out") {
      if (m_basis)
        delete m_basis;
      m_slater = new SlaterSet;
      MopacAux aux(info.absolutePath() + "/" + info.baseName() + ".aux", m_slater);

      // Set the number of MOs
      m_orbitalDialog->setMOs(m_slater->numMOs());
      for (unsigned int i = 0; i < m_slater->numMOs(); ++i) {
        if (m_slater->HOMO(i)) m_orbitalDialog->setHOMO(i);
        else if (m_slater->LUMO(i)) m_orbitalDialog->setLUMO(i);
      }

      Cube cube;
      double step = 0.18;
      cube.setLimits(m_molecule, step, 2.5);
      Vector3d min = cube.min();// / BOHR_TO_ANGSTROM;
      Vector3i dim = cube.dimensions();
      // Set these values on the form - they can then be altered by the user
      m_orbitalDialog->setCube(min, dim.x(), dim.y(), dim.z(), step);

      return true;
    }
    // If we get here it is a basis set we cannot load yet
    else {
      qDebug() << "baseName:" << info.completeSuffix();
      m_orbitalDialog->setCurrentTab(1);
      return false;
    }
  }

  void OrbitalExtension::calculateMO(int mo, const Vector3d &origin,
                                     const Vector3i &steps, double stepSize)
  {
    const double BOHR_TO_ANGSTROM = 0.529177249;
    // Slater type orbital
    if (m_slater) {
      // We have a slater type orbital....
      qDebug() << "Adding a slater type orbital...." << mo;
      Cube *cube = m_molecule->newCube();
      cube->setName(QString(tr("MO ") + QString::number(mo)));
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
      m_progress->setWindowTitle(tr("Calculating MO ") + QString::number(mo));
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
      Cube *cube = m_molecule->newCube();
      cube->setName(QString(tr("MO ") + QString::number(mo)));
      cube->setLimits(origin * BOHR_TO_ANGSTROM, steps,
                      stepSize * BOHR_TO_ANGSTROM);
      if (!m_timer) {
        m_timer = new QTime;
        m_timer->start();
      }
      m_basis->calculateCubeMO2(cube, mo);

     // Set up a progress dialog
      if (!m_progress) {
        m_progress = new QProgressDialog(m_orbitalDialog);
        m_progress->setCancelButtonText(tr("Abort Calculation"));
        m_progress->setWindowModality(Qt::NonModal);
      }

      // Set up the progress bar
      m_progress->setWindowTitle(tr("Calculating MO ") + QString::number(mo));
      m_progress->setRange(m_basis->watcher2().progressMinimum(),
                           m_basis->watcher2().progressMinimum());
      m_progress->setValue(m_basis->watcher2().progressValue());

      // Connect signals and slots
      connect(&m_basis->watcher2(), SIGNAL(progressValueChanged(int)),
              m_progress, SLOT(setValue(int)));
      connect(&m_basis->watcher2(), SIGNAL(progressRangeChanged(int, int)),
              m_progress, SLOT(setRange(int, int)));
      connect(m_progress, SIGNAL(canceled()),
              this, SLOT(calculation2Canceled()));
      connect(&m_basis->watcher2(), SIGNAL(finished()),
              this, SLOT(calculation2Done()));
      m_orbitalDialog->enableCalculation(false);
    }
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
      // We have a slater type orbital....
      Cube *cube = m_molecule->newCube();
      cube->setName(QString(tr("Electron Density")));
      cube->setLimits(origin * BOHR_TO_ANGSTROM, steps,
                      stepSize * BOHR_TO_ANGSTROM);
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
      // FIXME Not implemented yet...
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
      disconnect(&m_basis->watcher2(), SIGNAL(progressValueChanged(int)),
                 m_progress, SLOT(setValue(int)));
      disconnect(&m_basis->watcher2(), SIGNAL(progressRangeChanged(int, int)),
                 m_progress, SLOT(setRange(int, int)));
      disconnect(m_progress, SIGNAL(canceled()),
                 this, SLOT(calculation2Canceled()));
      disconnect(&m_basis->watcher2(), SIGNAL(finished()),
                 this, SLOT(calculation2Done()));

      qDebug() << "Single points calculation done in" << m_timer->elapsed() / 1000.0
               << "seconds";
      delete m_timer;
      m_timer = 0;
      m_molecule->update();
      m_orbitalDialog->enableCalculation(true);
    }
    else if (m_basis->numMOs() == m_currentMO) { // All MOs have been calculated
      disconnect(&m_basis->watcher2(), SIGNAL(progressValueChanged(int)),
                 m_progress, SLOT(setValue(int)));
      disconnect(&m_basis->watcher2(), SIGNAL(progressRangeChanged(int, int)),
                 m_progress, SLOT(setRange(int, int)));
      disconnect(m_progress, SIGNAL(canceled()),
                 this, SLOT(calculation2Canceled()));
      disconnect(&m_basis->watcher2(), SIGNAL(finished()),
                 this, SLOT(calculation2Done()));

      qDebug() << "All cube MOs calculated in" << m_timer->elapsed() / 1000.0
               << "seconds";
      delete m_timer;
      m_timer = 0;
      m_molecule->update();
      m_orbitalDialog->enableCalculation(true);
      m_currentMO = 0;
    }
    else { // More work to do
      disconnect(&m_basis->watcher2(), SIGNAL(progressValueChanged(int)),
                 m_progress, SLOT(setValue(int)));
      disconnect(&m_basis->watcher2(), SIGNAL(progressRangeChanged(int, int)),
                 m_progress, SLOT(setRange(int, int)));
      disconnect(m_progress, SIGNAL(canceled()),
                 this, SLOT(calculation2Canceled()));
      disconnect(&m_basis->watcher2(), SIGNAL(finished()),
                 this, SLOT(calculation2Done()));
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
    disconnect(&m_basis->watcher2(), SIGNAL(progressValueChanged(int)),
               m_progress, SLOT(setValue(int)));
    disconnect(&m_basis->watcher2(), SIGNAL(progressRangeChanged(int, int)),
            m_progress, SLOT(setRange(int, int)));
    connect(m_progress, SIGNAL(canceled()),
            this, SLOT(calculation2Canceled()));
    disconnect(&m_basis->watcher2(), SIGNAL(finished()),
            this, SLOT(calculation2Done()));
    m_basis->watcher2().cancel();
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

} // End namespace Avogadro

#include "orbitalextension.moc"

Q_EXPORT_PLUGIN2(orbitalextension, Avogadro::OrbitalExtensionFactory)
