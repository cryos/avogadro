/**********************************************************************
  AutoOptTool - Automatic Optimization Tool for Avogadro

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

#ifndef __AUTOOPTTOOL_H
#define __AUTOOPTTOOL_H

#include <avogadro/glwidget.h>
#include <avogadro/tool.h>

#include <openbabel/mol.h>
#include <openbabel/forcefield.h>

#include <QGLWidget>
#include <QObject>
#include <QStringList>
#include <QImage>
#include <QAction>
#include <QWidget>
#include <QPushButton>
#include <QComboBox>
#include <QCheckBox>
#include <QSpinBox>
#include <QUndoStack>

namespace Avogadro {

  class AutoOptThread : public QThread
  {
    Q_OBJECT

    public:
    AutoOptThread(QObject *parent=0);

    void setup(Molecule *molecule, OpenBabel::OBForceField* forceField, 
        int algorithm, int gradients, int convergence);

      void run();
      void update();

    Q_SIGNALS:
      void finished(bool calculated);
      void setupFailed();
      void setupSucces();

    public Q_SLOTS:
      void stop();

    private:
      Molecule *m_molecule;
      OpenBabel::OBForceField * m_forceField;
      bool m_velocities;
      int m_algorithm;
      int m_gradients;
      int m_convergence;

      bool m_stop;
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

    public:
      //! Constructor
      AutoOptTool(QObject *parent = 0);
      //! Deconstructor
      virtual ~AutoOptTool();

      //! \name Description methods
      //@{
      //! Tool Name (ie Draw)
      virtual QString name() const { return(tr("AutoOptimization")); }
      //! Tool Description (ie. Draws atoms and bonds)
      virtual QString description() const { return(tr("Automatic Optimization Tool")); }
      //@}

      //! \name Tool Methods
      //@{
      //! \brief Callback methods for ui.actions on the canvas.
      /*!
      */
      virtual QUndoCommand* mousePress(GLWidget *widget, const QMouseEvent *event);
      virtual QUndoCommand* mouseRelease(GLWidget *widget, const QMouseEvent *event);
      virtual QUndoCommand* mouseMove(GLWidget *widget, const QMouseEvent *event);
      virtual QUndoCommand* wheel(GLWidget *widget, const QWheelEvent *event);

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
      void setupFailed();
      void setupSucces();
      void toggle();
      void enable();
      void disable();

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
      int 			m_numConstraints;
      AutoOptThread *           m_thread;

      std::vector<std::string>  m_forceFieldList;

      QComboBox*                m_comboFF;
      QComboBox*                m_comboAlgorithm;
      QSpinBox*                 m_convergenceSpinBox;
      QPushButton*              m_buttonStartStop;
      QCheckBox*                m_fixedMovable;
      QCheckBox*                m_ignoredMovable;

      QPoint                    m_lastDraggingPosition;

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

  class AutoOptToolFactory : public QObject, public ToolFactory
    {
      Q_OBJECT
      Q_INTERFACES(Avogadro::ToolFactory)

      public:
        Tool *createInstance(QObject *parent = 0) { return new AutoOptTool(parent); }
    };

} // end namespace Avogadro

#endif
