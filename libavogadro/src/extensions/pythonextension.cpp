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


    if(!pluginDir.cd("scripts")) {
      if(!pluginDir.mkdir("scripts")) {
        return;
      }
      if(!pluginDir.cd("scripts")) {
        return;
      }
    }

    loadScripts(pluginDir);

    // Now for the system wide Python scripts
    QString systemScriptsPath = QString(INSTALL_PREFIX) + '/'
      + "share/libavogadro/scripts";
    pluginDir.cd(systemScriptsPath);
    loadScripts(pluginDir);
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
      qDebug() << file;
      PythonScript script(dir.canonicalPath(), file);
      if(script.module())
      {
        dict local;
        local[script.moduleName().toStdString()] = script.module();
        QAction *action = new QAction( this );
        QString name = m_interpreter.eval(script.moduleName() + ".name()", local);
        action->setText( name );
        action->setData(ScriptIndex + m_scripts.size());
        m_actions.append(action);

        m_scripts.append(script);

      }
    }
  }

  PythonExtension::~PythonExtension()
  {
  }

  QList<QAction *> PythonExtension::actions() const
  {
    return m_actions;
  }

  // allows us to set the intended menu path for each action
  QString PythonExtension::menuPath(QAction *) const
  {
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
    Q_UNUSED(widget);
    int i = action->data().toInt();

    // are we running a script?
    if(i >= ScriptIndex)
    {
//      PythonScript script = m_scripts.at(i - ScriptIndex);
//      qDebug() << "Executing Script" << script.name();
      dict local;
      local[m_scripts.at(i - ScriptIndex).moduleName().toStdString()] =
        m_scripts.at(i - ScriptIndex).module();
      QString output = m_interpreter.exec(m_scripts.at(i-ScriptIndex).moduleName() + ".extension()", local);
      emit message(output);
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
    completer->setCaseSensitivity(Qt::CaseSensitive);
    inputLine->setCompleter(completer);

    layout()->addWidget(inputLine);
  }

  PythonTerminalLineEdit::PythonTerminalLineEdit(QWidget *parent ) : QLineEdit(parent), m_current(0)
  {
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
      }
      m_current = m_commandStack.size();
      event->accept();
    }
    QLineEdit::keyPressEvent(event);
  }
}

#include "pythonextension.moc"

Q_EXPORT_PLUGIN2(pythonextension, Avogadro::PythonExtensionFactory)
