/**********************************************************************
  docking extension.cpp - Docking Plugin for Avogadro

  Copyright (C) 2007 by Tim Vandermeersch

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

#include "dockingextension.h"
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
  enum DockingExtensionIndex
    {
      CreateLigandIndex = 0,
      CreatePocketIndex,
      SelectLigandIndex,
      SelectPocketIndex,
      SelectLigPockIndex,
      DockIndex,
      PosesIndex,
    };

  DockingExtension::DockingExtension( QObject *parent ) : Extension( parent )
  {
    QAction *action;
    m_dock = new OpenBabel::OBDock;
    m_ligandDialog = new LigandDialog;
    m_pocketDialog = new PocketDialog;
    
    if ( m_dock ) {
      action = new QAction( this );
      action->setText( tr("Create Ligand..." ));
      action->setData(CreateLigandIndex);
      m_actions.append( action );
      
      action = new QAction( this );
      action->setText( tr("Create Binding Pocket..." ));
      action->setData(CreatePocketIndex);
      m_actions.append( action );

      action = new QAction( this );
      action->setSeparator(true);
      m_actions.append( action );

      action = new QAction( this );
      action->setText( tr("Select Ligand" ));
      action->setData(SelectLigandIndex);
      m_actions.append( action );

      action = new QAction( this );
      action->setText( tr("Select Binding Pocket" ));
      action->setData(SelectPocketIndex);
      m_actions.append( action );

      action = new QAction( this );
      action->setText( tr("Select Ligand + Pocket" ));
      action->setData(SelectLigPockIndex);
      m_actions.append( action );

      action = new QAction( this );
      action->setSeparator(true);
      m_actions.append( action );
      
      action = new QAction( this );
      action->setText( tr("Dock..." ));
      action->setData(DockIndex);
      m_actions.append( action );

      action = new QAction( this );
      action->setText( tr("View Poses..." ));
      action->setData(PosesIndex);
      m_actions.append( action );

    }

  }

  DockingExtension::~DockingExtension()
  {}

  QList<QAction *> DockingExtension::actions() const
  {
    return m_actions;
  }

  QString DockingExtension::menuPath(QAction *action) const
  {
    int i = action->data().toInt();
    switch(i) {
    case CreateLigandIndex:
    case CreatePocketIndex:
    case DockIndex:
    case PosesIndex:
    case SelectLigandIndex:
    case SelectPocketIndex:
    case SelectLigPockIndex:
      return tr("&Extensions") + ">" + tr("&Docking");
      break;
    default:
      break;
    };
    return QString();
  }

  QUndoCommand* DockingExtension::performAction( QAction *action, GLWidget *widget)
  {
    QUndoCommand *undo = NULL;
    QList<Primitive *> selectedAtoms;
    vector<int>::iterator j;
    
    connect(molecule, SIGNAL( primitiveRemoved(Primitive *) ), this, SLOT( primitiveRemoved(Primitive *) ));

    int i = action->data().toInt();
    switch ( i ) {
    case CreateLigandIndex: // create ligand
      m_fileName = QString("");
      m_ligandDialog->setDock(m_dock);
      m_ligandDialog->setMolecule(molecule);
      m_ligandDialog->setWidget(widget);
      m_ligandDialog->show();
      break;
    case CreatePocketIndex: // create binding pocket
      m_pocketDialog->setDock(m_dock);
      m_pocketDialog->setMolecule(molecule);
      m_pocketDialog->setWidget(widget);
      m_pocketDialog->show();
      break;
 
    case SelectLigandIndex: // select ligand
      m_ligand = m_ligandDialog->ligand();
      m_fileName = m_ligandDialog->fileName();
	
      if (!m_fileName.isEmpty()) {
        QMessageBox::warning(NULL, tr("Select ligand"), 
                             tr("Your ligands are stored in a different file, open that file to view them"));
        break;
      }
      if (m_ligand.empty()) {
        QMessageBox::warning(NULL, tr("Select ligand"), tr("Create a ligand first..."));
        break;
      }

      for (j = m_ligand.begin(); j != m_ligand.end(); ++j)
        selectedAtoms.append(static_cast<Atom*>(molecule->GetAtom(*j)));

      widget->clearSelected();
      widget->setSelected(selectedAtoms, true);
      widget->update();
      break;
    case SelectPocketIndex: // select pocket
      m_pocket = m_pocketDialog->pocket();

      if (m_pocket.empty())
        QMessageBox::warning(NULL, tr("Select binding pocket"), tr("Create a binding pocket first..."));

      for (j = m_pocket.begin(); j != m_pocket.end(); ++j)
        selectedAtoms.append(static_cast<Atom*>(molecule->GetAtom(*j)));

      widget->clearSelected();
      widget->setSelected(selectedAtoms, true);
      widget->update();
      break;
    case SelectLigPockIndex: // select ligand + pocket
      m_ligand = m_ligandDialog->ligand();
      m_pocket = m_pocketDialog->pocket();
	
      if (!m_fileName.isEmpty())
        QMessageBox::warning(NULL, tr("Select ligand + pocket"), 
                             tr("Your ligands are stored in a different file, only the pocket will be selected"));
      else if (m_ligand.empty()) {
        QMessageBox::warning(NULL, tr("Select ligand + pocket"), tr("Create a ligand first..."));
        break;
      }	
      if (m_pocket.empty()) {
        QMessageBox::warning(NULL, tr("Select ligand + pocket"), tr("Create a binding pocket first..."));
        break;
      }
	
      for (j = m_ligand.begin(); j != m_ligand.end(); ++j)
        selectedAtoms.append(static_cast<Atom*>(molecule->GetAtom(*j)));

      for (j = m_pocket.begin(); j != m_pocket.end(); ++j)
        selectedAtoms.append(static_cast<Atom*>(molecule->GetAtom(*j)));

      widget->clearSelected();
      widget->setSelected(selectedAtoms, true);
      widget->update();
      break;
    case DockIndex: // calculate energy
      if ( !m_dock )
        break;

      //m_dockingDialog->setup(molecule, m_forceField, m_constraints, textEdit, 
      //    0, m_Dialog->nSteps(), m_Dialog->algorithm(), m_Dialog->gradients(), m_Dialog->convergence());
      //m_dockingDialog->show();
      break;
    case PosesIndex: // show poses dialog
      //m_posesDialog->show();
      break;
    }

    return undo;
  }
  
  void DockingExtension::primitiveRemoved(Primitive *primitive)
  {
    // if atoms are removed, the previously obtained indices are no longer
    // valid, so the user will have to slect them again.
    if (primitive->type() == Primitive::AtomType) {
      m_ligand.clear();
      m_pocket.clear();
    }
  }
  /*
    DockingThread::DockingThread( Molecule *molecule, OpenBabel::OBDock* dock,
    QTextEdit *textEdit, int forceFieldID,
    int nSteps, int algorithm, int gradients, int convergence, int task,
    QObject *parent ) : QThread( parent )
    {
    m_cycles = 0;
    m_molecule = molecule;
    m_dock = dock;
    m_textEdit = textEdit;
    m_forceFieldID = forceFieldID;
    m_nSteps = nSteps;
    m_algorithm = algorithm;
    m_gradients = gradients;
    m_convergence = convergence;
    m_task = task;
    m_stop = false;
    }

    int DockingThread::cycles() const
    {
    return m_cycles;
    }
  
    void DockingThread::setTask(int task)
    {
    m_task = task;
    }
  
    void DockingThread::setNumConformers(int numConformers)
    {
    m_numConformers = numConformers;
    }
 
    void DockingThread::run()
    {
    QWriteLocker locker( m_molecule->lock() );
    m_stop = false;
    }

    void DockingThread::stop()
    {
    QMutexLocker locker(&m_mutex);
    m_stop = true;
    }

    DockingCommand::DockingCommand( Molecule *molecule, OpenBabel::OBDock* forceField,
    QTextEdit *textEdit,
    int forceFieldID, int nSteps, int algorithm, int gradients,
    int convergence, int task ) :
    m_nSteps( nSteps ),
    m_task( task ),
    m_molecule( molecule ),
    m_textEdit( textEdit ),
    m_thread( 0 ),
    m_dialog( 0 ),
    m_detached( false )
    {
    m_thread = new DockingThread( molecule, forceField, 
    textEdit, forceFieldID, nSteps, algorithm,
    gradients, convergence, task );

    m_moleculeCopy = *molecule;
    }

    DockingCommand::~DockingCommand()
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

    void DockingCommand::setTask(int task)
    {
    m_task = task;
    }
  
    void DockingCommand::setNumConformers(int numConformers)
    {
    m_numConformers = numConformers;
    }
  
    void DockingCommand::redo()
    {
    if(!m_dialog) {
    if ( m_task == 0 )
    m_dialog = new QProgressDialog( QObject::tr( "Docking" ),
    QObject::tr( "Cancel" ), 0,  m_nSteps );

    QObject::connect( m_thread, SIGNAL( stepsTaken( int ) ), m_dialog, SLOT( setValue( int ) ) );
    QObject::connect( m_dialog, SIGNAL( canceled() ), m_thread, SLOT( stop() ) );
    QObject::connect( m_thread, SIGNAL( finished() ), m_dialog, SLOT( close() ) );
    }

    m_thread->setTask(m_task);
    m_thread->setNumConformers(m_numConformers);
    m_thread->start();
    }

    void DockingCommand::undo()
    {
    m_thread->stop();
    m_thread->wait();

    m_textEdit->undo();
    *m_molecule = m_moleculeCopy;
    }

    bool DockingCommand::mergeWith( const QUndoCommand *command )
    {
    const DockingCommand *gc = dynamic_cast<const DockingCommand *>( command );
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

    DockingThread *DockingCommand::thread() const
    {
    return m_thread;
    }

    QProgressDialog *DockingCommand::progressDialog() const
    {
    return m_dialog;
    }

    void DockingCommand::detach() const
    {
    m_detached = true;
    }

    void DockingCommand::cleanup()
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

    int DockingCommand::id() const
    {
    return 54381241;
    }
  */
} // end namespace Avogadro

#include "dockingextension.moc"
Q_EXPORT_PLUGIN2( dockingextension, Avogadro::DockingExtensionFactory )
