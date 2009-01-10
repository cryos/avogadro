/**********************************************************************
  AutoOptTool - Automatic Optimization Tool for Avogadro

  Copyright (C) 2007,2008 by Marcus D. Hanwell
  Copyright (C) 2007 by Geoffrey R. Hutchison
  Copyright (C) 2007 by Benoit Jacob
  Copyright (C) 2008 by Tim Vandermeersch

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

#include "autoopttool.h"
#include <avogadro/navigate.h>
#include <avogadro/primitive.h>
#include <avogadro/atom.h>
#include <avogadro/bond.h>
#include <avogadro/color.h>
#include <avogadro/glwidget.h>
#include <avogadro/camera.h>
#include <avogadro/toolgroup.h>

#include <openbabel/obiter.h>
#include <openbabel/mol.h>

#include <QDebug>
#include <QtPlugin>
#include <QLabel>
#include <QVBoxLayout>
#include <QCheckBox>

using namespace std;
using namespace OpenBabel;
using namespace Eigen;

namespace Avogadro {

  AutoOptTool::AutoOptTool(QObject *parent) : Tool(parent), m_clickedAtom(0),
  m_leftButtonPressed(false), m_midButtonPressed(false), m_rightButtonPressed(false),
  m_running(false), m_block(false), m_setupFailed(false), m_timerId(0) ,m_toolGroup(0),
  m_settingsWidget(0), m_lastEnergy(0.0)
  {
    QAction *action = activateAction();
    action->setIcon(QIcon(QString::fromUtf8(":/autoopttool/autoopttool.png")));
    action->setToolTip(tr("Auto Optimization Tool\n\n"
          "Navigation Functions when Clicking in empty space.\n"
          "Left Mouse: Rotate Space\n"
          "Middle Mouse: Zoom Space\n"
          "Right Mouse: Move Space\n\n"
          "Extra Function when running\n"
          "Left Mouse: Click and drag atoms to move them"));
    m_forceField = OBForceField::FindForceField( "UFF" );
    // Check that the force field exists and was initialised OK
    if (!m_forceField)
    {
      // We can't do anything is the force field cannot be found - OB issue
      emit setupFailed();
      return;
    }
    m_thread = new AutoOptThread;
    connect(m_thread, SIGNAL(finished(bool)), this, SLOT(finished(bool)));
    connect(m_thread, SIGNAL(setupDone()),    this, SLOT(setupDone()));
    connect(m_thread, SIGNAL(setupFailed()),  this, SLOT(setupFailed()));
    connect(m_thread, SIGNAL(setupSucces()),  this, SLOT(setupSucces()));

    OBPlugin::ListAsVector("forcefields", "ids", m_forceFieldList);
    //action->setShortcut(Qt::Key_F10);
  }

  AutoOptTool::~AutoOptTool()
  {
    if (m_thread) {
      m_thread->exit();
      m_thread->wait();
      delete m_thread;
      m_thread = 0;
    }
  }

  int AutoOptTool::usefulness() const
  {
    return 10;
  }

  void AutoOptTool::translate(GLWidget *widget, const Eigen::Vector3d &what, const QPoint &from, const QPoint &to) const
  {
    // Translate the selected atoms in the x and y sense of the view
    Vector3d fromPos = widget->camera()->unProject(from, what);
    Vector3d toPos = widget->camera()->unProject(to, what);

    Vector3d atomTranslation = toPos - fromPos;

    if (widget->selectedPrimitives().size())
    {
      foreach(Primitive *p, widget->selectedPrimitives())
      {
        if (p->type() == Primitive::AtomType)
        {
          Atom *a = static_cast<Atom *>(p);
          a->setPos(atomTranslation + *a->pos());
          a->update();
        }
      }
    }

    if (m_clickedAtom)
    {
      m_clickedAtom->setPos(atomTranslation + *m_clickedAtom->pos());
      m_clickedAtom->update();
    }
  }

  QUndoCommand* AutoOptTool::mousePressEvent(GLWidget *widget, QMouseEvent *event)
  {
    m_glwidget = widget;
    m_lastDraggingPosition = event->pos();

    m_leftButtonPressed = (event->buttons() & Qt::LeftButton
        && event->modifiers() == Qt::NoModifier);
    // On the Mac, either use a three-button mouse
    // or hold down the Shift key
    m_midButtonPressed = ((event->buttons() & Qt::MidButton) ||
        (event->buttons() & Qt::LeftButton && event->modifiers()
         & Qt::ShiftModifier));
    // Hold down the Command key (ControlModifier in Qt notation) for right button
    m_rightButtonPressed = ((event->buttons() & Qt::RightButton) ||
        (event->buttons() & Qt::LeftButton && (event->modifiers() == Qt::ControlModifier || event->modifiers() == Qt::MetaModifier)));

    m_clickedAtom = widget->computeClickedAtom(event->pos());
    if(m_clickedAtom != 0 && m_leftButtonPressed && m_running)
    {
      event->accept();
      if (m_forceField->GetConstraints().IsIgnored(m_clickedAtom->index()+1) && !m_ignoredMovable->isChecked() )
        m_clickedAtom = 0;
      else if (m_forceField->GetConstraints().IsFixed(m_clickedAtom->index()+1) && !m_fixedMovable->isChecked() )
        m_clickedAtom = 0;

      if (m_clickedAtom)
      {
        m_forceField->SetFixAtom(m_clickedAtom->index()+1);
      }
    }

    widget->update();
    return 0;
  }

  QUndoCommand* AutoOptTool::mouseReleaseEvent(GLWidget *widget, QMouseEvent *event)
  {
    m_glwidget = widget;

    m_leftButtonPressed = false;
    m_midButtonPressed = false;
    m_rightButtonPressed = false;

    m_clickedAtom = 0;
    m_forceField->UnsetFixAtom();

    widget->update();
    event->accept();
    return 0;
  }

  QUndoCommand* AutoOptTool::mouseMoveEvent(GLWidget *widget, QMouseEvent *event)
  {
    m_glwidget = widget;
    if(!widget->molecule()) {
      return 0;
    }

    // Get the currently selected atoms from the view
    PrimitiveList currentSelection = widget->selectedPrimitives();

    QPoint deltaDragging = event->pos() - m_lastDraggingPosition;

    // Manipulation can be performed in two ways - centred on an individual atom

    if (m_clickedAtom && m_running)
    {
      if (m_leftButtonPressed)
      {
        event->accept();
        // translate the molecule following mouse movement
        Vector3d begin = widget->camera()->project(*m_clickedAtom->pos());
        QPoint point = QPoint(begin.x(), begin.y());
        translate(widget, *m_clickedAtom->pos(), point/*m_lastDraggingPosition*/, event->pos());
      }
    }

    m_lastDraggingPosition = event->pos();
    widget->update();

    return 0;
  }

  QUndoCommand* AutoOptTool::wheelEvent(GLWidget*, QWheelEvent*)
  {
    return 0;
  }

  bool AutoOptTool::paint(GLWidget *widget)
  {
    QPoint labelPos(10, 10);
    //QPoint debugPos(10, 50);
    glColor3f(1.0,1.0,1.0);
    if (m_running) {
      if (m_setupFailed) {
        widget->painter()->drawText(labelPos, tr("AutoOpt: Could not setup force field...."));
      } else {
        double energy = m_forceField->Energy(false);
        widget->painter()->drawText(labelPos,
            tr("AutoOpt: E = %1 %2 (dE = %3)").arg(energy).
            arg(m_forceField->GetUnit().c_str()).
            arg( fabs(m_lastEnergy - energy) ));
        //widget->painter()->drawText(debugPos,
        //    tr("Num Constraints: %1").arg(m_forceField->GetConstraints().Size()));
        m_lastEnergy = energy;
      }
    }

    m_glwidget = widget;
    if(m_leftButtonPressed) {
      if(m_running && m_clickedAtom)
      {
        // Don't highlight the atom on right mouse unless there is a selection
        double renderRadius = widget->radius(m_clickedAtom);
        renderRadius += 0.10;
        glEnable( GL_BLEND );
        widget->painter()->setColor(1.0, 0.3, 0.3, 0.7);
        widget->painter()->drawSphere(m_clickedAtom->pos(), renderRadius);
        glDisable( GL_BLEND );
      }
      else if (m_leftButtonPressed || m_midButtonPressed || m_rightButtonPressed)
      {
        widget->painter()->setColor(1.0, 0.3, 0.3, 0.7);
        widget->painter()->drawSphere(&m_selectedPrimitivesCenter, 0.10);
      }
    }
    else if (m_leftButtonPressed && (!m_clickedAtom
        || m_midButtonPressed || m_rightButtonPressed))
    {
      widget->painter()->setColor(1.0, 0.3, 0.3, 0.7);
      widget->painter()->drawSphere(&m_selectedPrimitivesCenter, 0.10);
    }
    return true;
  }

  QWidget* AutoOptTool::settingsWidget() {
    if(!m_settingsWidget) {
      m_settingsWidget = new QWidget;

      QLabel* labelFF = new QLabel(tr("Force Field:"));
      labelFF->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
      labelFF->setMaximumHeight(15);

      m_comboFF = new QComboBox(m_settingsWidget);
      for (unsigned int i = 0; i < m_forceFieldList.size(); ++i)
        m_comboFF->addItem(m_forceFieldList[i].c_str());

      // find UFF for the default item
      int currentFF = m_comboFF->findText("UFF");
      if (currentFF != -1) // couldn't find it, go for index 0
        m_comboFF->setCurrentIndex(currentFF);

      QHBoxLayout* hbox = new QHBoxLayout;
      hbox->addWidget(m_comboFF);
      hbox->addStretch(1);
      QGridLayout* grid = new QGridLayout;
      grid->addWidget(labelFF, 0, 0, Qt::AlignRight);
      grid->addLayout(hbox, 0, 1);

      QLabel* labelSteps = new QLabel(tr("Steps per Update:"));
      labelSteps->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
      labelSteps->setMaximumHeight(15);

      m_stepsSpinBox = new QSpinBox(m_settingsWidget);
      m_stepsSpinBox->setMinimum(1);
      m_stepsSpinBox->setMaximum(100);
      m_stepsSpinBox->setValue(4);

      hbox = new QHBoxLayout;
      hbox->addWidget(m_stepsSpinBox);
      hbox->addStretch(1);
      grid->addWidget(labelSteps, 1, 0, Qt::AlignRight);
      grid->addLayout(hbox, 1, 1);

      QLabel* labelAlg = new QLabel(tr("Algorithm:"));
      labelAlg->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
      labelAlg->setMaximumHeight(15);

      m_comboAlgorithm = new QComboBox(m_settingsWidget);
      m_comboAlgorithm->addItem(tr("Steepest Descent"));
      m_comboAlgorithm->addItem(tr("Conjugate Gradients"));
      m_comboAlgorithm->addItem(tr("Molecular Dynamics (300K)"));
      m_comboAlgorithm->addItem(tr("Molecular Dynamics (600K)"));
      m_comboAlgorithm->addItem(tr("Molecular Dynamics (900K)"));

      m_buttonStartStop = new QPushButton(tr("Start"), m_settingsWidget);

      m_fixedMovable = new QCheckBox(tr("Fixed atoms are movable"), m_settingsWidget);
      m_ignoredMovable = new QCheckBox(tr("Ignored atoms are movable"), m_settingsWidget);

      QVBoxLayout* layout = new QVBoxLayout();
      layout->addLayout(grid); // force field, steps and labels

      layout->addWidget(labelAlg);
      layout->addWidget(m_comboAlgorithm);
      layout->addWidget(m_fixedMovable);
      layout->addWidget(m_ignoredMovable);
      layout->addWidget(m_buttonStartStop);
      layout->addStretch(1);
      m_settingsWidget->setLayout(layout);

      // Connect the start/stop button
      connect(m_buttonStartStop, SIGNAL(clicked()),
          this, SLOT(toggle()));

      connect(m_settingsWidget, SIGNAL(destroyed()),
          this, SLOT(settingsWidgetDestroyed()));

      // Check the force field is there, if not disable the start button
      if (!m_forceField)
        m_buttonStartStop->setEnabled(false);
    }

    return m_settingsWidget;
  }

  void AutoOptTool::settingsWidgetDestroyed()
  {
    m_settingsWidget = 0;
  }

  void AutoOptTool::toggle()
  {
    // Toggle the timer on and off
    if (m_running) {
      disable();
    } else {
      enable();
    }
  }

  void AutoOptTool::enable()
  {
    // If the force field is false we have nothing and so should return
    if (!m_forceField)
      return;

    if(!m_running)
    {
      m_thread->setup(m_glwidget->molecule(), m_forceField,
                      m_comboAlgorithm->currentIndex(),
                      m_stepsSpinBox->value());
      m_thread->start();
      m_running = true;
      m_buttonStartStop->setText(tr("Stop"));
      QUndoStack *stack = m_glwidget->undoStack();
      AutoOptCommand *cmd = new AutoOptCommand(m_glwidget->molecule(),this,0);
      if(stack && cmd)
      {
        stack->push(cmd);
      }
      else
      {
        delete cmd;
      }
    }
  }

  void AutoOptTool::disable()
  {
    if(m_running)
    {
      if(m_timerId)
      {
        killTimer(m_timerId);
        m_timerId = 0;
      }
      m_thread->quit();
      m_running = false;
      m_setupFailed = false;
      m_buttonStartStop->setText(tr("Start"));

      m_glwidget->update(); // redraw AutoOpt label

      m_clickedAtom = 0;
      m_forceField->UnsetFixAtom();
      m_leftButtonPressed = false;
      m_midButtonPressed = false;
      m_rightButtonPressed = false;
    }
  }

  void AutoOptTool::timerEvent(QTimerEvent*)
  {
    if(m_block || m_glwidget->molecule()->numAtoms() < 2)
      return;
    else
      m_block = true;

    m_forceField = OBForceField::FindForceField(m_forceFieldList[m_comboFF->currentIndex()]);

    // Check that we can find our force field - if not return
    if (!m_forceField)
    {
      emit setupFailed();
      return;
    }

    m_thread->setup(m_glwidget->molecule(), m_forceField,
                    m_comboAlgorithm->currentIndex(),
                    m_stepsSpinBox->value());
    m_thread->update();
  }

  void AutoOptTool::finished(bool calculated)
  {
    if (m_running && calculated)
    {
      OBMol mol = m_glwidget->molecule()->OBMol();
      m_forceField->GetCoordinates( mol );
      QList<Atom*> atoms = m_glwidget->molecule()->atoms();
      double *coordPtr = mol.GetCoordinates();
      foreach(Atom* atom, atoms) {
        atom->setPos(Eigen::Vector3d(coordPtr));
        coordPtr += 3;
      }

      if(m_clickedAtom && m_leftButtonPressed)
      {
        Vector3d begin = m_glwidget->camera()->project(*m_clickedAtom->pos());
        QPoint point = QPoint(begin.x(), begin.y());
        translate(m_glwidget, *m_clickedAtom->pos(), point, m_lastDraggingPosition);
      }
    }

    m_glwidget->molecule()->update();
    m_glwidget->update();
    m_block = false;
  }

  void AutoOptTool::setupDone()
  {
    if(!m_timerId)
    {
      m_timerId = startTimer(50);
    }
  }

  void AutoOptTool::setupFailed()
  {
    m_setupFailed = true;
  }

  void AutoOptTool::setupSucces()
  {
    m_setupFailed = false;
  }

  AutoOptThread::AutoOptThread(QObject*)
  {
    m_stop = false;
    m_velocities = false;
  }

  void AutoOptThread::setup(Molecule *molecule, OpenBabel::OBForceField* forceField,
        int algorithm, int steps)
  {
    //cout << "start AutoOptThread::setup()" << endl;
    m_mutex.lock();
    m_molecule = molecule;
    m_forceField = forceField;
    m_algorithm = algorithm;
    m_steps = steps;
    m_stop = false;
    m_velocities = false;
    m_mutex.unlock();
    emit setupDone();
    //cout << "stop AutoOptThread::setup()" << endl;
  }


  void AutoOptThread::run()
  {
    exec();
  }

  void AutoOptThread::update()
  {
    // If the force field is false we have nothing and so should return
    if (!m_forceField)
      return;

     m_mutex.lock();

     m_forceField->SetLogFile(NULL);
     m_forceField->SetLogLevel(OBFF_LOGLVL_NONE);

     OBMol mol = m_molecule->OBMol();
     if ( !m_forceField->Setup( mol ) ) {
       m_stop = true;
       emit setupFailed();
       emit finished(false);
       m_mutex.unlock();
       return;
     } else {
       emit setupSucces();
     }

     m_forceField->SetConformers( mol );

     switch(m_algorithm) {
       case 0:
         m_forceField->SteepestDescent(m_steps);
         break;
       case 1:
         m_forceField->ConjugateGradients(m_steps);
         break;
       case 2:
         m_forceField->MolecularDynamicsTakeNSteps(m_steps, 300, 0.001);
         break;
       case 3:
         m_forceField->MolecularDynamicsTakeNSteps(m_steps, 600, 0.001);
         break;
       case 4:
         m_forceField->MolecularDynamicsTakeNSteps(m_steps, 900, 0.001);
         break;
     }

     m_mutex.unlock();

     emit finished(m_stop ? false : true);
  }

  void AutoOptThread::stop()
  {
    m_stop = true;
  }

  AutoOptCommand::AutoOptCommand(Molecule *molecule, AutoOptTool *tool,
      QUndoCommand *parent) : QUndoCommand(parent), m_molecule(0)
  {
    // Store the original molecule before any modifications are made
    setText(QObject::tr("AutoOpt Molecule"));
    m_moleculeCopy = *molecule;
    m_molecule = molecule;
    m_tool = tool;
  }

  void AutoOptCommand::redo()
  {
    m_moleculeCopy = *m_molecule;
  }

  void AutoOptCommand::undo()
  {
    if(m_tool)
    {
      m_tool->disable();
    }
    *m_molecule = m_moleculeCopy;
  }

  bool AutoOptCommand::mergeWith (const QUndoCommand *)
  {
    // Just return true to repeated calls - we have stored the original molecule
    return true;
  }

  int AutoOptCommand::id() const
  {
    return 1311387;
  }

  void AutoOptTool::writeSettings(QSettings &settings) const
  {
    Tool::writeSettings(settings);
    settings.setValue("forceField", m_comboFF->currentIndex());
    settings.setValue("algorithm", m_comboAlgorithm->currentIndex());
    settings.setValue("steps", m_stepsSpinBox->value());
    settings.setValue("fixedMovable", m_fixedMovable->checkState());
    settings.setValue("ignoredMovable", m_ignoredMovable->checkState());
  }

  void AutoOptTool::readSettings(QSettings &settings)
  {
    Tool::readSettings(settings);
    if(m_comboFF) {
      int currentFF = settings.value("forceField", -1).toInt();
      if (currentFF == -1) {
        // haven't set a default
        // find UFF for default
        currentFF = m_comboFF->findText("UFF");
        if (currentFF == -1) // couldn't find it, go for index 0
          currentFF = 0;
      }
      m_comboFF->setCurrentIndex(currentFF);
    }
    if(m_comboAlgorithm) {
      m_comboAlgorithm->setCurrentIndex(settings.value("algorithm", 0).toInt());
    }
    if(m_stepsSpinBox) {
      m_stepsSpinBox->setValue(settings.value("steps", 4).toInt());
    }
    if(m_fixedMovable) {
      m_fixedMovable->setCheckState((Qt::CheckState)settings.value("fixedMovable", 2).toInt());
    }
    if(m_ignoredMovable) {
      m_ignoredMovable->setCheckState((Qt::CheckState)settings.value("ignoredMovable", 2).toInt());
    }
  }

}

#include "autoopttool.moc"

Q_EXPORT_PLUGIN2(autoopttool, Avogadro::AutoOptToolFactory)
