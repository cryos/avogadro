/**********************************************************************
  PythonTool - PythonTool Tool for Avogadro

  Copyright (C) 2008 Tim Vandermeersch

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.sourceforge.net/>

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

#ifndef PYTHONTOOL_H
#define PYTHONTOOL_H

#include <avogadro/global.h>
#include <avogadro/glwidget.h>
#include <avogadro/tool.h>
#include <avogadro/pythoninterpreter.h>
#include <avogadro/pythonscript.h>

#include "ui_pythonsettingswidget.h"

#include <QGLWidget>
#include <QObject>
#include <QAction>

namespace Avogadro {
  
  class PythonSettingsWidget;

  class PythonTool : public Tool
  {
    Q_OBJECT

    public:
      //! Constructor
      PythonTool(QObject *parent = 0);
      //! Destructor
      virtual ~PythonTool();

      //! \name Description methods
      //@{
      //! Tool Name (i.e. Python)
      virtual QString name() const { return(tr("Python")); }
      //! Tool Description (i.e. Python molecules)
      virtual QString description() const { return(tr("Python tools")); }
      //@}

      //! \name Tool Methods
      //@{
      //! \brief Callback methods for ui.actions on the canvas.
      /*!
        */
      QUndoCommand* mouseEvent(const QString &what, GLWidget *widget, QMouseEvent *event);
      QUndoCommand* mousePressEvent(GLWidget *widget, QMouseEvent *event);
      QUndoCommand* mouseReleaseEvent(GLWidget *widget, QMouseEvent *event);
      QUndoCommand* mouseMoveEvent(GLWidget *widget, QMouseEvent *event);
      QUndoCommand* wheelEvent(GLWidget *widget, QWheelEvent *event);

      bool paint(GLWidget *widget);

      QWidget *settingsWidget();

    private:
      PythonSettingsWidget *m_settingsWidget;
    
      QList<PythonScript> m_scripts;
      PythonInterpreter m_interpreter;

      int m_scriptIndex;
      boost::python::object m_instance;
 
      void loadScripts(QDir dir);

    private Q_SLOTS:
      void settingsWidgetDestroyed();
      void setScriptIndex(int index);
  };

  class PythonSettingsWidget : public QWidget, public Ui::PythonSettingsWidget
  {
  public:
    PythonSettingsWidget(QWidget *parent=0) : QWidget(parent) {
      setupUi(this);
    }
  };


  class PythonToolFactory : public QObject, public PluginFactory
  {
    Q_OBJECT
    Q_INTERFACES(Avogadro::PluginFactory)
    AVOGADRO_TOOL_FACTORY(PythonTool, tr("Python Tool"), tr("Python tools."))
  };

} // end namespace Avogadro

#endif
