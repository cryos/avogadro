/**********************************************************************
  PythonExtension - PythonExtension

  Copyright (C) 2008 by Donald Ephraim Curtis
  Copyright (C) 2008,2009 by Tim Vandermeersch

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.openmolecules.net/>

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
 ***********************************************************************/

#include "pythonextension_p.h"
#include "pythonscript.h"
#include "pythonthread_p.h"

#include <avogadro/molecule.h>

#include <QApplication>
#include <QUndoCommand>
#include <QAction>
#include <QDockWidget>
#include <QFileInfo>
#include <QDebug>

using namespace std;
using namespace boost::python;

namespace Avogadro
{

  PythonExtension::PythonExtension(PythonScript *script, QObject *parent) :
      Extension(parent), PythonPlugin(script), m_dockWidget(0)
  {
    loadScript(script);
  }

  PythonExtension::~PythonExtension()
  {
    PythonThread pt;
    if (m_dockWidget)
      m_dockWidget->deleteLater();
  }

  QString PythonExtension::name() const
  {
    const QString name = PythonPlugin::name();
    if(!name.isNull())
      return name;
    else
      return tr("Unknown Python Extension");
  }

  QString PythonExtension::description() const
  {
    const QString desc = PythonPlugin::description();
    if(!desc.isNull())
      return desc;
    else
      return tr("N/A");
  }

  QList<QAction *> PythonExtension::actions() const
  {
    QList<QAction*> actions;

    if (!m_script)
      return actions;
    
    PythonThread pt;

    try {
      prepareToCatchError();
      actions = extract< QList<QAction*> >(m_instance.attr("actions")());
    } catch (error_already_set const &) {
      catchError();
    }

    // this will make the MainWindow call performAction on this extension
    foreach (QAction *action, actions)
      action->setParent( (PythonExtension*)this );

    return actions;
  }

  // allows us to set the intended menu path for each action
  QString PythonExtension::menuPath(QAction *action) const
  {
    if (!m_script || !hasAttrString("menuPath"))
      return tr("&Scripts");

    try {
      prepareToCatchError();

      boost::python::return_by_value::apply<QAction*>::type qconverter;
      PyObject *qobj = qconverter(action);
      object real_qobj = object(handle<>(qobj));

      return extract<QString>(m_instance.attr("menuPath")(real_qobj));
    } catch(error_already_set const &) {
      catchError();
    }

    return tr("&Scripts");
  }

  class PythonCommand : public QUndoCommand
  {
    public:
      PythonCommand(QUndoCommand *command) : m_command(command)
      {
        PythonThread pt;
        setText(m_command->text());
      }

      ~PythonCommand()
      {
        PythonThread pt;
        delete m_command;
      }

      void redo()
      {
        PythonThread pt;
        try {
          prepareToCatchError();
          m_command->redo();
          catchError();
        } catch(error_already_set const &) {
          catchError();
        }
      }

      void undo()
      {
        PythonThread pt;
        try {
          prepareToCatchError();
          m_command->undo();
          catchError();
        } catch(error_already_set const &) {
          catchError();
        }
      }
    private:
      QUndoCommand *m_command;
  };

  QUndoCommand* PythonExtension::performAction( QAction *action, GLWidget *widget )
  {
    if (!m_script)
      return 0;

    PythonThread pt;
    // Let's just catch the exception and print the error...
    //if (!PyObject_HasAttrString(m_instance.ptr(), "performAction"))
    //  return 0;

    try {
      prepareToCatchError();

      boost::python::reference_existing_object::apply<GLWidget*>::type converter;
      PyObject *obj = converter(widget);
      object real_obj = object(handle<>(obj));

      boost::python::return_by_value::apply<QAction*>::type qconverter;
      PyObject *qobj = qconverter(action);
      object real_qobj = object(handle<>(qobj));

      object pyObj(m_instance.attr("performAction")(real_qobj, real_obj)); // new reference
      QUndoCommand *command = extract<QUndoCommand*>(pyObj);
      if (!command)
        return 0;

      return new PythonCommand(command);
    } catch(error_already_set const &) {
      catchError();
    }

    return 0;
  }

  QDockWidget* PythonExtension::dockWidget()
  {
    if (!m_script)
      return 0; // nothing we can do

    PythonThread pt;

    if(!m_dockWidget)
    {
      if (hasAttrString("dockWidget")) {
        try {
          prepareToCatchError();
          m_dockWidget = extract<QDockWidget*>(m_instance.attr("dockWidget")());
          m_dockWidget->setObjectName(m_dockWidget->windowTitle());
        } catch (error_already_set const &) {
          m_dockWidget = 0;
          catchError();
        }
      }

      if (m_dockWidget)
        connect(m_dockWidget, SIGNAL(destroyed()), this, SLOT(dockWidgetDestroyed()));
    }

    return m_dockWidget;
  }

  void PythonExtension::dockWidgetDestroyed()
  {
    m_dockWidget = 0;
  }
      
  bool PythonExtension::paint(GLWidget *widget)
  {
    PythonThread pt;
    if (!hasAttrString("paint"))
      return false;

    try {
      prepareToCatchError();
      boost::python::reference_existing_object::apply<GLWidget*>::type converter;
      PyObject *obj = converter(widget);
      object real_obj = object(handle<>(obj));

      m_instance.attr("paint")(real_obj);
    } catch(error_already_set const &) {
      catchError();
    }

    return true;
  }

  void PythonExtension::readSettings(QSettings &settings)
  {
    Extension::readSettings(settings);
    PythonPlugin::readSettings(settings);
  }

  void PythonExtension::writeSettings(QSettings &settings) const
  {
    Extension::writeSettings(settings);
    PythonPlugin::writeSettings(settings);
  }

  void PythonExtension::loadScript(PythonScript *script)
  {
    PythonThread pt;

    try {
      prepareToCatchError();
      m_instance = script->module().attr("Extension")();
    } catch (error_already_set const &) {
      catchError();
      return;
    }

    // connect signal(s)
    if (hasAttrString("__pyqtSignals__")) {
      QObject *obj = extract<QObject*>(m_instance);
      connect(obj, SIGNAL(message(const QString&)), this, SIGNAL(message(const QString&)));
    }
  }

}
