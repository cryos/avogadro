/**********************************************************************
  PythonEngine - Allow python scripts to be used as engines.

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

#ifndef PYTHONENGINE_H
#define PYTHONENGINE_H

#include <avogadro/global.h>
#include <avogadro/engine.h>
#include <avogadro/pythoninterpreter.h>
#include <avogadro/pythonscript.h>

namespace Avogadro {

  class PythonEnginePrivate;

  class PythonEngine : public Engine
  {
    Q_OBJECT

    public:
      //! Constructor
      PythonEngine(QObject *parent = 0, const QString &filename = QString());
      //! Deconstructor
      ~PythonEngine();
      //! Copy
      Engine *clone() const;


      //! \name To python delegetad functions
      //@{
      QString name() const;
      QString description() const;
      Layers layers() const;
      double transparencyDepth() const;
      bool renderOpaque(PainterDevice *pd);
      QWidget* settingsWidget();
      //@}
      
      
      /**
       * Write the engine settings so that they can be saved between sessions.
       */
      void writeSettings(QSettings &settings) const;

      /**
       * Read in the settings that have been saved for the engine instance.
       */
      void readSettings(QSettings &settings);

  private:
    void loadScript(const QString &filename);

    PythonEnginePrivate * const d;
 
  private Q_SLOTS:
    void settingsWidgetDestroyed();
  };

  //! Generates instances of our PythonEngine class
  class PythonEngineFactory : public QObject, public PluginFactory
  {
    Q_OBJECT
    Q_INTERFACES(Avogadro::PluginFactory)

    public:
      PythonEngineFactory(const QString &filename) : m_filename(filename)
      {
        PythonEngine engine(0, filename);
        m_name = engine.name();
        m_desc = engine.description();
      }
      Plugin* createInstance(QObject *parent = 0)
      {
        return new PythonEngine(parent, m_filename);
      }
      Plugin::Type type() const { return Plugin::EngineType; }
      QString name() const { return m_name; }
      QString description() const { return m_desc; }
    private:
      QString m_filename;
      QString m_name, m_desc;
  };

} // end namespace Avogadro

#endif
