/**********************************************************************
  Python - Gives us some Python helper stuff

  Copyright (C) 2008 by Donald Ephraim Curtis

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.sourceforge.net/>

  Some code is based on Open Babel
  For more information, see <http://openbabel.sourceforge.net/>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation version 2 of the License.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
 ***********************************************************************/

#include "pythonextension.h"
#include "../config.h"

#include <avogadro/molecule.h>

#include <QDockWidget>
#include <QCompleter>
#include <QKeyEvent>
#include <QUndoCommand>

#include <QDebug>

using namespace std;
//using namespace OpenBabel;
using namespace boost::python;

namespace Avogadro
{

  enum PythonIndex
  {
    ScriptIndex = 0
  };

  PythonExtension::PythonExtension( QObject *parent ) : Extension( parent ), m_molecule(0), m_terminalDock(0)
  {
    m_reloadAction = 0;
    findScripts();
  }

  void PythonExtension::findScripts()
  {
    m_scripts.clear();
    m_instances.clear();
    foreach (QAction *action, m_actions)
      action->deleteLater();
    m_actions.clear();
    m_actionHash.clear();

    //if (m_reloadAction)
    //  delete m_reloadAction;
    m_reloadAction = new QAction( this );
    m_reloadAction->setText("Reload Python Extensions");
    m_actions.append(m_reloadAction);


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
  #ifdef WIN32
    pluginDir = QCoreApplication::applicationDirPath();
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
#endif

    if(!pluginDir.cd("extensionScripts")) {
      if(!pluginDir.mkdir("extensionScripts")) {
        return;
      }
      if(!pluginDir.cd("extensionScripts")) {
        return;
      }
    }

    loadScripts(pluginDir);

#ifndef WIN32
    // Now for the system wide Python scripts
    QString systemScriptsPath = QString(INSTALL_PREFIX) + '/'
      + "share/libavogadro/extensionScripts";
    pluginDir.cd(systemScriptsPath);
    loadScripts(pluginDir);
#endif
  }

  void PythonExtension::loadScripts(QDir dir)
  {
    // add it to the search path
    m_interpreter.addSearchPath(dir.canonicalPath());

    QStringList filters;
    filters << "*.py";
    dir.setNameFilters(filters);
    dir.setFilter(QDir::Files | QDir::Readable);

    foreach(const QString& file, dir.entryList())
    {
      qDebug() << "PythonExtension: checking " << file << "...";
      PythonScript script(dir.canonicalPath(), file);
      if(script.module())
      {
        // make sure there is an Engine class defined
        if (PyObject_HasAttrString(script.module().ptr(), "Extension")) {
          m_scripts.append(script);
          qDebug() << "  + 'Extension' class found";

          // instantiate the python extension
          object instance;
          try {
            prepareToCatchError();
            instance = script.module().attr("Extension")();
            m_instances.append(instance);
          } catch (error_already_set const &) {
            catchError();
          }
          
          // try getting the QActions
          if (PyObject_HasAttrString(instance.ptr(), "actions")) {
            //object pyqtActions = instance.attr("actions")();
            try {
              prepareToCatchError();
              QList<QAction*> actions = extract< QList<QAction*> >(instance.attr("actions")());

              foreach (QAction *action, actions) {
                action->setParent( this ); // this will make the MainWindow call performAction on this extension
                m_actions.append(action);
                m_actionHash[action] = m_instances.indexOf(instance);
              }

            } catch (error_already_set const &) {
              catchError();
            }
          } else {
            QString msg;
            msg = "PythonExtension: checking " + file + "...\n";
            msg += "  - script has no 'Extension.actions()' method defined\n";
            pythonError()->append( msg );


            qDebug() << "  - script has no 'Extension.actions()' method defined";
          }

        } else {
          QString msg;
          msg = "PythonExtension: checking " + file + "...\n";
          msg += "  - script has no 'Extension' class defined\n";
          pythonError()->append( msg );

          qDebug() << "  - script has no 'Extension' class defined";
        }

      } else {
        // PythonScript::module() creates it's own dialog with the error...
        qDebug() << "  - no module";
      }
    }
  }

  PythonExtension::~PythonExtension()
  {
    if (m_terminalDock)
      m_terminalDock->deleteLater();
  }

  QList<QAction *> PythonExtension::actions() const
  {
    return m_actions;
  }

  // allows us to set the intended menu path for each action
  QString PythonExtension::menuPath(QAction *action) const
  {
    qDebug() << "PythonExtension::menuPath()";

    if (action == m_reloadAction) {
      return tr("&Extensions");
    }

    int instanceIdx = m_actionHash[action];

    if (!PyObject_HasAttrString(m_instances.at(instanceIdx).ptr(), "menuPath")) {
      return tr("&Scripts");
    }

    try {
      prepareToCatchError();

      boost::python::return_by_value::apply<QAction*>::type qconverter;
      PyObject *qobj = qconverter(action);
      object real_qobj = object(handle<>(qobj));

      return extract<QString>(m_instances.at(instanceIdx).attr("menuPath")(real_qobj));
    } catch(error_already_set const &) {
      catchError();
    }

    return tr("&Scripts");
  }

  QDockWidget * PythonExtension::dockWidget()
  {
    if(!m_terminalDock)
    {
      m_terminalDock = new QDockWidget( tr("Python Terminal"), qobject_cast<QWidget *>(parent()) );
      m_terminalWidget = new PythonTerminalWidget();
      m_terminalDock->setWidget(m_terminalWidget);
      m_terminalDock->setObjectName( tr("pythonTerminalDock") );

      connect(m_terminalWidget->inputLine, SIGNAL(returnPressed()), this, SLOT(runCommand()));
    }

    return m_terminalDock;
  }

  void PythonExtension::setMolecule(Molecule *molecule)
  {
    m_molecule = molecule;
    m_interpreter.setMolecule(molecule);
  }

  void PythonExtension::runCommand()
  {
    QString text = m_terminalWidget->inputLine->text();
    if(!text.isEmpty())
    {
      m_terminalWidget->ui.outputText->append(">>> " + text);
      QString result = m_interpreter.exec(text);
      if(!result.isEmpty()) {
        m_terminalWidget->ui.outputText->append(result);
      }
      m_terminalWidget->inputLine->clear();
      // Always update the molecule when running commands from the terminal widget
      m_molecule->update();
    }
  }

  QUndoCommand* PythonExtension::performAction( QAction *action, GLWidget *widget )
  {
    qDebug() << "PythonExtension::performAction()";

    if (action == m_reloadAction) {
      qDebug() << "Reloading python extensions";
      findScripts();
      emit actionsChanged( this );
      return 0;
    }

    int instanceIdx = m_actionHash[action];

    if (!PyObject_HasAttrString(m_instances.at(instanceIdx).ptr(), "performAction")) {
      return 0;
    }

    try {
      prepareToCatchError();
      boost::python::reference_existing_object::apply<GLWidget*>::type converter;
      PyObject *obj = converter(widget);
      object real_obj = object(handle<>(obj));

      boost::python::return_by_value::apply<QAction*>::type qconverter;
      PyObject *qobj = qconverter(action);
      object real_qobj = object(handle<>(qobj));

      return extract<QUndoCommand*>(m_instances.at(instanceIdx).attr("performAction")(real_qobj, real_obj));
    } catch(error_already_set const &) {
      catchError();
    }

    return 0;
  }

  PythonTerminalWidget::PythonTerminalWidget( QWidget *parent ) : QWidget(parent)
  {
    ui.setupUi(this);
    QFont font;
    font.setFamily(QString::fromUtf8("DejaVu Sans Mono"));
    inputLine = new PythonTerminalLineEdit(this);
    inputLine->setObjectName(QString::fromUtf8("inputLine"));
    inputLine->setFont(font);

    // TODO: Make a full completion list, including spaces, separators, etc.
    QStringList wordList;
    wordList << "Avogadro" << "molecule" << "atom" << "bond";
    wordList << "numAtoms" << "numBonds";

    wordList << "import Avogadro" << "widget = Avogadro.GLWidget.current()";

    QCompleter *completer = new QCompleter(wordList, this);
    completer->setCompletionMode(QCompleter::InlineCompletion);
    completer->setCaseSensitivity(Qt::CaseSensitive);
    inputLine->setCompleter(completer);

    layout()->addWidget(inputLine);
  }

  PythonTerminalLineEdit::PythonTerminalLineEdit(QWidget *parent ) : QLineEdit(parent), m_current(0)
  {
    // Load the saved commands
    QSettings settings;
    int size = settings.beginReadArray("pythonCommands");
    for (int i = 0; i < size; ++i) {
      settings.setArrayIndex(i);
      m_commandStack.append( settings.value("command").toString() );
    }
    settings.endArray();
  }

  void PythonTerminalLineEdit::keyPressEvent ( QKeyEvent * event )
  {
    if(event->key() == Qt::Key_Up)
    {
      if(m_commandStack.size())
      {
        m_current--;
        if(m_current < 0) {
          m_current = m_commandStack.size();
        }

        if(m_current == m_commandStack.size())
        {
          clear();
        }
        else
        {
          setText(m_commandStack.at(m_current));
        }

      }
      event->accept();
    }
    else if(event->key() == Qt::Key_Down)
    {
      if(m_commandStack.size())
      {
        m_current++;
        if(m_current > m_commandStack.size()) {
          m_current = 0;
        }

        if(m_current == m_commandStack.size())
        {
          clear();
        }
        else
        {
          setText(m_commandStack.at(m_current));
        }

      }
      event->accept();
    }
    else if(event->key() == Qt::Key_Return)
    {
      QString t = text();
      if(!t.isEmpty())
      {
        m_commandStack.append(text());
        // this limits how many commands we save
        if(m_commandStack.size() > 100)
        {
          m_commandStack.removeFirst();
        }
        // save the commands before we execute, this will allow users to see
        // what they did before the crash
        QSettings settings;
        settings.beginWriteArray("pythonCommands");
        for (int i = 0; i < m_commandStack.size(); ++i) {
          settings.setArrayIndex(i);
          settings.setValue("command", m_commandStack.at(i));
        }
        settings.endArray();
      }
      m_current = m_commandStack.size();
      event->accept();
    }
    QLineEdit::keyPressEvent(event);
  }
}

#include "pythonextension.moc"

Q_EXPORT_PLUGIN2(pythonextension, Avogadro::PythonExtensionFactory)
