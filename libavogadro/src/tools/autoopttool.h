/**********************************************************************
  AutoOptTool - Automatic Optimization Tool for Avogadro

  Copyright (C) 2007,2008 by Marcus D. Hanwell
  Copyright (C) 2007 by Geoffrey R. Hutchison
  Copyright (C) 2007 by Benoit Jacob
  Copyright (C) 2008 by Tim Vandermeersch

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

#ifndef AUTOOPTTOOL_H
#define AUTOOPTTOOL_H

#include <avogadro/glwidget.h>
#include <avogadro/tool.h>
#include <avogadro/molecule.h>

#include <openbabel/mol.h>
#include <openbabel/forcefield.h>

#include <QtCore/QMutex>
#include <QtCore/QThread>
#include <QtGui/QAction>
#include <QtGui/QPushButton>
#include <QtGui/QComboBox>
#include <QtGui/QCheckBox>
#include <QtGui/QSpinBox>
#include <QtGui/QUndoStack>

namespace Avogadro {

  class AutoOptThread : public QThread
  {
    Q_OBJECT

    public:
      AutoOptThread(QObject *parent=0);

      void setup(Molecule *molecule, OpenBabel::OBForceField* forceField,
                 int algorithm, /* int convergence, */ int steps);

      void run();
      void update();

    Q_SIGNALS:
      void finished(bool calculated);
      void setupDone();
      void setupFailed();
      void setupSucces();

    public Q_SLOTS:
      void stop();

    private:
      Molecule *m_molecule;
      OpenBabel::OBForceField * m_forceField;
      bool m_velocities;
      int m_algorithm;
      //double m_convergence;
      int m_steps;
      bool m_stop;
      QMutex m_mutex;
  };

  /**
   * @class AutoOptTool
   * @brief Automatic Optimization Tool
   * @author Marcus D. Hanwell
   *
   * This tool enables the manipulation of the position of
   * the selected atoms while the optimiser is running.
   */
  class AutoOptTool : public Tool
  {
    Q_OBJECT
      AVOGADRO_TOOL("AutoOptimization", tr("AutoOptimization"),
                    tr("Automatic optimization of molecular geometry"),
                    tr("AutoOptimization Settings"))

    public:
      //! Constructor
      AutoOptTool(QObject *parent = 0);
      //! Deconstructor
      virtual ~AutoOptTool();

      //! \name Tool Methods
      //@{
      //! \brief Callback methods for ui.actions on the canvas.
      /*!
      */
      virtual QUndoCommand* mousePressEvent(GLWidget *widget, QMouseEvent *event);
      virtual QUndoCommand* mouseReleaseEvent(GLWidget *widget, QMouseEvent *event);
      virtual QUndoCommand* mouseMoveEvent(GLWidget *widget, QMouseEvent *event);
      virtual QUndoCommand* wheelEvent(GLWidget *widget, QWheelEvent *event);

      virtual int usefulness() const;

      virtual bool paint(GLWidget *widget);

      virtual QWidget* settingsWidget();
      /**
       * Write the tool settings so that they can be saved between sessions.
       */
      virtual void writeSettings(QSettings &settings) const;

      /**
       * Read in the settings that have been saved for the tool instance.
       */
      virtual void readSettings(QSettings &settings);


    public Q_SLOTS:
      void finished(bool calculated);
      void setupDone();
      void setupFailed();
      void setupSucces();
      void toggle();
      void enable();
      void disable();
      void abort();

    protected:
      GLWidget *                m_glwidget;
      Atom *                    m_clickedAtom;
      bool                      m_leftButtonPressed;  // rotation
      bool                      m_midButtonPressed;   // scale / zoom
      bool                      m_rightButtonPressed; // translation
      bool                      m_running;
      bool                      m_block;
      bool                      m_setupFailed;
      int                       m_timerId;
      ToolGroup *               m_toolGroup;
      QWidget*                  m_settingsWidget;
      Eigen::Vector3d           m_selectedPrimitivesCenter;    // centroid of selected atoms
      OpenBabel::OBForceField*  m_forceField;
      AutoOptThread *           m_thread;

      std::vector<std::string>  m_forceFieldList;

      QComboBox*                m_comboFF;
      QComboBox*                m_comboAlgorithm;
      //QSpinBox*                 m_convergenceSpinBox;
      QSpinBox*                 m_stepsSpinBox;
      QPushButton*              m_buttonStartStop;
      QCheckBox*                m_fixedMovable;
      QCheckBox*                m_ignoredMovable;

      QPoint                    m_lastDraggingPosition;
      double                    m_lastEnergy;

      void timerEvent(QTimerEvent* event);

      void translate(GLWidget *widget, const Eigen::Vector3d &what, const QPoint &from, const QPoint &to) const;

    private Q_SLOTS:
      void settingsWidgetDestroyed();
  };

  class AutoOptCommand : public QUndoCommand
  {
    public:
      AutoOptCommand(Molecule *molecule, AutoOptTool *tool, QUndoCommand *parent = 0);

      void redo();
      void undo();
      bool mergeWith ( const QUndoCommand * command );
      int id() const;

    private:
      Molecule m_moleculeCopy;
      Molecule *m_molecule;
      AutoOptTool *m_tool;
      bool undone;
  };

  class AutoOptToolFactory : public QObject, public PluginFactory
  {
    Q_OBJECT
    Q_INTERFACES(Avogadro::PluginFactory)
    AVOGADRO_TOOL_FACTORY(AutoOptTool)
  };

} // end namespace Avogadro

#endif
