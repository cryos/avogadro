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

#include "pythonextension.h"
#include "pythoninterpreter.h"
#include "pythonscript.h"


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

  class PythonExtensionPrivate
  {
    public:
      PythonExtensionPrivate() : script(0), dockWidget(0)
      {}

      PythonInterpreter      interpreter;
      PythonScript          *script;
      boost::python::object  instance;
      QDockWidget           *dockWidget;
      QString                identifier;
  };

  PythonExtension::PythonExtension(QObject *parent, const QString &filename) :
      Extension(parent), d(new PythonExtensionPrivate)
  {
    loadScript(filename);
  }

  PythonExtension::~PythonExtension()
  {
    if (d->script)
      delete d->script;
    if (d->dockWidget)
      d->dockWidget->deleteLater();
    delete d;
  }

  QString PythonExtension::identifier() const
  {
    return d->identifier;
  }

  QString PythonExtension::name() const
  {
    if (!PyObject_HasAttrString(d->instance.ptr(), "name"))
      return tr("Unknown Python Extension");

    try {
       prepareToCatchError();
       const char *name = extract<const char*>(d->instance.attr("name")());
       return QString(name);
    } catch(error_already_set const &) {
       catchError();
    }

    return tr("Unknown Python Extension");
  }

  QString PythonExtension::description() const
  {
    if (!PyObject_HasAttrString(d->instance.ptr(), "description"))
      return tr("N/A");

    try {
       prepareToCatchError();
       const char *name = extract<const char*>(d->instance.attr("description")());
       return QString(name);
    } catch(error_already_set const &) {
       catchError();
    }

    return tr("N/A");
  }


  QList<QAction *> PythonExtension::actions() const
  {
    QList<QAction*> actions;

    if (!d->script)
      return actions;

    try {
      prepareToCatchError();
      actions = extract< QList<QAction*> >(d->instance.attr("actions")());
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
    if (!d->script || !PyObject_HasAttrString(d->instance.ptr(), "menuPath"))
      return tr("&Scripts");

    try {
      prepareToCatchError();

      boost::python::return_by_value::apply<QAction*>::type qconverter;
      PyObject *qobj = qconverter(action);
      object real_qobj = object(handle<>(qobj));

      return extract<QString>(d->instance.attr("menuPath")(real_qobj));
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
        setText(m_command->text());
      }

      ~PythonCommand()
      {
        delete m_command;
      }

      void redo()
      {
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
    if (!d->script)
      return 0;

    // Let's just catch the exception and print the error...
    //if (!PyObject_HasAttrString(d->instance.ptr(), "performAction"))
    //  return 0;

    try {
      prepareToCatchError();

      boost::python::reference_existing_object::apply<GLWidget*>::type converter;
      PyObject *obj = converter(widget);
      object real_obj = object(handle<>(obj));

      boost::python::return_by_value::apply<QAction*>::type qconverter;
      PyObject *qobj = qconverter(action);
      object real_qobj = object(handle<>(qobj));

      object pyObj(d->instance.attr("performAction")(real_qobj, real_obj)); // new reference
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
    if (!d->script)
      return 0; // nothing we can do

    if(!d->dockWidget)
    {
      if (PyObject_HasAttrString(d->instance.ptr(), "dockWidget")) {
        try {
          prepareToCatchError();
          d->dockWidget = extract<QDockWidget*>(d->instance.attr("dockWidget")());
          d->dockWidget->setObjectName(d->dockWidget->windowTitle());
        } catch (error_already_set const &) {
          d->dockWidget = 0;
          catchError();
        }
      }

      if (d->dockWidget)
        connect(d->dockWidget, SIGNAL(destroyed()), this, SLOT(dockWidgetDestroyed()));
    }

    return d->dockWidget;
  }

  void PythonExtension::dockWidgetDestroyed()
  {
    d->dockWidget = 0;
  }

  void PythonExtension::readSettings(QSettings &settings)
  {
    Extension::readSettings(settings);

    if (!d->script)
      return;

    if (!PyObject_HasAttrString(d->instance.ptr(), "readSettings"))
      return;

    try {
      prepareToCatchError();

      boost::python::return_by_value::apply<QSettings*>::type qconverter;
      PyObject *qobj = qconverter(&settings);
      object real_qobj = object(handle<>(qobj));

      d->instance.attr("readSettings")(real_qobj);
    } catch(error_already_set const &) {
      catchError();
    }
  }

  void PythonExtension::writeSettings(QSettings &settings) const
  {
    Extension::writeSettings(settings);

    if (!d->script)
      return;

    if (!PyObject_HasAttrString(d->instance.ptr(), "writeSettings"))
      return;

    try {
      prepareToCatchError();

      boost::python::return_by_value::apply<QSettings*>::type qconverter;
      PyObject *qobj = qconverter(&settings);
      object real_qobj = object(handle<>(qobj));

      d->instance.attr("writeSettings")(real_qobj);
    } catch(error_already_set const &) {
      catchError();
    }
  }

  void PythonExtension::loadScript(const QString &filename)
  {
    QFileInfo info(filename);
    d->interpreter.addSearchPath(info.canonicalPath());

    PythonScript *script = new PythonScript(filename);
    d->identifier = script->identifier();

    if (script->module()) {
      // make sure there is an Extension class defined
      if (PyObject_HasAttrString(script->module().ptr(), "Extension")) {
        try {
          prepareToCatchError();
          d->instance = script->module().attr("Extension")();
        } catch (error_already_set const &) {
          catchError();
          return;
        }

        // connect signal(s)
        if (PyObject_HasAttrString(d->instance.ptr(), "__pyqtSignals__")) {
          QObject *obj = extract<QObject*>(d->instance);
          connect(obj, SIGNAL(message(const QString&)), this, SIGNAL(message(const QString&)));
        }

        d->script = script;

      } else {
        delete script;
        pythonError()->append(tr("PythonExtension: checking ") + filename + "...");
        pythonError()->append(tr("  - script has no 'Extension' class defined"));
      }
    } else {
      delete script;
      pythonError()->append(tr("PythonExtension: checking ") + filename + "...");
      pythonError()->append(tr("  - no module"));
    }
  }

}

#include "pythonextension.moc"
