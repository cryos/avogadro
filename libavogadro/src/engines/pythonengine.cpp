/**********************************************************************
  PythonEngine - Engine for python scripts

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

#include "pythonengine.h"

#include <config.h>
#include <avogadro/primitive.h>
#include <avogadro/atom.h>
#include <avogadro/bond.h>
#include <avogadro/molecule.h>
#include <avogadro/color.h>
#include <avogadro/glwidget.h>

#include <Eigen/Regression>

#include <QMessageBox>
#include <QDebug>

using namespace std;
using namespace Eigen;
using namespace boost::python;

namespace Avogadro {

  PythonEngine::PythonEngine(QObject *parent) : Engine(parent), m_settingsWidget(NULL), m_scriptIndex(0)
  {
    setDescription(tr("Python script rendering"));
  
    // create this directory for the user if it does not exist
    QDir pluginDir = QDir::home();

#ifdef Q_WS_MAC
    pluginDir.cd("Library/Application Support");
    if (!pluginDir.cd("Avogadro")) {
      if(!pluginDir.mkdir("Avogadro")) {
        return; // We can't create directories here
      }
      if(!pluginDir.cd("Avogadro")) {
        return; // We created the directory, but can't go into it?
      }
    }
#else
    if(!pluginDir.cd(".avogadro")) {
      if(!pluginDir.mkdir(".avogadro")) {
        return; // We can't create directories here
      }
      if(!pluginDir.cd(".avogadro")) {
        return; // We created the directory, but can't go into it?
      }
    }
#endif

    if(!pluginDir.cd("engineScripts")) {
      if(!pluginDir.mkdir("engineScripts")) {
        return;
      }
      if(!pluginDir.cd("engineScripts")) {
        return;
      }
    }

    loadScripts(pluginDir);

    // Now for the system wide Python scripts
    QString systemScriptsPath = QString(INSTALL_PREFIX) + '/'
      + "share/libavogadro/engineScripts";
    pluginDir.cd(systemScriptsPath);
    loadScripts(pluginDir);
  }
  
  PythonEngine::~PythonEngine()
  {
  }

  Engine* PythonEngine::clone() const
  {
    PythonEngine* engine = new PythonEngine(parent());

    engine->setAlias(alias());
    engine->setScriptIndex(m_scriptIndex);
    engine->setEnabled(isEnabled());

    return engine;
  }

  bool PythonEngine::renderOpaque(PainterDevice *pd)
  {
    if (m_scriptIndex >= m_scripts.size())
      return false;

    boost::python::reference_existing_object::apply<PainterDevice*>::type converter;
    PyObject *obj = converter(pd);
    object real_obj = object(handle<>(obj));
 
    try {
      m_instance.attr("renderOpaque")(real_obj);
    } catch(error_already_set const &) {
      PyErr_Print();
    }

    return true;
  }

  void PythonEngine::setScriptIndex(int index)
  {
    m_scriptIndex = index;
    try {
      // instantiate the new engine
      if (PyObject_HasAttrString(m_scripts.at(index).module().ptr(), "Engine")) {
        m_instance = m_scripts.at(index).module().attr("Engine")();
        QObject *instance = extract<QObject*>(m_instance);
        if (instance)
          connect(instance, SIGNAL(changed()), this, SIGNAL(changed()));

        if (m_settingsWidget) {
          if (PyObject_HasAttrString(m_instance.ptr(), "settingsWidget")) {
            QWidget *widget = extract<QWidget*>(m_instance.attr("settingsWidget")());
            if (widget) 
              m_settingsWidget->groupBox->layout()->addWidget(widget);
          }
        }

      }
    } catch (error_already_set const &) {
      PyErr_Print();
    }
    emit changed();
  }

  QWidget* PythonEngine::settingsWidget()
  {
    if(!m_settingsWidget)
    {
      m_settingsWidget = new PythonSettingsWidget();

      foreach (const PythonScript &script, m_scripts) {
        m_settingsWidget->scriptsComboBox->addItem(script.moduleName());
      }
      m_settingsWidget->scriptsComboBox->setCurrentIndex(m_scriptIndex);

      try {
        QWidget *widget = extract<QWidget*>(m_instance.attr("settingsWidget")());
        if (widget)
          m_settingsWidget->groupBox->layout()->addWidget(widget);
      } catch (error_already_set const &) {
        PyErr_Print();
      }

      connect(m_settingsWidget->scriptsComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(setScriptIndex(int)));
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
    if (m_scriptIndex < m_scripts.size())
      settings.setValue("scriptName", m_scripts.at(m_scriptIndex).moduleName());
  }

  void PythonEngine::readSettings(QSettings &settings)
  {
    Engine::readSettings(settings);

    QString refName = settings.value("scriptName").toString();
    int index = 0;
    foreach (const PythonScript &script, m_scripts) {
      if (script.moduleName() == refName)
        setScriptIndex(index);
      index++;
    }
  }

  void PythonEngine::loadScripts(QDir dir)
  {
    m_interpreter.addSearchPath(dir.canonicalPath());
    
    QStringList filters;
    filters << "*.py";
    dir.setNameFilters(filters);
    dir.setFilter(QDir::Files | QDir::Readable);

    foreach(const QString& file, dir.entryList())
    {
      qDebug() << "PythonEngine: checking " << file << "...";
      PythonScript script(dir.canonicalPath(), file);
      
      if(script.module()) {
        // make sure there is an Engine class defined
        if (PyObject_HasAttrString(script.module().ptr(), "Engine")) {
          m_scripts.append(script);
          qDebug() << "  + 'Engine' class found";
        } else
          qDebug() << "  - script has no 'Engine' class defined";
      } else
        qDebug() << "  - no module";

    } // foreach file
  }

  Engine::Layers PythonEngine::layers() const
  {
    try {
      // return flags from python script if the function is defined
      if (PyObject_HasAttrString(m_instance.ptr(), "layers"))
        return extract<Engine::Layers>(m_instance.attr("layers")());
    } catch(error_already_set const &) {
      PyErr_Print();
    }
      
    // return NoFlags, don't print an error, don't want to overwhelm new users with errors
    return Engine::Opaque;
  }
  
  double PythonEngine::transparencyDepth() const
  {
    // see flags()
    try {
      if (PyObject_HasAttrString(m_instance.ptr(), "transparencyDepth"))
        return extract<double>(m_instance.attr("transparencyDepth")());
    } catch(error_already_set const &) {
      PyErr_Print();
    }
      
    return 0.0;
  }
 

}

#include "pythonengine.moc"

Q_EXPORT_PLUGIN2(pythonengine, Avogadro::PythonEngineFactory)
