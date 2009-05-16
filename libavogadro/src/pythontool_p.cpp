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

  PythonTool::PythonTool(QObject *parent, const QString &filename) : Tool(parent), 
      m_script(0), m_settingsWidget(0)
  {
    QAction *action = activateAction();
    action->setIcon(QIcon(QString::fromUtf8(":/python/python.png")));
    //action->setShortcut(Qt::Key_F12);

    loadScript(filename);

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
    if (m_script)
      delete m_script;
    if (m_settingsWidget)
      m_settingsWidget->deleteLater();
  }

  QString PythonTool::identifier() const
  {
    return m_identifier;
  }

  QString PythonTool::name() const
  {
    if (!PyObject_HasAttrString(m_instance.ptr(), "name"))
      return tr("Unknown Python Tool");

    try {
      prepareToCatchError();
      const char *name = extract<const char*>(m_instance.attr("name")());
      return QString(name);
    } catch(error_already_set const &) {
      catchError();
      return tr("Unknown Python Tool");
    }
  }

  QString PythonTool::description() const
  {
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

  QUndoCommand* PythonTool::mouseEvent(const QString &what, GLWidget *widget, QMouseEvent *event)
  {
    if (!PyObject_HasAttrString(m_instance.ptr(), what.toStdString().c_str()))
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
    if (!PyObject_HasAttrString(m_instance.ptr(), "wheelEvent"))
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
    if (!PyObject_HasAttrString(m_instance.ptr(), "paint"))
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

  void PythonTool::settingsWidgetDestroyed()
  {
    m_settingsWidget = 0;
  }

  void PythonTool::readSettings(QSettings &settings)
  {
    Tool::readSettings(settings);

    if (!m_script)
      return;

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

  void PythonTool::writeSettings(QSettings &settings) const
  {
    Tool::writeSettings(settings);

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

  void PythonTool::loadScript(const QString &filename)
  {
    QFileInfo info(filename);
    initializePython(info.canonicalPath());

    PythonScript *script = new PythonScript(filename);
    m_identifier = script->identifier();

    if(script->module()) {
      // make sure there is a Tool class defined
      if (PyObject_HasAttrString(script->module().ptr(), "Tool")) {
        try {
          prepareToCatchError();
          // instantiate the new tool
          m_instance = script->module().attr("Tool")();
          // if we have a settings widget already, add the python content...
          if (m_settingsWidget) {
            if (PyObject_HasAttrString(m_instance.ptr(), "settingsWidget")) {
              QWidget *widget = extract<QWidget*>(m_instance.attr("settingsWidget")());
              if (widget)
                m_settingsWidget->layout()->addWidget(widget);
            }
          }
        } catch (error_already_set const &) {
          catchError();
          return;
        }

        m_script = script;

      } else {
        delete script;
        PythonError::instance()->append(tr("PythonTool: checking ") + filename + "...");
        PythonError::instance()->append(tr("  - script has no 'Tool' class defined"));
      }
    } else {
      delete script;
      PythonError::instance()->append(tr("PythonTool: checking ") + filename + "...");
      PythonError::instance()->append(tr("  - no module"));
    }
  }

}

#include "pythontool_p.moc"
