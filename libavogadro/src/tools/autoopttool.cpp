/**********************************************************************
  AutoOptTool - Automatic Optimisation Tool for Avogadro

  Copyright (C) 2007 by Marcus D. Hanwell
  Copyright (C) 2007 by Geoffrey R. Hutchison
  Copyright (C) 2007 by Benoit Jacob

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
#include "navigate.h"
#include <avogadro/primitive.h>
#include <avogadro/color.h>
#include <avogadro/glwidget.h>
#include <avogadro/camera.h>
#include <avogadro/toolgroup.h>

#include <openbabel/obiter.h>
#include <openbabel/mol.h>

#include <QtPlugin>

using namespace std;
using namespace OpenBabel;
using namespace Avogadro;
using namespace Eigen;

AutoOptTool::AutoOptTool(QObject *parent) : Tool(parent), m_clickedAtom(0),
  m_leftButtonPressed(false), m_midButtonPressed(false), m_rightButtonPressed(false),
  m_running(false), m_block(false), m_timerId(0) ,m_toolGroup(0), m_settingsWidget(0)
{
  QAction *action = activateAction();
  action->setIcon(QIcon(QString::fromUtf8(":/autoopttool/autoopttool.png")));
  action->setToolTip(tr("AutoOpt Tool\n\n"
  		"Navigation Functions when Clicking in empty space.\n"
        "Left Mouse: Rotate Space\n"
        "Middle Mouse: Zoom Space\n"
        "Right Mouse: Move Space\n\n"
        "Extra Function when running\n"
        "Left Mouse: Click and drag atoms to move them"));
  m_forceField = OBForceField::FindForceField( "Ghemical" );
  connect(action,SIGNAL(toggled(bool)),this,SLOT(toggled(bool)));
  //action->setShortcut(Qt::Key_F10);
}

AutoOptTool::~AutoOptTool()
{
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

  MatrixP3d atomTranslation;
  atomTranslation.loadTranslation(toPos - fromPos);

  if (widget->selectedPrimitives().size())
  {
    foreach(Primitive *p, widget->selectedPrimitives())
    {
      if (p->type() == Primitive::AtomType)
      {
        Atom *a = static_cast<Atom *>(p);
        widget->molecule()->BeginModify();
        a->setPos(atomTranslation * a->pos());
        widget->molecule()->EndModify();
        a->update();
      }
    }
  }
  if (m_clickedAtom)
  {
    widget->molecule()->BeginModify();
    m_clickedAtom->setPos(atomTranslation * m_clickedAtom->pos());
    widget->molecule()->EndModify();
    m_clickedAtom->update();
  }
}

QUndoCommand* AutoOptTool::mousePress(GLWidget *widget, const QMouseEvent *event)
{
  m_glwidget = widget;
  m_lastDraggingPosition = event->pos();
  
#ifdef Q_WS_MAC
  m_leftButtonPressed = (event->buttons() & Qt::LeftButton
                         && event->modifiers() == Qt::NoModifier);
  // On the Mac, either use a three-button mouse
  // or hold down the Option key (AltModifier in Qt notation)
  m_midButtonPressed = ((event->buttons() & Qt::MidButton) ||
                        (event->buttons() & Qt::LeftButton && event->modifiers()
                        & Qt::AltModifier));
  // Hold down the Command key (ControlModifier in Qt notation) for right button
  m_rightButtonPressed = ((event->buttons() & Qt::RightButton) ||
                          (event->buttons() & Qt::LeftButton && event->modifiers()
                          & Qt::ControlModifier));
#else
  m_leftButtonPressed = (event->buttons() & Qt::LeftButton);
  m_midButtonPressed = (event->buttons() & Qt::MidButton);
  m_rightButtonPressed = (event->buttons() & Qt::RightButton);
#endif

  m_clickedAtom = widget->computeClickedAtom(event->pos());

  widget->update();
  return 0;
}

QUndoCommand* AutoOptTool::mouseRelease(GLWidget *widget, const QMouseEvent*)
{
  m_glwidget = widget;
  m_leftButtonPressed = false;
  m_midButtonPressed = false;
  m_rightButtonPressed = false;
  m_clickedAtom = 0;

  widget->update();
  return 0;
}

QUndoCommand* AutoOptTool::mouseMove(GLWidget *widget, const QMouseEvent *event)
{
  m_glwidget = widget;
  if(!widget->molecule()) {
    return 0;
  }
  //m_undo = new MoveAtomCommand(widget->molecule());

  // Get the currently selected atoms from the view
  QList<Primitive *> currentSelection = widget->selectedPrimitives();

  QPoint deltaDragging = event->pos() - m_lastDraggingPosition;

  // Manipulation can be performed in two ways - centred on an individual atom

  if (m_clickedAtom && m_running)
  {
    if (m_leftButtonPressed)
    {
      // translate the molecule following mouse movement
      Vector3d begin = widget->camera()->project(m_clickedAtom->pos());
      QPoint point = QPoint(begin.x(), begin.y());
      translate(widget, m_clickedAtom->pos(), point/*m_lastDraggingPosition*/, event->pos());
    }
    else if (m_midButtonPressed)
    {
      // Perform the rotation
      Navigate::tilt(widget, widget->center(), deltaDragging.x());

      // Perform the zoom toward molecule center
      Navigate::zoom(widget, widget->center(), deltaDragging.y());
  	}
  	else if (m_rightButtonPressed)
  	{
  	  Navigate::translate(widget, widget->center(), m_lastDraggingPosition, event->pos());
  	}
  }
  else
  {
    if (m_leftButtonPressed)
    {
      // rotation around the center of the molecule
      Navigate::rotate(widget, widget->center(), deltaDragging.x(), deltaDragging.y());
    }
    else if (m_midButtonPressed)
    {
      // Perform the rotation
      Navigate::tilt(widget, widget->center(), deltaDragging.x());

      // Perform the zoom toward molecule center
      Navigate::zoom(widget, widget->center(), deltaDragging.y());
  	}
  	else if (m_rightButtonPressed)
  	{
  	  Navigate::translate(widget, widget->center(), m_lastDraggingPosition, event->pos());
  	}
  }

  m_lastDraggingPosition = event->pos();
  widget->update();

  return 0;
}

QUndoCommand* AutoOptTool::wheel(GLWidget* widget, const QWheelEvent* event)
{
  m_glwidget = widget;
  Primitive *clickedPrim = widget->computeClickedPrimitive(event->pos());

  if (clickedPrim && clickedPrim->type() == Primitive::AtomType)
  {
    Atom *clickedAtom = (Atom*)clickedPrim;
    // Perform the zoom toward clicked atom
    Navigate::zoom(widget, clickedAtom->pos(), - MOUSE_WHEEL_SPEED * event->delta());
  }
  else if (clickedPrim && clickedPrim->type() == Primitive::BondType)
  {
    Bond *clickedBond = (Bond*)clickedPrim;

    Atom *begin = static_cast<Atom *>(clickedBond->GetBeginAtom());
    Atom *end = static_cast<Atom *>(clickedBond->GetEndAtom());

    Vector3d btoe = end->pos() - begin->pos();
    double newLen = btoe.norm() / 2;
    btoe = btoe / btoe.norm();

    Vector3d mid = begin->pos() + btoe * newLen;

    // Perform the zoom toward the centre of a clicked bond
    Navigate::zoom(widget, mid, - MOUSE_WHEEL_SPEED * event->delta());
  }
  else {
    // Perform the zoom toward molecule center
    Navigate::zoom(widget, widget->center(), - MOUSE_WHEEL_SPEED * event->delta());
  }

  widget->update();
  return 0;
}

bool AutoOptTool::paint(GLWidget *widget)
{
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
      widget->painter()->drawSphere(m_selectedPrimitivesCenter, 0.10);
    }
  }
  else if (m_leftButtonPressed && !m_clickedAtom 
             || m_midButtonPressed || m_rightButtonPressed)
  {  
    widget->painter()->setColor(1.0, 0.3, 0.3, 0.7);
    widget->painter()->drawSphere(m_selectedPrimitivesCenter, 0.10);
  }
  return true;
}

QWidget* AutoOptTool::settingsWidget() {
  if(!m_settingsWidget) {
    m_settingsWidget = new QWidget;
    
    ui.setupUi(m_settingsWidget);
    
    // Connect the start/stop button
    connect(ui.m_buttonStartStop, SIGNAL(clicked()),
        this, SLOT(toggle()));

    connect(m_settingsWidget, SIGNAL(destroyed()),
        this, SLOT(settingsWidgetDestroyed()));
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
  if(!m_running)
  {
    if(!m_timerId)
    {
      m_timerId = startTimer(40);
    }
    m_running = true;
    ui.m_buttonStartStop->setText(tr("Stop"));
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
    m_running = false;
    ui.m_buttonStartStop->setText(tr("Start"));
    m_clickedAtom = 0;
    m_leftButtonPressed = false;
    m_midButtonPressed = false;
    m_rightButtonPressed = false;
  }
}

void AutoOptTool::timerEvent(QTimerEvent*)
{
	if(m_block)
	{
		return;
	}
	else
	{
		m_block = true;
	}

    if ( !m_forceField->Setup( *m_glwidget->molecule() ) ) {
      qWarning() << "GhemicalCommand: Could not set up force field on " << m_glwidget->molecule();
      m_block = false;
      return;
    }
	if(ui.AlgorithmComboBox->currentIndex() == 0)
	{
      m_forceField->SteepestDescent(ui.StepsSpinBox->value(),pow(10.0, -ui.ConvergenceSpinBox->value() ), ui.GradientsComboBox == 0 ? OBFF_NUMERICAL_GRADIENT : OBFF_ANALYTICAL_GRADIENT);
	}
	else if(ui.AlgorithmComboBox->currentIndex() == 1)
	{
      m_forceField->ConjugateGradients(ui.StepsSpinBox->value(),pow(10.0, -ui.ConvergenceSpinBox->value() ), ui.GradientsComboBox == 0 ? OBFF_NUMERICAL_GRADIENT : OBFF_ANALYTICAL_GRADIENT);
    }
    if (m_running)
    {
      m_forceField->UpdateCoordinates( *m_glwidget->molecule() );
      if(m_clickedAtom && m_leftButtonPressed)
      {
        Vector3d begin = m_glwidget->camera()->project(m_clickedAtom->pos());
        QPoint point = QPoint(begin.x(), begin.y());
        translate(m_glwidget, m_clickedAtom->pos(), point, m_lastDraggingPosition);
      }
      m_glwidget->molecule()->update();
    }
    
	m_block = false;
}

void AutoOptTool::toggled(bool checked)
{
  if(!checked)
  {
    disable();
  }
}

AutoOptCommand::AutoOptCommand(Molecule *molecule, AutoOptTool *tool, QUndoCommand *parent) : QUndoCommand(parent), m_molecule(0)
{
  // Store the original molecule before any modifications are made
  setText(QObject::tr("AutoOpt Molecule"));
  m_moleculeCopy = *molecule;
  m_molecule = molecule;
  m_tool = tool;
}

void AutoOptCommand::redo()
{
  Molecule newMolecule = *m_molecule;
  *m_molecule = m_moleculeCopy;
  m_moleculeCopy = newMolecule;
  QUndoCommand::redo();
}

void AutoOptCommand::undo()
{
  if(m_tool)
  {
  	m_tool->disable();
  }
  Molecule newMolecule = *m_molecule;
  *m_molecule = m_moleculeCopy;
  m_moleculeCopy = newMolecule;
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


#include "autoopttool.moc"

Q_EXPORT_PLUGIN2(autoopttool, AutoOptToolFactory)
