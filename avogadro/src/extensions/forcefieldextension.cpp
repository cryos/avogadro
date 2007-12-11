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
#include <QAbstractTableModel>

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
    FixAtomsIndex
  };

  ForceFieldExtension::ForceFieldExtension( QObject *parent ) : QObject( parent )
  {
    QAction *action;
    m_forceField = OBForceField::FindForceField( "Ghemical" );
    m_Dialog = new ForceFieldDialog;
    m_conformerDialog = new ConformerSearchDialog;
    m_ConstraintsDialog = new ConstraintsDialog;
    m_constraints = new ConstraintsModel;
    
    m_ConstraintsDialog->setModel(m_constraints);
    
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
      action->setText( tr("Conformer Search..." ));
      action->setData(ConformerSearchIndex);
      m_actions.append( action );

      action = new QAction( this );
      action->setText( tr("Setup Force Field..." ));
      action->setData(SetupForceFieldIndex);
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
      case ConformerSearchIndex:
      case SetupForceFieldIndex:
      case ConstraintsIndex:
      case IgnoreAtomsIndex: 
      case FixAtomsIndex: 
        return tr("&Extensions") + ">" + tr("&Molecular Mechanics");
        break;
      default:
        break;
    };
    return QString();
  }

  QUndoCommand* ForceFieldExtension::performAction( QAction *action, Molecule *molecule,
      GLWidget *widget, QTextEdit *textEdit )
  {
    QUndoCommand *undo = NULL;
    OpenBabel::OBForceField *copyForceField = NULL;
    QList<Primitive*> selectedAtoms;
    ostringstream buff;
    
    OpenBabel::OBFFConstraints constraints = m_forceField->GetConstraints(); // load constraints
    switch (m_Dialog->forceFieldID()) {
    case 1:
      m_forceField = OBForceField::FindForceField( "MMFF94" );
      break;
    case 2:
      m_forceField = OBForceField::FindForceField( "UFF" );
      break;
    case 0:
    default:
      m_forceField = OBForceField::FindForceField( "Ghemical" );
      break;
    }
    m_forceField->SetConstraints(constraints); // save constraints
    m_ConstraintsDialog->setForceField(m_forceField);


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

        if ( !m_forceField->Setup( *molecule, m_constraints->constraints() ) ) {
          qDebug() << "Could not set up force field on " << molecule;
          break;
        }

        m_forceField->Energy();
        textEdit->append( tr( buff.str().c_str() ) );
        break;
      case ConformerSearchIndex: // conformer search
        if (!m_forceField)
          break;

        m_conformerDialog->setup(molecule, m_forceField, m_constraints, textEdit, 
	    0, m_Dialog->nSteps(), m_Dialog->algorithm(), m_Dialog->gradients(), m_Dialog->convergence());
        m_conformerDialog->show();
        break;
      case OptimizeGeometryIndex: // geometry optimization
        if (!m_forceField)
          break;
	
	undo = new ForceFieldCommand( molecule, m_forceField, m_constraints, textEdit, 
	    0, m_Dialog->nSteps(), m_Dialog->algorithm(), m_Dialog->gradients(), 
	    m_Dialog->convergence(), 0 );
 
        undo->setText( QObject::tr( "Geometric Optimization" ) );
        break;
      case ConstraintsIndex: // show constraints dialog
        m_ConstraintsDialog->setMolecule(molecule);
        m_ConstraintsDialog->setForceField(m_forceField);
        m_ConstraintsDialog->show();
        break;
      case IgnoreAtomsIndex: // ignore the selected atoms
        selectedAtoms = widget->selectedPrimitives();

	for (int i = 0; i < selectedAtoms.size(); ++i) {
          if (selectedAtoms[i]->type() == Primitive::AtomType) {
            Atom *atom = static_cast<Atom *>(selectedAtoms[i]);
	    m_constraints->addIgnore(atom->GetIdx());
          }
	}
        
	// copy constraints to all force fields
	copyForceField = m_forceField;
        m_forceField = OBForceField::FindForceField( "Ghemical" );
        m_forceField->SetConstraints(m_constraints->constraints());
        m_forceField = OBForceField::FindForceField( "MMFF94" );
        m_forceField->SetConstraints(m_constraints->constraints());
        m_forceField = OBForceField::FindForceField( "UFF" );
        m_forceField->SetConstraints(m_constraints->constraints());
        m_forceField = copyForceField;

	break;
      case FixAtomsIndex: // fix the selected atom positions
        selectedAtoms = widget->selectedPrimitives();

	for (int i = 0; i < selectedAtoms.size(); ++i) {
          if (selectedAtoms[i]->type() == Primitive::AtomType) {
            Atom *atom = static_cast<Atom *>(selectedAtoms[i]);
	    m_constraints->addAtomConstraint(atom->GetIdx());
          }
	}
        
	// copy constraints to all force fields
	copyForceField = m_forceField;
        m_forceField = OBForceField::FindForceField( "Ghemical" );
        m_forceField->SetConstraints(m_constraints->constraints());
        m_forceField = OBForceField::FindForceField( "MMFF94" );
        m_forceField->SetConstraints(m_constraints->constraints());
        m_forceField = OBForceField::FindForceField( "UFF" );
        m_forceField->SetConstraints(m_constraints->constraints());
        m_forceField = copyForceField;

	break;
    }

    return undo;
  }

  ForceFieldThread::ForceFieldThread( Molecule *molecule, OpenBabel::OBForceField* forceField,
                                  ConstraintsModel* constraints, QTextEdit *textEdit, int forceFieldID,
				  int nSteps, int algorithm, int gradients, int convergence, int task,
				  QObject *parent ) : QThread( parent )
  {
    m_cycles = 0;
    m_molecule = molecule;
    m_constraints = constraints;
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
  
  void ForceFieldThread::setTask(int task)
  {
    m_task = task;
  }
  
  void ForceFieldThread::setNumConformers(int numConformers)
  {
    m_numConformers = numConformers;
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

    if ( !m_forceField->Setup( *m_molecule, m_constraints->constraints() ) ) {
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
      int n = m_forceField->SystematicRotorSearchInitialize(m_nSteps);
      while (m_forceField->SystematicRotorSearchNextConformer(m_nSteps)) {
        m_forceField->GetConformers( *m_molecule );
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
        m_forceField->GetConformers( *m_molecule );
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
      m_forceField->GetConformers( *m_molecule );
      m_forceField->ConjugateGradients(250);
    }

    m_forceField->GetConformers( *m_molecule );
    m_molecule->update();

    m_textEdit->append( QObject::tr( buff.str().c_str() ) );
    m_stop = false;
  }

  void ForceFieldThread::stop()
  {
    QMutexLocker locker(&m_mutex);
    m_stop = true;
  }

  ForceFieldCommand::ForceFieldCommand( Molecule *molecule, OpenBabel::OBForceField* forceField,
                                    ConstraintsModel* constraints, QTextEdit *textEdit,
				    int forceFieldID, int nSteps, int algorithm, int gradients,
				    int convergence, int task ) :
      m_nSteps( nSteps ),
      m_task( task ),
      m_molecule( molecule ),
      m_constraints( constraints ),
      m_textEdit( textEdit ),
      m_thread( 0 ),
      m_dialog( 0 ),
      m_detached( false )
  {
    m_thread = new ForceFieldThread( molecule, forceField, constraints,
                                   textEdit, forceFieldID, nSteps, algorithm,
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
