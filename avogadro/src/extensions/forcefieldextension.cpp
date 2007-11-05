/**********************************************************************
  forcefieldextension.cpp - molecular mechanics force field Plugin for Avogadro

  Copyright (C) 2006 by Donald Ephraim Curtis
  Copyright (C) 2006-2007 by Geoffrey R. Hutchison

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.sourceforge.net/>

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

#include <QtGui>
#include <QProgressDialog>
#include <QWriteLocker>
#include <QMutex>
#include <QMutexLocker>

using namespace std;
using namespace OpenBabel;

namespace Avogadro
{
  enum ForceFieldExtensionIndex
  {
    OptimizeGeometryIndex = 0,
    CalculateEnergyIndex,
    RotorSearchIndex,
    SetupForceFieldIndex
  };

  ForceFieldExtension::ForceFieldExtension( QObject *parent ) : QObject( parent )
  {
    QAction *action;
    m_forceField = OBForceField::FindForceField( "Ghemical" );
    m_Dialog = new ForceFieldDialog;

    if ( m_forceField ) { // make sure we can actually find and run it!

      action = new QAction( this );
      action->setText( tr("Optimize Geometry" ));
      action->setData(OptimizeGeometryIndex);
      m_actions.append( action );
      
      action = new QAction( this );
      action->setText( tr("Calculate Energy" ));
      action->setData(CalculateEnergyIndex);
      m_actions.append( action );
      
      action = new QAction( this );
      action->setText( tr("Rotor Search" ));
      action->setData(RotorSearchIndex);
      m_actions.append( action );
      
      action = new QAction( this );
      action->setText( tr("Setup Force Field..." ));
      action->setData(SetupForceFieldIndex);
      m_actions.append( action );
    }

  }

  ForceFieldExtension::~ForceFieldExtension()
  {}

  QList<QAction *> ForceFieldExtension::actions() const
  {
    return m_actions;
  }

  QString ForceFieldExtension::menuPath(QAction *action) const
  {
    int i = action->data().toInt();
    switch(i) {
      case CalculateEnergyIndex:
      case RotorSearchIndex:
      case SetupForceFieldIndex:
        return tr("&Tools>Molecular Mechanics");
        break;
      default:
        break;
    };
    return QString();
  }

  QUndoCommand* ForceFieldExtension::performAction( QAction *action, Molecule *molecule,
      GLWidget *, QTextEdit *textEdit )
  {
    QUndoCommand *undo = NULL;
    ostringstream buff;

    switch (m_Dialog->forceFieldID()) {
    case 1:
      m_forceField = OBForceField::FindForceField( "MMF94" );
      break;
    case 2:
      m_forceField = OBForceField::FindForceField( "UFF" );
      break;
    case 0:
    default:
      m_forceField = OBForceField::FindForceField( "Ghemical" );
      break;
    }

    int i = action->data().toInt();
    switch ( i ) {
      case SetupForceFieldIndex: // setup force field
        m_Dialog->show();
        break;
      case CalculateEnergyIndex: // calculate energy
        if ( !m_forceField )
          break;

        m_forceField->SetLogFile( &buff );
        m_forceField->SetLogLevel( OBFF_LOGLVL_HIGH );

        if ( !m_forceField->Setup( *molecule ) ) {
          qDebug() << "Could not set up force field on " << molecule;
          break;
        }

        m_forceField->Energy();
        textEdit->append( tr( buff.str().c_str() ) );
        break;
      case RotorSearchIndex: // random rotor search
        if (!m_forceField)
          break;

        undo = new ForceFieldCommand( molecule, m_forceField, textEdit, 0, m_Dialog->nSteps(),
                                    m_Dialog->algorithm(), m_Dialog->gradients(), m_Dialog->convergence(), 1 );
        undo->setText( QObject::tr( "Rotor Search" ) );
        break;

      case OptimizeGeometryIndex: // geometry optimization
        if (!m_forceField)
          break;

        undo = new ForceFieldCommand( molecule, m_forceField, textEdit, 0, m_Dialog->nSteps(),
                                    m_Dialog->algorithm(), m_Dialog->gradients(), m_Dialog->convergence(), 0 );
        undo->setText( QObject::tr( "Geometric Optimization" ) );
        break;
    }

    return undo;
  }

  ForceFieldThread::ForceFieldThread( Molecule *molecule, OpenBabel::OBForceField* forceField,
                                  QTextEdit *textEdit, int forceFieldID, int nSteps, int algorithm,
                                  int gradients, int convergence, int task, QObject *parent ) : QThread( parent )
  {
    m_cycles = 0;
    m_molecule = molecule;
    m_forceField = forceField;
    m_textEdit = textEdit;
    m_forceFieldID = forceFieldID;
    m_nSteps = nSteps;
    m_algorithm = algorithm;
    m_gradients = gradients;
    m_convergence = convergence;
    m_task = task;
    m_stop = false;
  }

  int ForceFieldThread::cycles() const
  {
    return m_cycles;
  }

  void ForceFieldThread::run()
  {
    QWriteLocker locker( m_molecule->lock() );
    m_stop = false;
    m_cycles = 0;

    int steps = 0;

    ostringstream buff;
    m_forceField->SetLogFile( &buff );
    m_forceField->SetLogLevel( OBFF_LOGLVL_LOW );

    if ( !m_forceField->Setup( *m_molecule ) ) {
      qWarning() << "ForceFieldCommand: Could not set up force field on " << m_molecule;
      return;
    }

    if ( m_task == 0 ) {
      if ( m_algorithm == 0 ) {
        if ( m_gradients == 0 ) {
          m_forceField->SteepestDescentInitialize( m_nSteps, pow( 10.0, -m_convergence ), OBFF_NUMERICAL_GRADIENT ); // initialize sd
        } else {
          m_forceField->SteepestDescentInitialize( m_nSteps, pow( 10.0, -m_convergence ), OBFF_ANALYTICAL_GRADIENT ); // initialize sd
        }

        while ( m_forceField->SteepestDescentTakeNSteps( 5 ) ) { // take 5 steps until convergence or m_nSteps taken
          m_forceField->UpdateCoordinates( *m_molecule );
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
        if ( m_gradients == 0 ) {
          m_forceField->ConjugateGradientsInitialize( m_nSteps, pow( 10.0, -m_convergence ), OBFF_NUMERICAL_GRADIENT ); // initialize cg
        } else {
          m_forceField->ConjugateGradientsInitialize( m_nSteps, pow( 10.0, -m_convergence ), OBFF_ANALYTICAL_GRADIENT ); // initialize cg
        }

        while ( m_forceField->ConjugateGradientsTakeNSteps( 5 ) ) { // take 5 steps until convergence or m_nSteps taken
          m_forceField->UpdateCoordinates( *m_molecule );
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
      m_forceField->RandomRotorSearch(10, 50);
      m_forceField->UpdateCoordinates( *m_molecule );
      m_molecule->update();
    }

    m_textEdit->append( QObject::tr( buff.str().c_str() ) );
    m_stop = false;
  }

  void ForceFieldThread::stop()
  {
    QMutexLocker locker(&m_mutex);
    m_stop = true;
  }

  ForceFieldCommand::ForceFieldCommand( Molecule *molecule, OpenBabel::OBForceField* forceField,
                                    QTextEdit *textEdit, int forceFieldID, int nSteps, int algorithm,
                                    int gradients, int convergence, int task ) :
      m_nSteps( nSteps ),
      m_molecule( molecule ),
      m_textEdit( textEdit ),
      m_thread( 0 ),
      m_dialog( 0 ),
      m_detached( false )
  {
    m_thread = new ForceFieldThread( molecule, forceField, textEdit,
                                   forceFieldID, nSteps, algorithm,
                                   gradients, convergence, task );

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

  void ForceFieldCommand::redo()
  {
    if(!m_dialog) {
      m_dialog = new QProgressDialog( QObject::tr( "Forcefield Optimization" ),
                                      QObject::tr( "Cancel" ), 0,  m_nSteps );
      QObject::connect( m_thread, SIGNAL( stepsTaken( int ) ), m_dialog, SLOT( setValue( int ) ) );
      QObject::connect( m_dialog, SIGNAL( canceled() ), m_thread, SLOT( stop() ) );
      QObject::connect( m_thread, SIGNAL( finished() ), m_dialog, SLOT( close() ) );
    }

    m_thread->start();
  }
  
  void ForceFieldCommand::undo()
  {
    m_thread->stop();
    m_thread->wait();

    m_textEdit->undo();
    *m_molecule = m_moleculeCopy;
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
Q_EXPORT_PLUGIN2( ghemicalextension, Avogadro::ForceFieldExtensionFactory )
