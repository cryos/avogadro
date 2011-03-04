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
#include "pythonscript.h"
#include "pythonthread_p.h"

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

  PythonEngine::PythonEngine(PythonScript *script, QObject *parent) :
      Engine(parent), PythonPlugin(script), m_settingsWidget(0)
  {
    loadScript(script);
  }

  PythonEngine::~PythonEngine()
  {
    //PythonThread pt;
    /*
    if (m_script) {
      delete m_script;
      m_script = 0;
    }
    */
    //if (m_settingsWidget)
    //  m_settingsWidget->deleteLater();
  }

  Engine* PythonEngine::clone() const
  {
    PythonEngine* engine = new PythonEngine(m_script, parent());

    engine->setAlias(alias());
    engine->setEnabled(isEnabled());

    return engine;
  }

  QString PythonEngine::name() const
  {
    const QString name = PythonPlugin::name();
    if(!name.isNull())
      return name;
    else
      return tr("Unknown Python Engine");
  }

  QString PythonEngine::description() const
  {
    const QString desc = PythonPlugin::description();
    if(!desc.isNull())
      return desc;
    else
      return tr("N/A");
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

      if (hasAttrString("settingsWidget")) {
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
    PythonPlugin::writeSettings(settings);
  }

  void PythonEngine::readSettings(QSettings &settings)
  {
    Engine::readSettings(settings);
    PythonPlugin::readSettings(settings);
  }

  void PythonEngine::loadScript(PythonScript *script)
  {
    PythonThread pt;

    try {
      prepareToCatchError();
      // instatiate the new Engine
      m_instance = script->module().attr("Engine")();
    } catch (error_already_set const &) {
      catchError();
      return;
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
      if (hasAttrString("layers"))
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
      if (hasAttrString("transparencyDepth"))
        return extract<double>(m_instance.attr("transparencyDepth")());
    } catch(error_already_set const &) {
      catchError();
    }

    return 0.0;
  }


}
