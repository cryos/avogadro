/**********************************************************************
  OrbitalExtension - Extension for visualizing molecular orbitals

  Copyright (C) 2010 David C. Lonie
  Copyright (C) 2009-2011 Marcus D. Hanwell

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

#include <openqube/basissetloader.h>
#include <openqube/basisset.h>
#include <openqube/gaussianset.h>
#include <openqube/gamessukout.h>
#include <openqube/gamessus.h>
#include <openqube/cube.h>

#include <avogadro/molecule.h>
#include <avogadro/atom.h>
#include <avogadro/cube.h>
#include <avogadro/mesh.h>
#include <avogadro/meshgenerator.h>
#include <avogadro/engine.h>
#include <avogadro/glwidget.h>

#include <QDir>
#include <QFileInfo>
#include <QMessageBox>

using OpenQube::BasisSet;
using OpenQube::GaussianSet;
using OpenQube::GamessukOut;
using OpenQube::GAMESSUSOutput;

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
    m_molecule(0),
    m_qube(0)
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
//      m_dock = new OrbitalDock( tr("Orbitals"),
//                                qobject_cast<QWidget *>(parent()) );
      m_dock = new QDockWidget( tr("Orbitals"));
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

    if (!m_basis || m_basis->numElectrons() == 0) {
        if (m_dock) {
          m_widget->setEnabled(false);
          QList<Orbital> list;
          list.clear();
          m_widget->fillTable(list);

          if (m_dock->toggleViewAction()->isChecked())
            m_dock->toggleViewAction()->activate(QAction::Trigger);
        }
      return; // no electrons, no orbitals, no orbital widget
    }
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

      // energies and symmetries
      //TODO: Alpha / Beta orbitals
      QList<QVariant> alphaEnergies;
      QVariant property = molecule->property("alphaOrbitalEnergies");
      if (property.isValid())
        alphaEnergies = property.toList();
      QStringList alphaSymmetries;
      property = molecule->property("alphaOrbitalSymmetries");
      if (property.isValid())
        alphaSymmetries = property.toStringList();

      for (size_t i = 0; i < m_basis->numMOs(); i++) {
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
        // Get the energy from the molecule property list, if available
        if (static_cast<size_t>(alphaEnergies.size()) > i)
          orb.energy = alphaEnergies[i].toDouble();
        else
          orb.energy = 0.0;
        // symmetries (if available)
        if (static_cast<size_t>(alphaSymmetries.size()) > i)
          orb.symmetry = alphaSymmetries[i];
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

    qDebug() << "PreCalculate" ;

    precalculateOrbitals();

    // FIXME: Add this feature back in, refactor ideally.
#if 0
    // Load Properties for QTAIM calculation
    if(m_basis)
    {

      // QTAIM is only implemented for Cartesian Gaussians.
      // 6D, 10F, 15G, etc...
      QString basisSetType = m_basis->basisSetType();
      QVariant basisSetTypeVariant(basisSetType);
      m_molecule->setProperty("QTAIMBasisSetType",basisSetTypeVariant);

      if( basisSetType == "Cartesian GTO")
      {

        qDebug() << "QTAIM: Expand Cartesian GTOs into Primitives";
        m_basis->expandIntoPrimitives();

        QString fileNameString(m_molecule->fileName());
        QVariant fileNameVariant(fileNameString);
        m_molecule->setProperty("QTAIMFileName",fileNameVariant);
        QString commentString("Wavefunction from Surfaces extension");
        QVariant commentVariant(commentString);
        m_molecule->setProperty("QTAIMComment",commentVariant);

        // only keep the occupied orbitals
        unsigned int totalNumberOfMolecularOrbitals( static_cast<qint64>(m_basis->numMOs()) );
        qint64 numberOfMolecularOrbitals=-1;
        for( unsigned int i=0 ; i < totalNumberOfMolecularOrbitals ; ++i)
        {
          if( m_basis->HOMO(i) )
            numberOfMolecularOrbitals=static_cast<qint64>(i) + 1;
        }
        QVariant numberOfMolecularOrbitalsVariant(numberOfMolecularOrbitals);
        m_molecule->setProperty("QTAIMNumberOfMolecularOrbitals",numberOfMolecularOrbitalsVariant);

        qint64 numberOfNuclei( static_cast<qint64>(m_molecule->numAtoms()) );
        QVariant numberOfNucleiVariant(numberOfNuclei);
        m_molecule->setProperty("QTAIMNumberOfNuclei",numberOfNucleiVariant);

        // Nuclear Coordinates
        QList<qreal> xcoordList;
        QList<qreal> ycoordList;
        QList<qreal> zcoordList;
        QList<qint64> nucZList;
        for( qint64 i=0 ; i < numberOfNuclei ; ++i )
        {
          static const double BOHR_TO_ANGSTROM = 0.529177249;
          static const double ANGSTROM_TO_BOHR = 1.0 / 0.529177249;
          Atom *atom=m_molecule->atoms().at(i);
          const Eigen::Vector3d pos=*(atom->pos());
          xcoordList.append(pos(0) * ANGSTROM_TO_BOHR );
          ycoordList.append(pos(1) * ANGSTROM_TO_BOHR );
          zcoordList.append(pos(2) * ANGSTROM_TO_BOHR );
          nucZList.append(atom->atomicNumber());
        }

        QVariantList xcoordVariantList;
        QVariantList ycoordVariantList;
        QVariantList zcoordVariantList;
        QVariantList nucZVariantList;
        for( qint64 i=0 ; i < xcoordList.length() ; ++i )
        {
          xcoordVariantList.append( xcoordList.at(i) );
          ycoordVariantList.append( ycoordList.at(i) );
          zcoordVariantList.append( zcoordList.at(i) );
          nucZVariantList.append( nucZList.at(i) );
        }
        m_molecule->setProperty("QTAIMXNuclearCoordinates",xcoordVariantList);
        m_molecule->setProperty("QTAIMYNuclearCoordinates",ycoordVariantList);
        m_molecule->setProperty("QTAIMZNuclearCoordinates",zcoordVariantList);
        m_molecule->setProperty("QTAIMNuclearCharges",nucZVariantList);

        // Primitive Centers
        QList<qreal> X0List( m_basis->X0List() );
        QVariantList X0VariantList;
        for( qint64 i=0 ; i < X0List.length() ; ++i )
        {
          X0VariantList.append( X0List.at(i) );
        }
        m_molecule->setProperty("QTAIMXGaussianPrimitiveCenterCoordinates",X0VariantList);

        QList<qreal> Y0List( m_basis->Y0List() );
        QVariantList Y0VariantList;
        for( qint64 i=0 ; i < Y0List.length() ; ++i )
        {
          Y0VariantList.append( Y0List.at(i) );
        }
        m_molecule->setProperty("QTAIMYGaussianPrimitiveCenterCoordinates",Y0VariantList);

        QList<qreal> Z0List( m_basis->Z0List() );
        QVariantList Z0VariantList;
        for( qint64 i=0 ; i < Z0List.length() ; ++i )
        {
          Z0VariantList.append( Z0List.at(i) );
        }
        m_molecule->setProperty("QTAIMZGaussianPrimitiveCenterCoordinates",Z0VariantList);

        // Angular Momenta
        QList<qint64> xamomList( m_basis->xamomList() );
        QVariantList xamomVariantList;
        for( qint64 i=0 ; i < xamomList.length() ; ++i )
        {
          xamomVariantList.append( xamomList.at(i) );
        }
        m_molecule->setProperty("QTAIMXGaussianPrimitiveAngularMomenta",xamomVariantList);

        QList<qint64> yamomList( m_basis->yamomList() );
        QVariantList yamomVariantList;
        for( qint64 i=0 ; i < yamomList.length() ; ++i )
        {
          yamomVariantList.append( yamomList.at(i) );
        }
        m_molecule->setProperty("QTAIMYGaussianPrimitiveAngularMomenta",yamomVariantList);

        QList<qint64> zamomList( m_basis->zamomList() );
        QVariantList zamomVariantList;
        for( qint64 i=0 ; i < zamomList.length() ; ++i )
        {
          zamomVariantList.append( zamomList.at(i) );
        }
        m_molecule->setProperty("QTAIMZGaussianPrimitiveAngularMomenta",zamomVariantList);


        QList<qreal> alphaList( m_basis->alphaList() );
        QVariantList alphaVariantList;
        for( qint64 i=0 ; i < alphaList.length() ; ++i )
        {
          alphaVariantList.append( alphaList.at(i) );
        }
        m_molecule->setProperty("QTAIMGaussianPrimitiveExponentCoefficients",alphaVariantList);

        // Orbital Eigenvalues
        QList<qreal> orbeList( m_basis->orbeList() );
        QVariantList orbeVariantList;
        for( qint64 i=0 ; i < numberOfMolecularOrbitals /* orbeList.length() */ ; ++i )
        {
          // orbeVariantList.append( orbeList.at(i) );
          qreal zero=0.0;
          orbeVariantList.append( zero );
        }
        m_molecule->setProperty("QTAIMMolecularOrbitalEigenvalues",orbeVariantList);


        // Occupation Numbers (remember only non-zero)
        QList<qreal> occnoList( m_basis->occnoList() );
        QVariantList occnoVariantList;
        for( qint64 i=0 ; i < numberOfMolecularOrbitals /* occnoList.length() */ ; ++i )
        {
//          occnoVariantList.append( occnoList.at(i) );
          qreal two=2.0;
          occnoVariantList.append( two );
        }
        m_molecule->setProperty("QTAIMMolecularOrbitalOccupationNumbers",occnoVariantList);

        // Primitive Coefficients (remember only non-zero)

        qint64 numberOfGaussianPrimitives( static_cast<qint64>( alphaList.length() ) );
        QVariant numberOfGaussianPrimitivesVariant(numberOfGaussianPrimitives);
        m_molecule->setProperty("QTAIMNumberOfGaussianPrimitives",numberOfGaussianPrimitivesVariant);

        QList<qreal> coefList( m_basis->coefList() );
        QVariantList coefVariantList;
        for( qint64 m=0 ; m < numberOfMolecularOrbitals /* coefList.length() */ ; ++m )
        {
          for( qint64 p=0 ; p < numberOfGaussianPrimitives ; ++p )
          {
            coefVariantList.append( coefList.at( m*numberOfGaussianPrimitives + p ) );
          }
        }
        m_molecule->setProperty("QTAIMMolecularOrbitalCoefficients",coefVariantList);

        // Zero the Total Energy for now
        qreal zero=0.0;
        m_molecule->setProperty("QTAIMTotalEnergy", zero );

        // Set the Virial Ratio (-V/T) to 2 for now
        qreal two=2.0;
        m_molecule->setProperty("QTAIMVirialRatio", two );

      }

    }
#endif


  }

  void OrbitalExtension::calculateOrbitalFromWidget(unsigned int orbital,
                                                    double resolution)
  {
    addCalculationToQueue(orbital, resolution, m_widget->isovalue(), m_widget->boxPadding(), 0);
    checkQueue();
  }


  void OrbitalExtension::precalculateOrbitals()
  {
    if (m_basis) {
      // Determine HOMO
      unsigned int homo = ceil( m_basis->numElectrons() / 2.0 );

      qDebug() << " num electrons " << m_basis->numElectrons();

      // Initialize prioritizer at HOMO's index
      int priority = homo;

      // Loop through all MOs, submitting calculations with increasing
      // priority until HOMO is reached, submit both HOMO and LUMO at
      // priority=1, then start increasing for orbitals above LUMO.
      // E.g,
      // .... HOMO-2 HOMO-1 HOMO LUMO LUMO+1 LUMO+2 ... << orbitals
      // ....   3      2     1    1     2      3    ... << priorities

      // Determine range of precalculated orbitals
      int startIndex = (m_widget->precalcLimit())
          ? homo - (m_widget->precalcRange()/2)
          : 0;
      if (startIndex < 0) {
        startIndex = 0;
      }
      int endIndex =  (m_widget->precalcLimit())
          ? homo + (m_widget->precalcRange()/2) - 1
          : m_basis->numMOs();
      if (endIndex > static_cast<int>(m_basis->numMOs() - 1)) {
        endIndex = m_basis->numMOs() - 1;
      }

      for (int i = startIndex; i <= endIndex; i++) {
        addCalculationToQueue(i+1,  // orbital
                              OrbitalWidget::OrbitalQualityToDouble(m_widget->defaultQuality()),
                              m_widget->isovalue(),
                              m_widget->boxPadding(),
                              priority);

        // Update priority. Stays the same when i = homo.
        if ( i + 1 < static_cast<int>(homo) ) priority--;
        else if ( i + 1 > static_cast<int>(homo)) priority++;
      }
    }
    checkQueue();
  }

  void OrbitalExtension::addCalculationToQueue(unsigned int orbital,
                                               double resolution,
                                               double isovalue,
                                               double boxPadding,
                                               unsigned int priority)
  {
    // Create new queue entry
    calcInfo newCalc;
    newCalc.orbital = orbital;
    newCalc.resolution = resolution;
    newCalc.isovalue = isovalue;
    newCalc.boxPadding = boxPadding;
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
          cI->resolution == info->resolution &&
          cI->boxPadding == info->boxPadding) {
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
    cube->setLimits(m_molecule, info->resolution, info->boxPadding);

    if (m_qube) {
      delete m_qube;
      m_qube = 0;
    }

    m_qube = new OpenQube::Cube;
    m_qube->setLimits(cube->min(), cube->max(), cube->dimensions());

    m_basis->calculateCubeMO(m_qube, info->orbital);
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

    // Convert the cube data
    if (m_qube) {
      info->cube->setData(*m_qube->data());
      delete m_qube;
      m_qube = 0;
    }

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
          cI->isovalue == info->isovalue &&
          cI->boxPadding == info->boxPadding) {
        info->posMesh = cI->posMesh;
        qDebug() << "Reusing posMesh from calculation " << i << ":\n"
                 << "\tOrbital " << cI->orbital << "\n"
                 << "\tResolution " << cI->resolution << "\n"
                 << "\tIsovalue " << cI->isovalue << "\n"
                 << "\tBoxpadding " << cI->boxPadding;
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
          cI->isovalue == info->isovalue &&
          cI->boxPadding == info->boxPadding) {
        info->negMesh = cI->negMesh;
        qDebug() << "Reusing negMesh from calculation " << i << ":\n"
                 << "\tOrbital " << cI->orbital << "\n"
                 << "\tResolution " << cI->resolution << "\n"
                 << "\tIsovalue " << cI->isovalue << "\n"
                 << "\tBoxpadding " << cI->boxPadding;
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
    if (!engine) {
      qDebug() << "Engine is null - no engines of this type loaded.";
      return; // prevent a crash if the surface engine isn't loaded
    }

    engine->setMolecule(m_molecule); // prevent a crash after switching file

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
      qDebug() << "Finished queue.";
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

    // Check to see if this molecule has been tagged with a file format
    QVariant fileFormat = m_molecule->property("File Format");
    if (fileFormat.isValid()) {
      QString format = fileFormat.toString();
      if (format == QLatin1String("gamout")) {
        qDebug() << " deduced from format ";
        if (m_basis) {
          delete m_basis;
          m_basis = 0;
        }
        GaussianSet *gaussian = new GaussianSet;
        GAMESSUSOutput gamout(m_molecule->fileName(), gaussian);
        m_basis = gaussian;
        return true;
      }
      else if (format == QLatin1String("gukout")) {
        qDebug() << " deduced from format ";
        if (m_basis) {
          delete m_basis;
          m_basis = 0;
        }
        GaussianSet *gaussian = new GaussianSet;
        GamessukOut gukout(m_molecule->fileName(), gaussian);
        m_basis = gaussian;
        return true;
      }
    }

    // Everything looks good, a new basis set needs to be loaded
    // Check for files in this directory -- first the file itself
    // and then any other similar file
    if (m_basis) {
      delete m_basis;
      m_basis = 0;
    }
    // Set up the MOs along with the electron density maps
    QString basisFileName =
        OpenQube::BasisSetLoader::MatchBasisSet(m_molecule->fileName());
    if (basisFileName.isEmpty())
    {
      qDebug() << "No matching basis set file found: " <<  m_molecule->fileName();
      return false;
    }
    else
    {
      m_basis = OpenQube::BasisSetLoader::LoadBasisSet(basisFileName);
      if (m_basis)
        return true;
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
