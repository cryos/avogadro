/**********************************************************************
  SurfaceExtension - Extension for generating cubes and meshes

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

#include "surfaceextension.h"

#include "gaussianset.h"
#include "slaterset.h"
#include "gamessukout.h"
#include "gaussianfchk.h"
#include "molpro.h"
#include "mopacaux.h"
#include "molden.h"
#include "vdwsurface.h"
#include "surfacedialog.h"

#include <vector>
#include <avogadro/toolgroup.h>
#include <avogadro/molecule.h>
#include <avogadro/atom.h>
#include <avogadro/cube.h>
#include <avogadro/mesh.h>
#include <avogadro/color3f.h>
#include <avogadro/meshgenerator.h>
#include <avogadro/engine.h>
#include <avogadro/neighborlist.h>
#include <avogadro/glwidget.h>

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
  SurfaceExtension::SurfaceExtension(QObject* parent) : Extension(parent),
    m_glwidget(0), m_surfaceDialog(0), m_molecule(0), m_gaussian(0), m_slater(0),
    m_progress(0), m_mesh1(0), m_mesh2(0), m_meshGen1(0), m_meshGen2(0),
    m_VdWsurface(0)
  {
    QAction* action = new QAction(this);
    action->setText(tr("Create Surfaces..."));
    m_actions.append(action);
  }

  SurfaceExtension::~SurfaceExtension()
  {
    delete m_gaussian;
    m_gaussian = 0;
    delete m_slater;
    m_slater = 0;
    delete m_meshGen1;
    m_meshGen1 = 0;
    delete m_meshGen2;
    m_meshGen2 = 0;
    delete m_VdWsurface;
    m_VdWsurface = 0;
  }

  QList<QAction *> SurfaceExtension::actions() const
  {
    return m_actions;
  }

  QString SurfaceExtension::menuPath(QAction*) const
  {
    return tr("E&xtensions");
  }

  QUndoCommand* SurfaceExtension::performAction(QAction *, GLWidget *widget)
  {
    m_glwidget = widget;
    if (!m_surfaceDialog) {
      m_surfaceDialog = new SurfaceDialog(qobject_cast<QWidget *>(parent()));
      m_surfaceDialog->setGLWidget(widget);
      m_surfaceDialog->setMolecule(m_molecule);
      connect(m_surfaceDialog, SIGNAL(calculate()), this, SLOT(calculate()));
      loadBasis();
      m_surfaceDialog->show();
    }
    else {
      m_surfaceDialog->setGLWidget(widget);
      loadBasis();
      m_surfaceDialog->show();
    }

    return 0;
  }

  void SurfaceExtension::setMolecule(Molecule *molecule)
  {
    m_molecule = molecule;

    // Stuff we manage that will not be valid any longer
    delete m_slater;
    m_slater = 0;
    delete m_gaussian;
    m_gaussian = 0;
    delete m_VdWsurface;
    m_VdWsurface = 0;
    m_loadedFileName = QString();
    m_cubes.clear();
    m_cubes << FALSE_ID << FALSE_ID;
    m_moCubes.clear();

    // This will no longer be valid if the molecule has changed - clear them
    m_mesh1 = 0;
    m_mesh2 = 0;
    m_cube = 0;
    m_cubeColor = 0;
    m_calculationPhase = -1;

    // Update the dialog
    if (m_surfaceDialog) {
      m_surfaceDialog->setMolecule(molecule);
      // If the dialog is visible, then try loading the new basis too
      if (m_surfaceDialog->isVisible())
        loadBasis();
    }
  }

  bool SurfaceExtension::loadBasis()
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
        if (m_gaussian) {
          delete m_gaussian;
          m_gaussian = 0;
        }
        m_gaussian = new GaussianSet;
        GaussianFchk fchk(fullFileName, m_gaussian);

        // Set up the MOs along with the electron density maps
        m_cubes << FALSE_ID;
        m_surfaceDialog->setMOs(m_gaussian->numMOs());
        m_moCubes.resize(m_gaussian->numMOs());
        m_moCubes.fill(FALSE_ID);
        for (unsigned int i = 0; i < m_gaussian->numMOs(); ++i) {
          if (m_gaussian->HOMO(i)) m_surfaceDialog->setHOMO(i);
          else if (m_gaussian->LUMO(i)) m_surfaceDialog->setLUMO(i);
        }
        return true;
      }
      else if (completeSuffix.contains("gukout", Qt::CaseInsensitive)) {
        if (m_slater) {
          delete m_slater;
          m_slater = 0;
        }
        if (m_gaussian) {
          delete m_gaussian;
          m_gaussian = 0;
        }
        m_gaussian = new GaussianSet;
        GamessukOut gukout(fullFileName, m_gaussian);

        // Set up the MOs along with the electron density maps
        m_cubes << FALSE_ID;
        m_surfaceDialog->setMOs(m_gaussian->numMOs());
        m_moCubes.resize(m_gaussian->numMOs());
        m_moCubes.fill(FALSE_ID);
        for (int i = 0; i < m_gaussian->numMOs(); ++i) {
          if (m_gaussian->HOMO(i)) m_surfaceDialog->setHOMO(i);
          else if (m_gaussian->LUMO(i)) m_surfaceDialog->setLUMO(i);
        }
        return true;
      }
      else if (completeSuffix.contains("aux", Qt::CaseInsensitive)) {
        if (m_slater) {
          delete m_slater;
          m_slater = 0;
        }
        if (m_gaussian) {
          delete m_gaussian;
          m_gaussian = 0;
        }
        m_slater = new SlaterSet;
        MopacAux aux(fullFileName, m_slater);

        // Set up the MOs along with the electron density maps
        m_cubes << FALSE_ID;
        m_surfaceDialog->setMOs(m_slater->numMOs());
        m_moCubes.resize(m_slater->numMOs());
        m_moCubes.fill(FALSE_ID);
        for (unsigned int i = 0; i < m_slater->numMOs(); ++i) {
          if (m_slater->HOMO(i)) m_surfaceDialog->setHOMO(i);
          else if (m_slater->LUMO(i)) m_surfaceDialog->setLUMO(i);
        }
        return true;
      }
      else if (completeSuffix.contains("mpo", Qt::CaseInsensitive)) {
        if (m_slater) {
          delete m_slater;
          m_slater = 0;
        }
        if (m_gaussian) {
          delete m_gaussian;
          m_gaussian = 0;
        }
        m_gaussian = new GaussianSet;
        Molpro mpo(fullFileName, m_gaussian);
        qDebug() << "numMOs: " << m_gaussian->numMOs();

        // Set up the MOs along with the electron density maps
        m_cubes << FALSE_ID;
        m_surfaceDialog->setMOs(m_gaussian->numMOs());
        m_moCubes.resize(m_gaussian->numMOs());
        m_moCubes.fill(FALSE_ID);
        for (int i = 0; i < m_gaussian->numMOs(); ++i) {
          if (m_gaussian->HOMO(i)) m_surfaceDialog->setHOMO(i);
          else if (m_gaussian->LUMO(i)) m_surfaceDialog->setLUMO(i);
        }
        return true;
      }
      else if (completeSuffix.contains("molden", Qt::CaseInsensitive)
          || completeSuffix.contains("mold", Qt::CaseInsensitive)
          || completeSuffix.contains("molf", Qt::CaseInsensitive)) {
        if (m_slater) {
          delete m_slater;
          m_slater = 0;
        }
        if (m_gaussian) {
          delete m_gaussian;
          m_gaussian = 0;
        }
        m_gaussian = new GaussianSet;
        MoldenFile fchk(fullFileName, m_gaussian);

        // Set up the MOs along with the electron density maps
        m_cubes << FALSE_ID;
        m_surfaceDialog->setMOs(m_gaussian->numMOs());
        m_moCubes.resize(m_gaussian->numMOs());
        m_moCubes.fill(FALSE_ID);
        for (int i = 0; i < m_gaussian->numMOs(); ++i) {
          if (m_gaussian->HOMO(i)) m_surfaceDialog->setHOMO(i);
          else if (m_gaussian->LUMO(i)) m_surfaceDialog->setLUMO(i);
        }
        return true;
      }
    }

    // We didn't find an appropriate filetype
    qDebug() << "baseName:" << parentInfo.completeSuffix();
    return false;
  }

  void SurfaceExtension::calculateESP(Mesh *mesh)
  {
    // Calculate the ESP mapped onto the vertices of the Mesh supplied
    if (!m_molecule)
      return;

    // Check to see if molecule has hydrogens
    bool hasHydrogens = false;
    foreach (Atom *atom, m_molecule->atoms())
      if (atom->atomicNumber() == 1) {
        hasHydrogens = true;
        break;
      }

    NeighborList *nbrList = new NeighborList(m_molecule, 7.0, false, 2);

    std::vector<Color3f> colors;
    for(unsigned int i=0; i < mesh->vertices().size(); ++i) {
      const Vector3f *v = mesh->vertex(i);

      double energy = 0.0;

      QList<Atom*> nbrAtoms = nbrList->nbrs(v);
      // Include formal charges when there are hydrogens
      if (hasHydrogens) {
        foreach(Atom *a, nbrAtoms) {
          Vector3f dist = a->pos()->cast<float>() - v->cast<float>();
          energy += (a->formalCharge() + a->partialCharge()) / dist.squaredNorm();
        }
      } else {
        foreach(Atom *a, nbrAtoms) {
          Vector3f dist = a->pos()->cast<float>() - v->cast<float>();
          energy += a->partialCharge() / dist.squaredNorm();
        }
      }

      // Chemistry convention: red = negative, blue = positive
      //
      // Use HSV color model for smooth transitions
      int red_hue = 0;
      int blue_hue = 240;
      int hue = 0; // meaningless if gray (i.e. low saturation)
      int saturation = 0; // 0 = white, 0-40 = grayish, 40-255 colors from hue
      int value = 255; // lightness or brightness (0 = black, 255 = white)

      if (energy < 0.0) {
        hue = red_hue;
        saturation = -255 * 5 * energy;
      } else if (energy > 0.0) {
        hue = blue_hue;
        saturation = 255 * 5 * energy;
      }

      if (saturation > 255)
        saturation = 255;

      QColor qcolor(QColor::fromHsv(hue, saturation, value));
      Color3f color(qcolor.red(), qcolor.green(), qcolor.blue());
      colors.push_back(color);
    }
    mesh->setColors(colors);
  }

  Cube * SurfaceExtension::newCube()
  {
    // This function takes the requested resolution and makes a new cube
    Cube *cube = m_molecule->addCube();
    double step = m_surfaceDialog->stepSize();
    cube->setLimits(m_molecule, step, 2.5);
    return cube;
  }

  void SurfaceExtension::calculateVdW(Cube *cube)
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

    m_VdWsurface->calculateCube(cube);

    // Set up a progress dialog
    if (!m_progress) {
      m_progress = new QProgressDialog(m_surfaceDialog);
      m_progress->setCancelButtonText(tr("Abort Calculation"));
      m_progress->setWindowModality(Qt::NonModal);
    }

    // Set up the progress bar
    m_progress->setWindowTitle(tr("Calculating VdW Cube"));
    m_progress->setRange(m_VdWsurface->watcher().progressMinimum(),
                         m_VdWsurface->watcher().progressMaximum());
    m_progress->setValue(m_VdWsurface->watcher().progressValue());
    m_progress->show();

    connect(&m_VdWsurface->watcher(), SIGNAL(progressValueChanged(int)),
            m_progress, SLOT(setValue(int)));
    connect(&m_VdWsurface->watcher(), SIGNAL(progressRangeChanged(int, int)),
            m_progress, SLOT(setRange(int, int)));
    connect(m_progress, SIGNAL(canceled()),
            this, SLOT(calculateCanceled()));
    connect(&m_VdWsurface->watcher(), SIGNAL(finished()),
            this, SLOT(calculateDone()));
  }

  void SurfaceExtension::calculateMo(Cube *cube, int mo)
  {
    // Slater type orbital
    if (m_slater) {
      m_slater->calculateCubeMO(cube, mo);

      // Set up a progress dialog
      if (!m_progress) {
        m_progress = new QProgressDialog(m_surfaceDialog);
        m_progress->setCancelButtonText(tr("Abort Calculation"));
        m_progress->setWindowModality(Qt::NonModal);
      }

      // Set up the progress bar
      m_progress->setWindowTitle(tr("Calculating MO %L1", "Molecular Orbital").arg(mo));
      m_progress->setRange(m_slater->watcher().progressMinimum(),
                           m_slater->watcher().progressMaximum());
      m_progress->setValue(m_slater->watcher().progressValue());
      m_progress->show();

      // Connect the signals and slots
      connect(&m_slater->watcher(), SIGNAL(progressValueChanged(int)),
              m_progress, SLOT(setValue(int)));
      connect(&m_slater->watcher(), SIGNAL(progressRangeChanged(int, int)),
              m_progress, SLOT(setRange(int, int)));
      connect(m_progress, SIGNAL(canceled()),
              this, SLOT(calculateCanceled()));
      connect(&m_slater->watcher(), SIGNAL(finished()),
              this, SLOT(calculateDone()));
    }
    else if (m_gaussian) {
      m_gaussian->calculateCubeMO(cube, mo);

     // Set up a progress dialog
      if (!m_progress) {
        m_progress = new QProgressDialog(m_surfaceDialog);
        m_progress->setCancelButtonText(tr("Abort Calculation"));
      }

      // Set up the progress bar
      m_progress->setWindowTitle(tr("Calculating MO %L1", "Molecular Orbital").arg(mo));
      m_progress->setRange(m_gaussian->watcher().progressMinimum(),
                           m_gaussian->watcher().progressMaximum());
      m_progress->setValue(m_gaussian->watcher().progressValue());
      m_progress->show();

      // Connect signals and slots
      connect(&m_gaussian->watcher(), SIGNAL(progressValueChanged(int)),
              m_progress, SLOT(setValue(int)));
      connect(&m_gaussian->watcher(), SIGNAL(progressRangeChanged(int, int)),
              m_progress, SLOT(setRange(int, int)));
      connect(m_progress, SIGNAL(canceled()),
              this, SLOT(calculateCanceled()));
      connect(m_gaussian, SIGNAL(finished()),
              this, SLOT(calculateDone()));
    }
    m_surfaceDialog->enableCalculation(false);
  }

  void SurfaceExtension::calculateElectronDensity(Cube *cube)
  {
    if (!m_gaussian && !m_slater)
      return;

    if (m_slater) {
      m_slater->calculateCubeDensity(cube);

      // Set up a progress dialog
      if (!m_progress) {
        m_progress = new QProgressDialog(m_surfaceDialog);
        m_progress->setCancelButtonText(tr("Abort Calculation"));
        m_progress->setWindowModality(Qt::NonModal);
      }

      // Set up the progress bar
      m_progress->setWindowTitle(tr("Calculating Electron Density"));
      m_progress->setRange(m_slater->watcher().progressMinimum(),
                           m_slater->watcher().progressMaximum());
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
              this, SLOT(calculateDone()));
    }
    else if (m_gaussian) {
      // Gaussian type orbital
      m_gaussian->calculateCubeDensity(cube);

      // Set up a progress dialog
      if (!m_progress) {
        m_progress = new QProgressDialog(m_surfaceDialog);
        m_progress->setCancelButtonText(tr("Abort Calculation"));
        m_progress->setWindowModality(Qt::NonModal);
      }

      // Set up the progress bar
      m_progress->setWindowTitle(tr("Calculating Electron Density"));
      m_progress->setRange(m_gaussian->watcher().progressMinimum(),
                           m_gaussian->watcher().progressMinimum());
      m_progress->setValue(m_gaussian->watcher().progressValue());
      m_progress->show();

      // Connect the signals and slots
      connect(&m_gaussian->watcher(), SIGNAL(progressValueChanged(int)),
              m_progress, SLOT(setValue(int)));
      connect(&m_gaussian->watcher(), SIGNAL(progressRangeChanged(int, int)),
              m_progress, SLOT(setRange(int, int)));
      connect(m_progress, SIGNAL(canceled()),
              this, SLOT(calculateCanceled()));
      connect(m_gaussian, SIGNAL(finished()),
              this, SLOT(calculateDone()));
    }
    m_surfaceDialog->enableCalculation(false);
  }

  void SurfaceExtension::calculateMesh(Cube *cube, double isoValue)
  {
    qDebug() << "calculateMesh called" << isoValue << cube;

    if (!cube->lock()->tryLockForRead()) {
      qDebug() << "SurfaceExtension::calculateMesh could not obtain read lock."
          << cube->id() << cube->name() << isoValue;
    }
    else
      cube->lock()->unlock();

    m_mesh1 = m_molecule->addMesh();
    m_mesh1->setName(cube->name());
    m_mesh1->setIsoValue(isoValue);
    m_mesh1->setCube(cube->id());

    if (!m_meshGen1) {
      m_meshGen1 = new MeshGenerator;
      connect(m_meshGen1, SIGNAL(finished()), this, SLOT(calculateDone()));
    }
    else {
      disconnect(m_meshGen1, 0, this, 0);
      delete m_meshGen1;
      m_meshGen1 = new MeshGenerator;
      connect(m_meshGen1, SIGNAL(finished()), this, SLOT(calculateDone()));
    }
    m_meshGen1->initialize(cube, m_mesh1, isoValue,
                           m_surfaceDialog->cubeType() == Cube::VdW);
    m_meshGen1->start();

    // Calculate the negative part of the MO if this is an MO mesh
    if (m_surfaceDialog->cubeType() == Cube::MO ||
        m_surfaceDialog->cubeType() == Cube::FromFile) {
      m_mesh2 = m_molecule->addMesh();
      m_mesh2->setName(cube->name() + " negative");
      m_mesh2->setIsoValue(-isoValue);
      m_mesh2->setCube(cube->id());
      // Add pair information
      m_mesh1->setOtherMesh(m_mesh2->id());
      m_mesh2->setOtherMesh(m_mesh1->id());
      // Now generate the mesh
      if (!m_meshGen2) {
        m_meshGen2 = new MeshGenerator;
        connect(m_meshGen2, SIGNAL(finished()), this, SLOT(calculateDone()));
      }
      else {
        disconnect(m_meshGen2, 0, this, 0);
        delete m_meshGen2;
        m_meshGen2 = new MeshGenerator;
        connect(m_meshGen2, SIGNAL(finished()), this, SLOT(calculateDone()));
      }
      // Reverse the windings for the negative isosurface
      m_meshGen2->initialize(cube, m_mesh2, -isoValue, true);
      m_meshGen2->start();
    }

    qDebug() << "calculateMesh called" << isoValue;
  }

  Cube * SurfaceExtension::startCubeCalculation(Cube::Type type,
                                                int mo, bool &calculateCube)
  {
    switch (type) {
      case Cube::VdW: {
        Cube *cube = m_molecule->cubeById(m_cubes[0]);
        if (!cube) { // We need a new cube
          cube = newCube();
          cube->setName(tr("VdW"));
          cube->setCubeType(Cube::VdW);
          m_cubes[0] = cube->id();
          calculateVdW(cube);
          calculateCube = true;
          return cube;
        }
        // There is a valid cube - check the resolution
        else if (fabs(cube->spacing().x() - m_surfaceDialog->stepSize()) > 0.02) {
          // Resize the cube and recalculate at the desired resolution
          cube->setLimits(m_molecule, m_surfaceDialog->stepSize(), 2.5);
          calculateVdW(cube);
          calculateCube = true;
          return cube;
        }
        else {
          // The cube is valid, the resolution is valid. Return cube
          calculateCube = false;
          return cube;
        }
      }
      case Cube::ESP:
        ;
      case Cube::ElectronDensity: {
        qDebug() << "m_cubes.size() =" << m_cubes.size();
        Cube *cube = m_molecule->cubeById(m_cubes[2]);
        if (!cube) { // We need a new cube
          cube = newCube();
          cube->setName(tr("Electron Density"));
          cube->setCubeType(Cube::ElectronDensity);
          m_cubes[2] = cube->id();
          calculateElectronDensity(cube);
          calculateCube = true;
          return cube;
        }
        // There is a valid cube - check the resolution
        else if (fabs(cube->spacing().x() - m_surfaceDialog->stepSize()) > 0.02) {
          // Resize the cube and recalculate at the desired resolution
          cube->setLimits(m_molecule, m_surfaceDialog->stepSize(), 2.5);
          calculateElectronDensity(cube);
          calculateCube = true;
          return cube;
        }
        else {
          // The cube is valid, the resolution is valid. Return cube
          calculateCube = false;
          return cube;
        }
      }
      case Cube::MO: {
        if ((mo - 1) >= m_moCubes.size())
          m_moCubes.resize(mo - 1);
        // Attempt to retrieve the cube - will be 0 if no cube was calculated
        Cube *cube = m_molecule->cubeById(m_moCubes[mo - 1]);
        if (!cube) { // We need a new cube
          cube = newCube();
          cube->setName(tr("MO %L1", "Molecular Orbital").arg(mo));
          cube->setCubeType(Cube::MO);
          m_moCubes[mo - 1] = cube->id();
          calculateMo(cube, mo);
          calculateCube = true;
          return cube;
        }
        // There is a valid cube - check the resolution
        else if (fabs(cube->spacing().x() - m_surfaceDialog->stepSize()) > 0.02) {
          qDebug() << "Recalculating MO cube, delta ="
              << fabs(cube->spacing().x() - m_surfaceDialog->stepSize());
          // Resize the cube and recalculate at the desired resolution
          cube->setLimits(m_molecule, m_surfaceDialog->stepSize(), 2.5);
          calculateMo(cube, mo);
          calculateCube = true;
          return cube;
        }
        else {
          // The cube is valid, the resolution is valid. Return cube
          calculateCube = false;
          return cube;
        }
      }
      case Cube::FromFile: {
        // If it is a cube from a file, query the dialog for the cube id
        calculateCube = false;
        return m_molecule->cubeById(m_surfaceDialog->cubeFromFile());
      }
      case Cube::None:
      default: // Do nothing
        return 0;
    }
  }

  void SurfaceExtension::calculate()
  {
    qDebug() << "Calculate called!";
    m_calculationPhase = 0;
    m_cube = 0;
    m_cubeColor = 0;
    m_mesh1 = 0;
    m_mesh2 = 0;

    // Now attempt to begin the calculation
    bool calculateCube = false;
    m_cube = startCubeCalculation(m_surfaceDialog->cubeType(),
                                      m_surfaceDialog->moNumber(),
                                      calculateCube);
    if (!calculateCube) {
      // Use the existing cube - calculate the isosurface
      m_calculationPhase = 2;
      calculateMesh(m_cube, m_surfaceDialog->isoValue());
    }
  }

  void SurfaceExtension::calculateDone()
  {
    // Figure out what to do based on the calculation phase
    // 0 = main cube, 1 = color cube (optional) and 2 = mesh calculation
    switch (m_calculationPhase) {
      case 0: { // main cube was calculated - possibly kick off a color cube
        qDebug() << "Calculation phase 0 complete - now to phase 1...";
        m_calculationPhase = 1;
        // Disconnect the signals and slots that we are now finished with
        if (m_surfaceDialog->cubeType() == Cube::MO ||
            m_surfaceDialog->cubeType() == Cube::ElectronDensity) {
          if (m_gaussian)
            disconnect(&m_gaussian->watcher(), 0, this, 0);
          else if (m_slater)
            disconnect(&m_slater->watcher(), 0, this, 0);
        }
        disconnect(m_progress, 0, this, 0);
        // FIXME Skipped for now!
        if (m_surfaceDialog->cubeColorType() != Cube::None) {

        }
      }
      case 1: { // color cube done (if needed) - now calculate the mesh
        m_calculationPhase = 2;
        // Disconnect the signals and slots that we are now finished with
        disconnect(m_progress, 0, this, 0);

        calculateMesh(m_cube, m_surfaceDialog->isoValue());
        return;
      }
      case 2: { // Mesh calculated - now display it in an engine if possible
        if (!m_mesh2)
          m_calculationPhase = -1; // i.e. no calculation in progress any more
        else if (m_mesh1->stable() && m_mesh2 && m_mesh2->stable()) {
          // The MO meshes have both been calculated
          m_calculationPhase = -1;
        }
        else // Still calculating one of the meshes
          return;

        Engine *engine = m_surfaceDialog->currentEngine();
        if (engine) {
          QSettings settings;
          engine->writeSettings(settings);
          // If there is a color by and it is 1 then do ESP estimation
          if (m_surfaceDialog->cubeColorType() == Cube::ESP) {
            qDebug() << "Calculating approximate ESP mapping...";
            calculateESP(m_mesh1);
            if (m_mesh2)
              calculateESP(m_mesh2);
            settings.setValue("colorMode", 1);
          }
          else
            settings.setValue("colorMode", 0);

          settings.setValue("mesh1Id", static_cast<int>(m_mesh1->id()));
          if (m_mesh2)
            settings.setValue("mesh2Id", static_cast<int>(m_mesh2->id()));
          else
            settings.setValue("mesh2Id", qulonglong(FALSE_ID));

          engine->readSettings(settings);
          engine->setEnabled(true);
          // Trigger a repaint with the new mesh
          /// FIXME Should be using m_molecule->update() to trigger a repaint in
          /// all open displays, this currently causes crashes - need to track
          /// down the cause.
          m_glwidget->update();
        }
        else
          qDebug() << "Engine is null - no engines of this type loaded.";

        if (m_calculationPhase == -1)
          m_surfaceDialog->enableCalculation(true);
      }
    }
  }

  void SurfaceExtension::calculateCanceled()
  {
    /// FIXME - implement this code too!
  }

} // End namespace Avogadro

Q_EXPORT_PLUGIN2(surfaceextension, Avogadro::SurfaceExtensionFactory)
