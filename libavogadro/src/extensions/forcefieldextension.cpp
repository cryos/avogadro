/**********************************************************************
  forcefieldextension.cpp - molecular mechanics force field Plugin for Avogadro

  Copyright (C) 2006 by Donald Ephraim Curtis
  Copyright (C) 2006-2007 by Geoffrey R. Hutchison

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.openmolecules.net/>

  Some code is based on Open Babel
  For more information, see <http://openbabel.sourceforge.net/>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation version 2 of the License.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
 ***********************************************************************/

#include "forcefieldextension.h"
#include <avogadro/primitive.h>
#include <avogadro/color.h>
#include <avogadro/glwidget.h>
#include <avogadro/atom.h>

#include <QProgressDialog>
#include <QWriteLocker>
#include <QMutex>
#include <QMutexLocker>
#include <QAbstractTableModel>
#include <QMessageBox>
#include <QDebug>

using namespace std;
using namespace OpenBabel;

namespace Avogadro
{
  enum ForceFieldExtensionIndex
    {
      OptimizeGeometryIndex = 0,
      CalculateEnergyIndex,
      ConformerSearchIndex,
      SetupForceFieldIndex,
      ConstraintsIndex,
      IgnoreAtomsIndex,
      FixAtomsIndex,
      SeparatorIndex
    };

  ForceFieldExtension::ForceFieldExtension( QObject *parent ) :
    Extension( parent ), m_molecule(0)
  {
    QAction *action;
    // If you change this, see forcefielddialog.cpp, where we need to set the popup menu
    m_forceField = OBForceField::FindForceField( "MMFF94" );
    m_dialog = 0;
    m_conformerDialog = 0;
    m_constraintsDialog = 0;
    m_constraints = 0;

    if ( m_forceField ) { // make sure we can actually find and run it!

      action = new QAction( this );
      action->setText( tr("Optimize Geometry" ));
      action->setData(OptimizeGeometryIndex);
      m_actions.append( action );

      action = new QAction( this );
      action->setText( tr("Setup Force Field..." ));
      action->setData(SetupForceFieldIndex);
      m_actions.append( action );

      action = new QAction( this );
      action->setSeparator(true);
      action->setData(SeparatorIndex);
      m_actions.append( action );

      action = new QAction( this );
      action->setText( tr("Calculate Energy" ));
      action->setData(CalculateEnergyIndex);
      m_actions.append( action );

      action = new QAction( this );
      action->setText( tr("Conformer Search..." ));
      action->setData(ConformerSearchIndex);
      m_actions.append( action );

      action = new QAction( this );
      action->setSeparator(true);
      action->setData(SeparatorIndex);
      m_actions.append( action );

      action = new QAction( this );
      action->setText( tr("Constraints..." ));
      action->setData(ConstraintsIndex);
      m_actions.append( action );

      action = new QAction( this );
      action->setText( tr("Ignore Selection" ));
      action->setData(IgnoreAtomsIndex);
      m_actions.append( action );

      action = new QAction( this );
      action->setText( tr("Fix Selected Atoms" ));
      action->setData(FixAtomsIndex);
      m_actions.append( action );

      // back in the main menu -- separate us from others
      action = new QAction( this );
      action->setSeparator(true);
      action->setData(OptimizeGeometryIndex);
      m_actions.append( action );
    }
    else
      qDebug() << "Unable to set up and use any force fields.";

    OBPlugin::ListAsVector("forcefields", "ids", m_forcefieldList);
  }

  ForceFieldExtension::~ForceFieldExtension()
  {
    delete m_constraints;
    m_constraints = 0;
  }

  QList<QAction *> ForceFieldExtension::actions() const
  {
    return m_actions;
  }

  QString ForceFieldExtension::menuPath(QAction *action) const
  {
    int i = action->data().toInt();

    if (i == OptimizeGeometryIndex)
      return QString();

    return tr("E&xtensions") + '>' + tr("&Molecular Mechanics");
  }

  void ForceFieldExtension::setMolecule(Molecule *molecule)
  {
    m_molecule = molecule;
  }

  QUndoCommand* ForceFieldExtension::performAction( QAction *action, GLWidget *widget)
  {
    ForceFieldCommand *undo = NULL;
    QList<Primitive*> selectedAtoms;
    ostringstream buff;

    if (!m_dialog)
      m_dialog = new ForceFieldDialog(static_cast<QWidget*>(parent()));
    if (!m_constraints)
      m_constraints = new ConstraintsModel;

    m_forceField = OBForceField::FindForceField(m_forcefieldList[m_dialog->forceFieldID()]);
    m_forceField->SetLogFile( &buff );

    int i = action->data().toInt();
    double energy = 0.0;
    QString msg;
    OBMol mol;
    switch ( i ) {
    case SetupForceFieldIndex: // setup force field
      m_dialog->show();
      break;
    case CalculateEnergyIndex: // calculate energy
      if ( !m_forceField )
        break;

      m_forceField->SetLogLevel( OBFF_LOGLVL_HIGH );

      mol = m_molecule->OBMol();
      if ( !m_forceField->Setup( mol, m_constraints->constraints() ) ) {
        QMessageBox::warning( widget, tr( "Avogadro" ),
          tr( "Cannot set up the force field for this molecule." ));
        break;
      }

      energy = m_forceField->Energy();
      msg = QString( tr( "Energy = %1 %2" ))
                     .arg(energy).arg(m_forceField->GetUnit().c_str());
      QMessageBox::information( widget, tr( "Avogadro" ), msg);

      emit message( tr( buff.str().c_str() ) );
      break;
    case ConformerSearchIndex: // conformer search
      if (!m_forceField)
        break;

      m_forceField->SetLogLevel( OBFF_LOGLVL_LOW );

      mol = m_molecule->OBMol();
      if ( !m_forceField->Setup( mol, m_constraints->constraints() ) ) {
        QMessageBox::warning( widget, tr( "Avogadro" ),
          tr( "Cannot set up the force field for this molecule." ));
        break;
      }

      if (!m_conformerDialog)
        m_conformerDialog = new ConformerSearchDialog(static_cast<QWidget*>(parent()));

      m_conformerDialog->setup(m_molecule, m_forceField, m_constraints,
                               0, m_dialog->nSteps(), m_dialog->algorithm(), m_dialog->convergence());
      m_conformerDialog->show();
      break;
    case OptimizeGeometryIndex: // geometry optimization
      if (!m_forceField)
        break;

      m_forceField->SetLogLevel( OBFF_LOGLVL_LOW );

      mol = m_molecule->OBMol();
      if ( !m_forceField->Setup( mol, m_constraints->constraints() ) ) {
        QMessageBox::warning( widget, tr( "Avogadro" ),
          tr( "Cannot set up the force field for this molecule." ));
        break;
      }

      undo = new ForceFieldCommand( m_molecule, m_forceField, m_constraints,
                                    0, m_dialog->nSteps(), m_dialog->algorithm(),
                                    m_dialog->convergence(), 0 );

      connect(undo, SIGNAL(message(QString)), this, SIGNAL(message(QString)));

      undo->setText( QObject::tr( "Geometric Optimization" ) );
      break;
    case ConstraintsIndex: // show constraints dialog
      if (!m_constraintsDialog) {
        m_constraintsDialog = new ConstraintsDialog(static_cast<QWidget*>(parent()));
        m_constraintsDialog->setModel(m_constraints);
      }

      m_constraintsDialog->setMolecule(m_molecule);
      m_constraintsDialog->show();
      break;
    case IgnoreAtomsIndex: // ignore the selected atoms
      selectedAtoms = widget->selectedPrimitives().subList(Primitive::AtomType);

      foreach(Primitive *p, selectedAtoms)
      {
        Atom *atom = static_cast<Atom *>(p);
        m_constraints->addIgnore(atom->index() + 1);
      }

      m_forceField->SetConstraints(m_constraints->constraints());

      break;

    case FixAtomsIndex: // fix the selected atom positions
      selectedAtoms = widget->selectedPrimitives().subList(Primitive::AtomType);

      foreach(Primitive *p, selectedAtoms)
      {
        Atom *atom = static_cast<Atom *>(p);
        m_constraints->addAtomConstraint(atom->index() + 1);
      }

      m_forceField->SetConstraints(m_constraints->constraints());

      break;
    }

    return undo;
  }

  ForceFieldThread::ForceFieldThread( Molecule *molecule, OpenBabel::OBForceField* forceField,
                                      ConstraintsModel* constraints, int forceFieldID,
                                      int nSteps, int algorithm, int convergence, int task,
                                      QObject *parent ) : QThread( parent )
  {
    m_cycles = 0;
    m_molecule = molecule;
    m_constraints = constraints;
    m_forceField = forceField;
    m_forceFieldID = forceFieldID;
    m_nSteps = nSteps;
    m_algorithm = algorithm;
    m_convergence = convergence;
    m_task = task;
    m_stop = false;
  }

  int ForceFieldThread::cycles() const
  {
    return m_cycles;
  }

  void ForceFieldThread::setTask(int task)
  {
    m_task = task;
  }

  void ForceFieldThread::setNumConformers(int numConformers)
  {
    m_numConformers = numConformers;
  }

  void ForceFieldThread::copyConformers()
  {
    OBMol obmol = m_molecule->OBMol();
    // copy conformers to obmol
    m_forceField->GetConformers( obmol );
    // set the current conformer
    obmol.SetConformer(m_cycles);
    // copy the coordinates
    double *coordPtr = obmol.GetCoordinates();
    std::vector<Eigen::Vector3d> conformer;
    foreach (Atom *atom, m_molecule->atoms()) {
      while (conformer.size() < atom->id())
        conformer.push_back(Eigen::Vector3d(0.0, 0.0, 0.0));
      conformer.push_back(Eigen::Vector3d(coordPtr));
      coordPtr += 3;
    }

    // add the conformer to m_molecule
    m_molecule->addConformer(conformer, m_cycles);
    // set it to the current conformer
    m_molecule->setConformer(m_cycles);

    // copy the conformer energies
    if (obmol.HasData(OBGenericDataType::ConformerData)) {
      OBConformerData *cd = (OBConformerData*) obmol.GetData(OBGenericDataType::ConformerData);
      m_molecule->setEnergies(cd->GetEnergies());
    }
  }

  void ForceFieldThread::run()
  {
    m_stop = false;
    m_cycles = 0;

    int steps = 0;

    ostringstream buff;
    m_forceField->SetLogFile( &buff );
    m_forceField->SetLogLevel( OBFF_LOGLVL_LOW );

    OBMol mol = m_molecule->OBMol();
    if ( !m_forceField->Setup( mol, m_constraints->constraints() ) ) {
      qWarning() << "ForceFieldCommand: Could not set up force field on " << m_molecule;
      return;
    }

    if ( m_task == 0 ) {
      if ( m_algorithm == 0 ) {
        m_forceField->SteepestDescentInitialize( m_nSteps, pow( 10.0, -m_convergence )); // initialize sd

        while ( m_forceField->SteepestDescentTakeNSteps( 5 ) ) { // take 5 steps until convergence or m_nSteps taken
          m_forceField->UpdateCoordinates( mol );
          assert( mol.NumAtoms() == m_molecule->numAtoms() );
          double *coordPtr = mol.GetCoordinates();
          foreach (Atom *atom, m_molecule->atoms()) {
            atom->setPos(Eigen::Vector3d(coordPtr));
            coordPtr += 3;
          }
          m_molecule->update();
          m_cycles++;
          steps += 5;
          m_mutex.lock();
          if ( m_stop ) {
            m_mutex.unlock();
            break;
          }
          m_mutex.unlock();
          emit stepsTaken( steps );
        }
      } else if ( m_algorithm == 1 ) {
        m_forceField->ConjugateGradientsInitialize( m_nSteps, pow( 10.0, -m_convergence )); // initialize cg

        OBMol mol;
        while ( m_forceField->ConjugateGradientsTakeNSteps( 5 ) ) { // take 5 steps until convergence or m_nSteps taken
          mol = m_molecule->OBMol();
          m_forceField->UpdateCoordinates( mol );
          assert( mol.NumAtoms() == m_molecule->numAtoms() );
          double *coordPtr = mol.GetCoordinates();
          foreach (Atom *atom, m_molecule->atoms()) {
            atom->setPos(Eigen::Vector3d(coordPtr));
            coordPtr += 3;
          }
          m_molecule->update();
          m_cycles++;
          steps += 5;
          m_mutex.lock();
          if ( m_stop ) {
            m_mutex.unlock();
            break;
          }
          m_mutex.unlock();
          emit stepsTaken( steps );
        }
      }
    } else if ( m_task == 1 ) {
      int n = m_forceField->SystematicRotorSearchInitialize(m_nSteps);
      while (m_forceField->SystematicRotorSearchNextConformer(m_nSteps)) {
        copyConformers();
        m_molecule->update();
        m_cycles++;
        m_mutex.lock();
        if ( m_stop ) {
          m_mutex.unlock();
          break;
        }
        m_mutex.unlock();
        emit stepsTaken( (int) ((double) m_cycles / n * 100));
      }
    } else if ( m_task == 2 ) {
      m_forceField->RandomRotorSearchInitialize(m_numConformers, m_nSteps);
      while (m_forceField->RandomRotorSearchNextConformer(m_nSteps)) {
        copyConformers();
        m_molecule->update();
        m_cycles++;
        m_mutex.lock();
        if ( m_stop ) {
          m_mutex.unlock();
          break;
        }
        m_mutex.unlock();
        emit stepsTaken( m_cycles );
      }
    } else if ( m_task == 3 ) {
      m_forceField->WeightedRotorSearch(m_numConformers, m_nSteps);
      m_forceField->ConjugateGradients(250);
      copyConformers();
    }

    m_molecule->update();

    emit message( QObject::tr( buff.str().c_str() ) );
    m_stop = false;
  }

  void ForceFieldThread::stop()
  {
    QMutexLocker locker(&m_mutex);
    m_stop = true;
  }

  ForceFieldCommand::ForceFieldCommand( Molecule *molecule, OpenBabel::OBForceField* forceField,
                                        ConstraintsModel* constraints,
                                        int forceFieldID, int nSteps, int algorithm,
                                        int convergence, int task ) :
    m_nSteps( nSteps ),
    m_task( task ),
    m_molecule( molecule ),
    m_constraints( constraints ),
    m_thread( 0 ),
    m_dialog( 0 ),
    m_detached( false )
  {
    m_thread = new ForceFieldThread( molecule, forceField, constraints,
                                     forceFieldID, nSteps, algorithm,
                                     convergence, task );

    connect(m_thread, SIGNAL(message(QString)), this, SIGNAL(message(QString)));

    m_moleculeCopy = *molecule;
  }

  ForceFieldCommand::~ForceFieldCommand()
  {
    if ( !m_detached ) {
      if ( m_thread->isRunning() ) {
        m_thread->stop();
        m_thread->wait();
      }
      delete m_thread;

      if ( m_dialog ) {
        delete m_dialog;
      }
    }
  }

  void ForceFieldCommand::setTask(int task)
  {
    m_task = task;
  }

  void ForceFieldCommand::setNumConformers(int numConformers)
  {
    m_numConformers = numConformers;
  }

  void ForceFieldCommand::redo()
  {
    if(!m_dialog) {
      if ( m_task == 0 )
        m_dialog = new QProgressDialog( QObject::tr( "Forcefield Optimization" ),
                                        QObject::tr( "Cancel" ), 0,  m_nSteps );
      else if ( m_task == 1)
        m_dialog = new QProgressDialog( QObject::tr( "Systematic Rotor Search" ),
                                        QObject::tr( "Cancel" ), 0,  100 );
      else if ( m_task == 2)
        m_dialog = new QProgressDialog( QObject::tr( "Random Rotor Search" ),
                                        QObject::tr( "Cancel" ), 0,  100 );
      else if ( m_task == 3)
        m_dialog = new QProgressDialog( QObject::tr( "Weighted Rotor Search" ),
                                        QObject::tr( "Cancel" ), 0,  100 );


      QObject::connect( m_thread, SIGNAL( stepsTaken( int ) ), m_dialog, SLOT( setValue( int ) ) );
      QObject::connect( m_dialog, SIGNAL( canceled() ), m_thread, SLOT( stop() ) );
      QObject::connect( m_thread, SIGNAL( finished() ), m_dialog, SLOT( close() ) );
    }

    m_thread->setTask(m_task);
    m_thread->setNumConformers(m_numConformers);
    m_thread->start();
  }

  void ForceFieldCommand::undo()
  {
    m_thread->stop();
    m_thread->wait();

    *m_molecule = m_moleculeCopy;
    m_molecule->update();
  }

  bool ForceFieldCommand::mergeWith( const QUndoCommand *command )
  {
    const ForceFieldCommand *gc = dynamic_cast<const ForceFieldCommand *>( command );
    if ( gc ) {
      // delete our current info
      cleanup();
      gc->detach();
      m_thread = gc->thread();
      m_dialog = gc->progressDialog();
    }
    // received another of the same call
    return true;
  }

  ForceFieldThread *ForceFieldCommand::thread() const
  {
    return m_thread;
  }

  QProgressDialog *ForceFieldCommand::progressDialog() const
  {
    return m_dialog;
  }

  void ForceFieldCommand::detach() const
  {
    m_detached = true;
  }

  void ForceFieldCommand::cleanup()
  {
    if ( !m_detached ) {
      if ( m_thread->isRunning() ) {
        m_thread->stop();
        m_thread->wait();
      }
      delete m_thread;

      if ( m_dialog ) {
        delete m_dialog;
      }
    }
  }

  int ForceFieldCommand::id() const
  {
    return 54381241;
  }

} // end namespace Avogadro

#include "forcefieldextension.moc"
Q_EXPORT_PLUGIN2(forcefieldextension, Avogadro::ForceFieldExtensionFactory)
