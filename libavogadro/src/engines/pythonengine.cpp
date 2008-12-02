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

#include <openbabel/mol.h>
#include <Eigen/Regression>

#include <QMessageBox>
#include <QDebug>

using namespace std;
using namespace Eigen;
using namespace boost::python;

namespace Avogadro {

  PythonEngine::PythonEngine(QObject *parent) : Engine(parent)/*, m_settingsWidget(NULL),*/
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

  Engine* PythonEngine::clone() const
  {
    PythonEngine* engine = new PythonEngine(parent());

    engine->setAlias(alias());
    //engine->setShowDots(m_showDots);
    //engine->setShowMultipleBonds(m_showMulti);
    engine->setEnabled(isEnabled());

    return engine;
  }

  bool PythonEngine::renderOpaque(PainterDevice *pd)
  {
    if (m_scripts.isEmpty())
      return false;

    boost::python::reference_existing_object::apply<PainterDevice*>::type converter;
    PyObject *obj = converter(pd);
    object real_obj = object(handle<>(obj));
 
    m_scripts.at(0).module().attr("renderOpaque")(real_obj);

    return true;
  }

  void PythonEngine::setScriptIndex(int index)
  {
    m_scriptIndex = index;
    emit changed();
  }

  QWidget* PythonEngine::settingsWidget()
  {
    return 0;
    /*
    if(!m_settingsWidget)
    {
      m_settingsWidget = new PythonSettingsWidget();
      connect(m_settingsWidget->showMultipleCheckBox, SIGNAL(stateChanged(int)), this, SLOT(setShowMultipleBonds(int)));
      connect(m_settingsWidget->showDotsCheckBox, SIGNAL(stateChanged(int)), this, SLOT(setShowDots(int)));
      connect(m_settingsWidget, SIGNAL(destroyed()), this, SLOT(settingsWidgetDestroyed()));
      m_settingsWidget->showDotsCheckBox->setCheckState((Qt::CheckState)m_showDots);
      m_settingsWidget->showMultipleCheckBox->setCheckState((Qt::CheckState)m_showMulti);
    }
    return m_settingsWidget;
    */
  }
/*
  void PythonEngine::settingsWidgetDestroyed()
  {
    qDebug() << "Destroyed Settings Widget";
    m_settingsWidget = 0;
  }
*/
  void PythonEngine::writeSettings(QSettings &settings) const
  {
    Engine::writeSettings(settings);
  //  settings.setValue("showDots", m_showDots);
  //  settings.setValue("showMulti", m_showMulti);
  }

  void PythonEngine::readSettings(QSettings &settings)
  {
    Engine::readSettings(settings);
  //  setShowDots(settings.value("showDots", 2).toInt());
  //  setShowMultipleBonds(settings.value("showMulti", 0).toInt());
//    if (m_settingsWidget) {
 //     m_settingsWidget->showDotsCheckBox->setCheckState((Qt::CheckState)m_showDots);
   //   m_settingsWidget->showMultipleCheckBox->setCheckState((Qt::CheckState)m_showMulti);
    //}
  }

  void PythonEngine::loadScripts(QDir dir)
  {
    // add it to the search path
    m_interpreter.addSearchPath(dir.canonicalPath());

    QStringList filters;
    filters << "*.py";
    dir.setNameFilters(filters);
    dir.setFilter(QDir::Files | QDir::Readable);

    foreach(const QString& file, dir.entryList())
    {
      qDebug() << file;
      PythonScript script(dir.canonicalPath(), file);
      if(script.module())
      {
        dict local;
        local[script.moduleName().toStdString()] = script.module();
        QString name = m_interpreter.eval(script.moduleName() + ".name()", local);
        //m_actions.append(action);
        qDebug() << "Python engine name: " << name;
        m_scripts.append(script);
      }
    }
  }



}

#include "pythonengine.moc"

Q_EXPORT_PLUGIN2(pythonengine, Avogadro::PythonEngineFactory)
