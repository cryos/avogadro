/**********************************************************************
  PythonPlugin - Python plugins base class

  Copyright (C) 2011 Konstantin Tokarev

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

#include "pythonplugin_p.h"
#include "pythonengine_p.h"
#include "pythonextension_p.h"
#include "pythontool_p.h"
#include "pythonscript.h"
#include "pythonthread_p.h"

using namespace std;
using namespace boost::python;

namespace Avogadro
{
  PythonPlugin::PythonPlugin(PythonScript *script) :
    m_script(script)
  {
    if(script)
      m_identifier = script->identifier();
  }

  PythonPlugin::~PythonPlugin() {
    if (m_script)
      delete m_script;
  }

  bool PythonPlugin::hasAttrString(const char *attrString) const
  {
    return PyObject_HasAttrString(m_instance.ptr(), attrString);
  }

  QString PythonPlugin::extractString(const char *attrString) const
  {
    PythonThread pt;
    if (!hasAttrString(attrString))
      return QString();

    try {
       prepareToCatchError();
       const char *attr = extract<const char*>(m_instance.attr(attrString)());
       return QString(attr);
    } catch(error_already_set const &) {
       catchError();
    }

    return QString();
  }

  QString PythonPlugin::identifier() const
  {
    return m_identifier;
  }

  QString PythonPlugin::name() const
  {
    return extractString("name");
  }

  QString PythonPlugin::description() const
  {
    return extractString("description");
  }

  void PythonPlugin::readOrWriteSettings(const char *command, QSettings &settings) const
  {
    if (!m_script)
      return;

    PythonThread pt;

    if (!hasAttrString(command))
      return;

    try {
      prepareToCatchError();

      boost::python::return_by_value::apply<QSettings*>::type qconverter;
      PyObject *qobj = qconverter(&settings);
      object real_qobj = object(handle<>(qobj));

      m_instance.attr(command)(real_qobj);
    } catch(error_already_set const &) {
      catchError();
    }
  }

  void PythonPlugin::writeSettings(QSettings &settings) const
  {
    readOrWriteSettings("writeSettings", settings);
  }

  void PythonPlugin::readSettings(QSettings &settings)
  {
    readOrWriteSettings("readSettings", settings);
  }

  PythonPluginFactory::PythonPluginFactory(const QString &filename) :
    m_pluginInstance(0)
  {
    QFileInfo info(filename);
    initializePython(info.canonicalPath());
    PythonThread pt;

    PythonScript *script = new PythonScript(filename);

    if (script->module()) {
      // make sure there is one of possible classes defined
      if (script->hasAttrString("Engine")) {
        m_pluginInstance = new PythonEngine(script);
      } else if (script->hasAttrString("Extension")) {
        m_pluginInstance = new PythonExtension(script);
      } else if (script->hasAttrString("Tool")) {
        m_pluginInstance = new PythonTool(script);
      } else {
        delete script;
        PythonError::instance()->append(tr("Checking Python plugin ") + filename + "...");
        PythonError::instance()->append(tr("  - script has no class defined"));
      }
    } else {
      delete script;
      PythonError::instance()->append(tr("Checking Python plugin ") + filename + "...");
      PythonError::instance()->append(tr("  - no module"));
    }

    if(m_pluginInstance) {
      m_identifier = m_pluginInstance->identifier();
      m_name = m_pluginInstance->name();
      m_desc = m_pluginInstance->description();
    }
  }

  Plugin *PythonPluginFactory::createInstance(QObject *parent)
  {
    if(m_pluginInstance)
      m_pluginInstance->setParent(parent);
    return m_pluginInstance;
  }
}
