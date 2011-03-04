/**********************************************************************
  PythonEngine - Allow python scripts to be used as engines.

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

#ifndef PYTHONENGINE_H
#define PYTHONENGINE_H

#include <avogadro/global.h>
#include <avogadro/engine.h>
#include <boost/python.hpp>
#include "pythonplugin_p.h"

namespace Avogadro {

  class PythonScript;

  class PythonEngine : public Engine, public PythonPlugin
  {
    Q_OBJECT

    public:
      //! Constructor
      explicit PythonEngine(PythonScript *script, QObject *parent = 0);
      //! Deconstructor
      ~PythonEngine();
      //! Copy
      Engine *clone() const;

      Plugin::Type type() const { return Plugin::EngineType; }

      //! \name To python delegetad functions
      //@{
      QString identifier() const { return PythonPlugin::identifier(); }
      QString name() const;
      QString description() const;
      Layers layers() const;
      double transparencyDepth() const;
      bool renderOpaque(PainterDevice *pd);
      QWidget* settingsWidget();
      void writeSettings(QSettings &settings) const;
      void readSettings(QSettings &settings);
      //@}
      
    private:
      void loadScript(PythonScript *script);

//      PythonScript          *m_script;
//      boost::python::object  m_instance;
      QWidget               *m_settingsWidget;
//      QString                m_identifier;
    private Q_SLOTS:
      void settingsWidgetDestroyed();
  };
/*
  //! Generates instances of our PythonEngine class
  class PythonEngineFactory : public QObject, public PluginFactory
  {
    Q_OBJECT
    Q_INTERFACES(Avogadro::PluginFactory)

    public:
      PythonEngineFactory(const QString &filename) : m_filename(filename)
      {
        PythonEngine engine(0, filename);
        m_identifier = engine.identifier();
        m_name = engine.name();
        m_desc = engine.description();
      }
      Plugin* createInstance(QObject *parent = 0)
      {
        return new PythonEngine(parent, m_filename);
      }
      Plugin::Type type() const { return Plugin::EngineType; }
      QString identifier() const { return m_identifier; }
      QString name() const { return m_name; }
      QString description() const { return m_desc; }
    private:
      QString m_filename;
      QString m_identifier, m_name, m_desc;
  };
*/
} // end namespace Avogadro

#endif
