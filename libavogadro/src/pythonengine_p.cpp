/**********************************************************************
  PythonEngine - Engine for python scripts

  Copyright (C) 2008,2009 Tim Vandermeersch

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
 **********************************************************************/

#include "pythonengine_p.h"
#include "pythoninterpreter.h"
#include "pythonscript.h"

#include <avogadro/atom.h>
#include <avogadro/bond.h>
#include <avogadro/molecule.h>
#include <avogadro/color.h>
#include <avogadro/glwidget.h>
#include <avogadro/painterdevice.h>

#include <QMessageBox>
#include <QLayout>
#include <QDebug>

using namespace std;
using namespace Eigen;
using namespace boost::python;

namespace Avogadro {

  PythonEngine::PythonEngine(QObject *parent, const QString &filename) : Engine(parent), 
      m_script(0), m_settingsWidget(0)
  {
    loadScript(filename);
  }

  PythonEngine::~PythonEngine()
  {
    PythonThread pt;
    if (m_script) {
      delete m_script;
      m_script = 0;
    }
  }

  Engine* PythonEngine::clone() const
  {
    PythonEngine* engine = new PythonEngine(parent());

    engine->setAlias(alias());
    engine->setEnabled(isEnabled());

    return engine;
  }

  QString PythonEngine::identifier() const
  {
    return m_identifier;
  }

  QString PythonEngine::name() const
  {
    PythonThread pt;
    if (!PyObject_HasAttrString(m_instance.ptr(), "name"))
      return tr("Unknown Python Engine");

    try {
      prepareToCatchError();
      const char *name = extract<const char*>(m_instance.attr("name")());
      return QString(name);
    } catch(error_already_set const &) {
      catchError();
      return tr("Unknown Python Engine");
    }
  }

  QString PythonEngine::description() const
  {
    PythonThread pt;
    if (!PyObject_HasAttrString(m_instance.ptr(), "description"))
      return tr("N/A");

    try {
      prepareToCatchError();
      const char *desc = extract<const char*>(m_instance.attr("description")());
      return QString(desc);
    } catch(error_already_set const &) {
      catchError();
      return tr("N/A");
    }
  }


  bool PythonEngine::renderOpaque(PainterDevice *pd)
  {
    PythonThread pt;
    if (!m_script)
      return false; // nothing we can do

    try {
      prepareToCatchError();
      boost::python::reference_existing_object::apply<PainterDevice*>::type converter;
      PyObject *obj = converter(pd);
      object real_obj = object(handle<>(obj));

      m_instance.attr("renderOpaque")(real_obj);
    } catch(error_already_set const &) {
      catchError();
    }

    return true;
  }

  QWidget* PythonEngine::settingsWidget()
  {
    if (!m_script)
      return 0; // nothing we can do
    
    PythonThread pt;

    if(!m_settingsWidget)
    {
      m_settingsWidget = new QWidget();
      m_settingsWidget->setLayout( new QVBoxLayout() );

      if (PyObject_HasAttrString(m_instance.ptr(), "settingsWidget")) {
        try {
          prepareToCatchError();
          QWidget *widget = extract<QWidget*>(m_instance.attr("settingsWidget")());
          if (widget)
            m_settingsWidget->layout()->addWidget(widget);
        } catch (error_already_set const &) {
          catchError();
        }
      }

      connect(m_settingsWidget, SIGNAL(destroyed()), this, SLOT(settingsWidgetDestroyed()));
    }

    return m_settingsWidget;
  }

  void PythonEngine::settingsWidgetDestroyed()
  {
    m_settingsWidget = 0;
  }

  void PythonEngine::writeSettings(QSettings &settings) const
  {
    Engine::writeSettings(settings);

    if (!m_script)
      return;
    
    PythonThread pt;

    if (!PyObject_HasAttrString(m_instance.ptr(), "readSettings"))
      return;

    try {
      prepareToCatchError();

      boost::python::return_by_value::apply<QSettings*>::type qconverter;
      PyObject *qobj = qconverter(&settings);
      object real_qobj = object(handle<>(qobj));

      m_instance.attr("readSettings")(real_qobj);
    } catch(error_already_set const &) {
      catchError();
    }
  }

  void PythonEngine::readSettings(QSettings &settings)
  {
    Engine::readSettings(settings);

    if (!m_script)
      return;

    if (!PyObject_HasAttrString(m_instance.ptr(), "writeSettings"))
      return;

    try {
      prepareToCatchError();

      boost::python::return_by_value::apply<QSettings*>::type qconverter;
      PyObject *qobj = qconverter(&settings);
      object real_qobj = object(handle<>(qobj));

      m_instance.attr("writeSettings")(real_qobj);
    } catch(error_already_set const &) {
      catchError();
    }
  }

  void PythonEngine::loadScript(const QString &filename)
  {
    QFileInfo info(filename);
    initializePython(info.canonicalPath());
    PythonThread pt;

    PythonScript *script = new PythonScript(filename);
    m_identifier = script->identifier();

    if(script->module()) {
      // make sure there is an Engine class defined
      if (PyObject_HasAttrString(script->module().ptr(), "Engine")) {

        try {
          prepareToCatchError();
          // instatiate the new Engine
          m_instance = script->module().attr("Engine")();
        } catch (error_already_set const &) {
          catchError();
          return;
        }

        m_script = script;

      } else {
        delete script;
        PythonError::instance()->append(tr("PythonEngine: checking ") + filename + "...");
        PythonError::instance()->append(tr("  - script has no 'Engine' class defined"));
      }
    } else {
      delete script;
      PythonError::instance()->append(tr("PythonEngine: checking ") + filename + "...");
      PythonError::instance()->append(tr("  - no module"));
    }
  }

  Engine::Layers PythonEngine::layers() const
  {
    if (!m_script)
      return Engine::Opaque; // nothing we can do

    PythonThread pt;

    try {
      prepareToCatchError();
      // return layers from python script if the function is defined
      if (PyObject_HasAttrString(m_instance.ptr(), "layers"))
        return extract<Engine::Layers>(m_instance.attr("layers")());
    } catch(error_already_set const &) {
      catchError();
    }

    // return NoFlags, don't print an error, don't want to overwhelm new users with errors
    return Engine::Opaque;
  }

  double PythonEngine::transparencyDepth() const
  {
    if (!m_script)
      return 0.0; // nothing we can do
    
    PythonThread pt;

    try {
      prepareToCatchError();
      // return transparencyDepth from python script if the function is defined
      if (PyObject_HasAttrString(m_instance.ptr(), "transparencyDepth"))
        return extract<double>(m_instance.attr("transparencyDepth")());
    } catch(error_already_set const &) {
      catchError();
    }

    return 0.0;
  }


}

#include "pythonengine_p.moc"
