/**********************************************************************
  PythonTool - PythonTool Tool for Avogadro

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

#include "pythontool_p.h"
#include "pythonscript.h"
#include "pythonthread_p.h"

#include <avogadro/navigate.h>
#include <avogadro/atom.h>
#include <avogadro/molecule.h>
#include <avogadro/color.h>
#include <avogadro/glwidget.h>

#include <cmath>

#include <QDebug>
#include <QtPlugin>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QUndoCommand>

using namespace std;
using namespace Eigen;
using namespace boost::python;

namespace Avogadro {

  PythonTool::PythonTool(PythonScript *script, QObject *parent) :
      Tool(parent), PythonPlugin(script), m_settingsWidget(0)
  {
    loadScript(script);

    QAction *action = activateAction();
    action->setIcon(QIcon(QString::fromUtf8(":/python/python.png")));

    PythonThread pt;
    if (PyObject_HasAttrString(m_instance.ptr(), "toolTip")) {
      try {
        prepareToCatchError();
        const char *toolTip = extract<const char*>(m_instance.attr("toolTip")());
        action->setToolTip(QString(toolTip));
      } catch(error_already_set const &) {
        catchError();
      }
    }
  }

  PythonTool::~PythonTool()
  {
    PythonThread pt;
    if (m_settingsWidget)
      m_settingsWidget->deleteLater();
  }

  QString PythonTool::name() const
  {
    const QString name = PythonPlugin::name();
    if(!name.isNull())
      return name;
    else
      return tr("Unknown Python Tool");
  }

  QString PythonTool::description() const
  {
    const QString desc = PythonPlugin::description();
    if(!desc.isNull())
      return desc;
    else
      return tr("N/A");
  }

  QString PythonTool::settingsTitle() const
  {
    PythonThread pt;
    if (!hasAttrString("settingsTitle"))
      return tr("N/A");

    try {
      prepareToCatchError();
      const char *desc = extract<const char*>(m_instance.attr("settingsTitle")());
      return QString(desc);
    } catch(error_already_set const &) {
      catchError();
      return tr("N/A");
    }
  }


  QUndoCommand* PythonTool::mouseEvent(const QString &what, GLWidget *widget, QMouseEvent *event)
  {
    PythonThread pt;
    if (!hasAttrString(what.toStdString().c_str()))
      return 0;

    try {
      prepareToCatchError();
      boost::python::reference_existing_object::apply<GLWidget*>::type converter;
      PyObject *obj = converter(widget);
      object real_obj = object(handle<>(obj));

      boost::python::return_by_value::apply<QMouseEvent*>::type qconverter;
      PyObject *qobj = qconverter(event);
      object real_qobj = object(handle<>(qobj));

      return extract<QUndoCommand*>(m_instance.attr(what.toStdString().c_str())(real_obj, real_qobj));
    } catch(error_already_set const &) {
      catchError();
    }

    return 0;
  }

  QUndoCommand* PythonTool::mousePressEvent(GLWidget *widget, QMouseEvent *event)
  {
    return mouseEvent("mousePressEvent", widget, event);
  }

  QUndoCommand* PythonTool::mouseMoveEvent(GLWidget *widget, QMouseEvent *event)
  {
    return mouseEvent("mouseMoveEvent", widget, event);
  }

  QUndoCommand* PythonTool::mouseReleaseEvent(GLWidget *widget, QMouseEvent *event)
  {
    return mouseEvent("mouseReleaseEvent", widget, event);
  }

  QUndoCommand* PythonTool::wheelEvent(GLWidget *widget, QWheelEvent *event)
  {
    PythonThread pt;
    if (!hasAttrString("wheelEvent"))
      return 0;

    try {
      prepareToCatchError();
      boost::python::reference_existing_object::apply<GLWidget*>::type converter;
      PyObject *obj = converter(widget);
      object real_obj = object(handle<>(obj));

      boost::python::return_by_value::apply<QWheelEvent*>::type qconverter;
      PyObject *qobj = qconverter(event);
      object real_qobj = object(handle<>(qobj));

      return extract<QUndoCommand*>(m_instance.attr("wheelEvent")(real_obj, real_qobj));
    } catch(error_already_set const &) {
      catchError();
    }

    return 0;
  }

  bool PythonTool::paint(GLWidget *widget)
  {
    PythonThread pt;
    if (!hasAttrString("paint"))
      return false;

    try {
      prepareToCatchError();
      boost::python::reference_existing_object::apply<GLWidget*>::type converter;
      PyObject *obj = converter(widget);
      object real_obj = object(handle<>(obj));

      m_instance.attr("paint")(real_obj);
    } catch(error_already_set const &) {
      catchError();
    }

    return true;
  }

  QWidget* PythonTool::settingsWidget()
  {
    if (!m_script)
      return 0; // nothing we can do -- we don't have any real scripts

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

  void PythonTool::settingsWidgetDestroyed()
  {
    m_settingsWidget = 0;
  }

  void PythonTool::readSettings(QSettings &settings)
  {
    Tool::readSettings(settings);
    PythonPlugin::readSettings(settings);
  }

  void PythonTool::writeSettings(QSettings &settings) const
  {
    Tool::writeSettings(settings);
    PythonPlugin::writeSettings(settings);
  }

  void PythonTool::loadScript(PythonScript *script)
  {
    PythonThread pt;
    try {
      prepareToCatchError();
      // instantiate the new tool
      m_instance = script->module().attr("Tool")();
      // if we have a settings widget already, add the python content...
      if (m_settingsWidget) {
        if (hasAttrString("settingsWidget")) {
          QWidget *widget = extract<QWidget*>(m_instance.attr("settingsWidget")());
          if (widget)
            m_settingsWidget->layout()->addWidget(widget);
        }
      }
    } catch (error_already_set const &) {
      catchError();
      return;
    }
  }

}
