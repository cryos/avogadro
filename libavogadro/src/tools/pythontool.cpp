/**********************************************************************
  PythonTool - PythonTool Tool for Avogadro

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

#include "pythontool.h"

#include <config.h>
#include <avogadro/navigate.h>
#include <avogadro/primitive.h>
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

  PythonTool::PythonTool(QObject *parent) : Tool(parent), m_settingsWidget(0)
  {
    QAction *action = activateAction();
    action->setIcon(QIcon(QString::fromUtf8(":/python/python.png")));
    action->setToolTip(tr("Python Tools"));
    action->setShortcut(Qt::Key_F12);

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

    if(!pluginDir.cd("toolScripts")) {
      if(!pluginDir.mkdir("toolScripts")) {
        return;
      }
      if(!pluginDir.cd("toolScripts")) {
        return;
      }
    }

    loadScripts(pluginDir);

    // Now for the system wide Python scripts
    QString systemScriptsPath = QString(INSTALL_PREFIX) + '/'
      + "share/libavogadro/toolScripts";
    if (pluginDir.cd(systemScriptsPath))
      loadScripts(pluginDir);
  
    setScriptIndex(0);
  }

  PythonTool::~PythonTool()
  {
  }

  QUndoCommand* PythonTool::mouseEvent(const QString &what, GLWidget *widget, QMouseEvent *event)
  {
    if (!PyObject_HasAttrString(m_instance.ptr(), what.toStdString().c_str()))
      return 0;

    try {
      boost::python::reference_existing_object::apply<GLWidget*>::type converter;
      PyObject *obj = converter(widget);
      object real_obj = object(handle<>(obj));
    
      boost::python::return_by_value::apply<QMouseEvent*>::type qconverter;
      PyObject *qobj = qconverter(event);
      object real_qobj = object(handle<>(qobj));
 
      return extract<QUndoCommand*>(m_instance.attr(what.toStdString().c_str())(real_obj, real_qobj));
    } catch(error_already_set const &) {
      PyErr_Print();
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
      boost::python::reference_existing_object::apply<GLWidget*>::type converter;
      PyObject *obj = converter(widget);
      object real_obj = object(handle<>(obj));
    
      boost::python::return_by_value::apply<QWheelEvent*>::type qconverter;
      PyObject *qobj = qconverter(event);
      object real_qobj = object(handle<>(qobj));
 
      return extract<QUndoCommand*>(m_instance.attr("wheelEvent")(real_obj, real_qobj));
    } catch(error_already_set const &) {
      PyErr_Print();
    }
 
    return 0;
  }

  bool PythonTool::paint(GLWidget *widget)
  {
    if (!PyObject_HasAttrString(m_instance.ptr(), "paint"))
      return false;

    try {
      boost::python::reference_existing_object::apply<GLWidget*>::type converter;
      PyObject *obj = converter(widget);
      object real_obj = object(handle<>(obj));
 
      m_instance.attr("paint")(real_obj);
    } catch(error_already_set const &) {
      PyErr_Print();
    }

    return true;
  }

  void PythonTool::setScriptIndex(int index)
  {
    m_scriptIndex = index;
    try {
      // instantiate the new tool
      if (PyObject_HasAttrString(m_scripts.at(index).module().ptr(), "Tool")) {
        m_instance = m_scripts.at(index).module().attr("Tool")();
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
  }

  QWidget* PythonTool::settingsWidget()
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

  void PythonTool::settingsWidgetDestroyed()
  {
    m_settingsWidget = 0;
  }

  void PythonTool::loadScripts(QDir dir)
  {
    m_interpreter.addSearchPath(dir.canonicalPath());
    
    QStringList filters;
    filters << "*.py";
    dir.setNameFilters(filters);
    dir.setFilter(QDir::Files | QDir::Readable);

    foreach(const QString& file, dir.entryList())
    {
      qDebug() << "PythonTool: checking " << file << "...";
      PythonScript script(dir.canonicalPath(), file);
      
      if(script.module()) {
        // make sure there is a Tool class defined
        if (PyObject_HasAttrString(script.module().ptr(), "Tool")) {
          m_scripts.append(script);
          qDebug() << "  + 'Tool' class found";
        } else
          qDebug() << "  - script has no 'Tool' class defined";
      } else
        qDebug() << "  - no module";

    } // foreach file
  }

}

#include "pythontool.moc"

Q_EXPORT_PLUGIN2(pythontool, Avogadro::PythonToolFactory)
