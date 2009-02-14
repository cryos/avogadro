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

#include <config.h>
#include <avogadro/molecule.h>

#include <QApplication>
#include <QUndoCommand>
#include <QAction>
#include <QFileInfo>
#include <QDebug>

using namespace std;
using namespace boost::python;

namespace Avogadro
{

  class PythonExtensionPrivate
  {
    public:
      PythonExtensionPrivate() : script(0)
      {}

      PythonInterpreter      interpreter;
      PythonScript          *script;
      boost::python::object  instance;
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
    delete d;
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

      return new PythonCommand(command);
    } catch(error_already_set const &) {
      catchError();
    }

    return 0;
  }

  void PythonExtension::loadScript(const QString &filename)
  {
    QFileInfo info(filename);
    d->interpreter.addSearchPath(info.canonicalPath());
      
    pythonError()->append(tr("PythonExtension: checking ") + filename + "...");

    PythonScript *script = new PythonScript(filename);

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

        d->script = script;
          
      } else {
        delete script;
        pythonError()->append(tr("  - script has no 'Extension' class defined"));
      }
    } else {
      delete script;
      pythonError()->append(tr("  - no module"));
    }
  }

}

#include "pythonextension.moc"
