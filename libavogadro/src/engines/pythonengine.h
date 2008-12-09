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

#include "ui_pythonsettingswidget.h"

namespace Avogadro {

  class Atom;
  class Bond;
  class PythonSettingsWidget;

  class PythonEngine : public Engine
  {
    Q_OBJECT
    AVOGADRO_ENGINE(tr("Python"))

    public:
      //! Constructor
      PythonEngine(QObject *parent=0);
      //! Deconstructor
      ~PythonEngine();
      //! Copy
      Engine *clone() const;


      //! \name Methods Render Methods
      //@{
      //! Return flags from the current script
      EngineFlags flags() const;
      double transparencyDepth() const;
      bool renderOpaque(PainterDevice *pd);
      
      //! Configuration options
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
    PythonSettingsWidget *m_settingsWidget;
    
    QList<PythonScript> m_scripts;
    PythonInterpreter m_interpreter;

    int m_scriptIndex;
    boost::python::object m_instance;
    
    void loadScripts(QDir dir);

  private Q_SLOTS:
    void settingsWidgetDestroyed();
    void setScriptIndex(int index);
  };

  class PythonSettingsWidget : public QWidget, public Ui::PythonSettingsWidget
  {
  public:
    PythonSettingsWidget(QWidget *parent=0) : QWidget(parent) {
      setupUi(this);
    }
  };

  //! Generates instances of our PythonEngine class
  class PythonEngineFactory : public QObject, public PluginFactory
  {
    Q_OBJECT
    Q_INTERFACES(Avogadro::PluginFactory)
    AVOGADRO_ENGINE_FACTORY(PythonEngine,
        tr("Allow python scripts to be used as engines."))
  };

} // end namespace Avogadro

#endif
