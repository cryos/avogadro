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

#ifndef PYTHONPLUGIN_H
#define PYTHONPLUGIN_H

#include <avogadro/plugin.h>
#include <boost/python.hpp>

//#ifdef ENABLE_PYTHON

namespace Avogadro {

  class PythonScript;

  class PythonPlugin
  {
  public:
    virtual ~PythonPlugin();

    // Copy of Plugin interface (to prevent diamond inheritance)
    virtual Plugin::Type type() const = 0;
    virtual QString identifier() const = 0;
    virtual QString name() const = 0;
    virtual QString description() const = 0;
    virtual void writeSettings(QSettings &settings) const = 0;
    virtual void readSettings(QSettings &settings) = 0;

  protected:
    explicit PythonPlugin(PythonScript *script);
    bool hasAttrString(const char *attrString) const;
    QString extractString(const char *attrString) const;

    PythonScript *m_script;
    boost::python::object m_instance;
    QString m_identifier;

  private:
    void readOrWriteSettings(const char *command, QSettings &settings) const;
  };

  class PythonPluginFactory : public QObject, public PluginFactory
  {
    Q_OBJECT
    Q_INTERFACES(Avogadro::PluginFactory)

  public:
    PythonPluginFactory(const QString &filename);
    Plugin *createInstance(QObject *parent = 0);

    Plugin::Type type() const { return Plugin::ToolType; }
    QString identifier() const { return m_identifier; }
    QString name() const { return m_name; }
    QString description() const { return m_desc; }
  private:
    QString m_identifier;
    QString m_name;
    QString m_desc;
    Plugin *m_pluginInstance;
  };
}

//#endif

#endif
