/**********************************************************************
  Ghemical - Ghemical Plugin for Avogadro

  Copyright (C) 2006 by Donald Ephraim Curtis
  Copyright (C) 2006 by Geoffrey R. Hutchison

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

#include "ghemicalextension.h"
#include <avogadro/primitive.h>
#include <avogadro/color.h>
#include <avogadro/glwidget.h>

#include <QtGui>
#include <QProgressDialog>

using namespace std;
using namespace OpenBabel;

  namespace Avogadro {
    GhemicalExtension::GhemicalExtension(QObject *parent) : QObject(parent)
    {
      QAction *action;
      m_forceField = OBForceField::FindForceField("Ghemical");
      m_Dialog = new ForceFieldDialog;

      if (m_forceField) { // make sure we can actually find and run it!
        action = new QAction(this);
        action->setText("Setup Force Field...");
        m_actions.append(action);
      }

      action = new QAction(this);
      action->setText("Calculate Energy");
      m_actions.append(action);

      action = new QAction(this);
      action->setText("Optimize Geometry");
      m_actions.append(action);

      action = new QAction(this);
      action->setText("Rotor Search");
      m_actions.append(action);
    }

    GhemicalExtension::~GhemicalExtension() 
    {
    }

    QList<QAction *> GhemicalExtension::actions() const
    {
      return m_actions;
    }

    QUndoCommand* GhemicalExtension::performAction(QAction *action, Molecule *molecule, 
        GLWidget *, QTextEdit *textEdit)
    {
      QUndoCommand *undo = NULL;
      ostringstream buff;
      int i = m_actions.indexOf(action);

      switch(i)
      {
        case 0: // setup force field
          m_Dialog->show();
          break;
        case 1: // calculate energy 
          if (!m_forceField)
            break;

          m_forceField->SetLogFile(&buff);
          m_forceField->SetLogLevel(OBFF_LOGLVL_HIGH);

          if (!m_forceField->Setup(*molecule)) {
            qDebug() << "Could not set up force field on " << molecule;
            break;
          }

          m_forceField->Energy();
          textEdit->append(tr(buff.str().c_str()));
          break;
        case 2: // geometry optimization
          undo = new GhemicalCommand(molecule, m_forceField, textEdit, 0, m_Dialog->nSteps(), 
              m_Dialog->algorithm(), m_Dialog->gradients(), m_Dialog->convergence(), 0);
          undo->setText(QObject::tr("Geometric Optimization"));
          break;
        case 3: // systematic rotor search
          undo = new GhemicalCommand(molecule, m_forceField, textEdit, 0, m_Dialog->nSteps(), 
              m_Dialog->algorithm(), m_Dialog->gradients(), m_Dialog->convergence(), 1);
          undo->setText(QObject::tr("Rotor Search"));
          break;

      }

      return undo;
    }

    GhemicalThread::GhemicalThread(Molecule *molecule, OpenBabel::OBForceField* forceField, 
        QTextEdit *textEdit, int forceFieldID, int nSteps, int algorithm,
        int gradients, int convergence, int task, QObject *parent) : QThread(parent)
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

    int GhemicalThread::cycles() const
    {
      return m_cycles;
    }

    void GhemicalThread::run()
    {
      m_stop = false;
      m_cycles = 0;

      ostringstream buff;
      m_forceField->SetLogFile(&buff);
      m_forceField->SetLogLevel(OBFF_LOGLVL_LOW);

      if (!m_forceField->Setup(*m_molecule)) {
        qWarning() << "GhemicalCommand: Could not set up force field on " << m_molecule;
        return;
      }

      if (m_task == 0) {
        if (m_algorithm == 0) {
          if (m_gradients == 0)
          {
            m_forceField->SteepestDescentInitialize(m_nSteps, pow(10.0, -m_convergence), OBFF_NUMERICAL_GRADIENT); // initialize sd
          }
          else
          {
            m_forceField->SteepestDescentInitialize(m_nSteps, pow(10.0, -m_convergence), OBFF_ANALYTICAL_GRADIENT); // initialize sd
          }

          while (m_forceField->SteepestDescentTakeNSteps(5)) { // take 5 steps until convergence or m_nSteps taken
            m_forceField->UpdateCoordinates(*m_molecule);
            m_molecule->update();
            m_cycles++;
            emit cyclesChanged(m_cycles);
          }
        } else if (m_algorithm == 1) {
          if (m_gradients == 0) {
            m_forceField->ConjugateGradientsInitialize(m_nSteps, pow(10.0, -m_convergence), OBFF_NUMERICAL_GRADIENT); // initialize cg
          } else {
            m_forceField->ConjugateGradientsInitialize(m_nSteps, pow(10.0, -m_convergence), OBFF_ANALYTICAL_GRADIENT); // initialize cg
          }

          while (m_forceField->ConjugateGradientsTakeNSteps(5)) { // take 5 steps until convergence or m_nSteps taken
            m_forceField->UpdateCoordinates(*m_molecule);
            m_molecule->update();
            m_cycles++;
            emit cyclesChanged(m_cycles);
            if(m_stop) {
              break;
            }
          }
        }
      } else if (m_task == 1) {
        m_forceField->SystematicRotorSearch();
        m_forceField->UpdateCoordinates(*m_molecule);
        m_molecule->update();
      }

      m_textEdit->append(QObject::tr(buff.str().c_str()));
      m_stop = false;
    }

    void GhemicalThread::stop()
    {
      m_stop = true;
    }

    GhemicalCommand::GhemicalCommand(Molecule *molecule, OpenBabel::OBForceField* forceField, 
        QTextEdit *textEdit, int forceFieldID, int nSteps, int algorithm,
        int gradients, int convergence, int task) : 
        m_nSteps(nSteps), 
        m_molecule(molecule), 
        m_textEdit(textEdit),
        m_thread(0),
        m_dialog(0) 
    {
      m_thread = new GhemicalThread(molecule, forceField, textEdit, 
          forceFieldID, nSteps, algorithm,
          gradients, convergence, task);

      m_moleculeCopy = *molecule;
    }

    GhemicalCommand::~GhemicalCommand()
    {
      if(m_thread->isRunning())
      {
        m_thread->stop();
        m_thread->wait();
      }
      delete m_thread;
    }

    void GhemicalCommand::redo() {
      QProgressDialog *m_dialog = new QProgressDialog(QObject::tr("Forcefield Optimization"), 
          QObject::tr("Cancel"), 0,  m_nSteps);

      QObject::connect(m_thread, SIGNAL(cyclesChanged(int)), m_dialog, SLOT(setValue(int)));
      QObject::connect(m_dialog, SIGNAL(canceled()), m_thread, SLOT(stop()));
      QObject::connect(m_thread, SIGNAL(finished()), m_dialog, SLOT(close()));

      m_thread->start();
    }

    void GhemicalCommand::undo() {
      m_thread->stop();
      m_thread->wait();

      m_textEdit->undo();
      *m_molecule = m_moleculeCopy;
      // for(int i=0; i<m_cycles; i++) {
      // }
    }

    bool GhemicalCommand::mergeWith ( const QUndoCommand * )
    {
      // received another of the same call
      return true;
    }

    int GhemicalCommand::id() const
    {
      return 54381241;
    }

  } // end namespace Avogadro

#include "ghemicalextension.moc"
Q_EXPORT_PLUGIN2(ghemicalextension, Avogadro::GhemicalExtensionFactory)
