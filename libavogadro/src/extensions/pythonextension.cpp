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

#include <QDockWidget>
#include <QDebug>

using namespace std;
using namespace OpenBabel;

namespace Avogadro
{

  PythonScript::PythonScript(QDir dir, QString fileName)
  {
    m_fileName = fileName;
    m_dir = dir;
    m_fileInfo.setFile(dir, fileName);
    m_lastModified = m_fileInfo.lastModified();

    QString moduleName = fileName.left(fileName.size()-3);

    m_moduleName = moduleName;
//    object script_module(handle<>(PyImport_ImportModule(moduleName.toAscii().data())));
    try
    {
      // these do the same thing one is just a boost helper function
      // the other just wraps in the same way
//      m_module = object(handle<>(PyImport_ImportModule(moduleName.toAscii().data())));
      m_module = import(moduleName.toAscii().data());
    }
    catch(error_already_set const &)
    {
    }
  }

  QString PythonScript::moduleName() const
  {
    return m_moduleName;
  }

  object PythonScript::module() const
  {
    // check if the file has changed and reload the module
    m_fileInfo.refresh();
    if(m_fileInfo.lastModified() > m_lastModified)
    {
      try
      {
        m_module = object(handle<>(PyImport_ReloadModule(m_module.ptr())));
      }
      catch(error_already_set const &)
      {
      }
      m_lastModified = m_fileInfo.lastModified();
    }
    return m_module;
  }

  enum PythonIndex
  {
    ScriptIndex = 0
  };

  PythonExtension::PythonExtension( QObject *parent ) : Extension( parent ), m_terminalDock(0)
  {
    // create this directory for the user
    QDir pluginDir = QDir::home();
    if(!pluginDir.cd(".avogadro"))
    {
      if(!pluginDir.mkdir(".avogadro"))
      {
        return;
      }
      if(!pluginDir.cd(".avogadro"))
      {
        return;
      }
    }

    if(!pluginDir.cd("scripts"))
    {
      if(!pluginDir.mkdir("scripts"))
      {
        return;
      }
      if(!pluginDir.cd("scripts"))
      {
        return;
      }
    }

    loadScripts(pluginDir);

//    loadScripts(Library::prefix() + "/share/libavogadro-" + Library::version() + "/scripts");
  }

  void PythonExtension::loadScripts(QDir dir)
  {
    // add it to the search path
    m_interpreter.addSearchPath(dir.canonicalPath());

    QStringList filters;
    filters << "*.py";
    dir.setNameFilters(filters);
    dir.setFilter(QDir::Files | QDir::Readable);

    foreach(QString file, dir.entryList())
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

        
//        dict local;
//        local["test"] = script.module();

  //      qDebug() << m_interpreter.run("import test", local);
//        qDebug() << m_interpreter.run("print test", local);
//        qDebug() << m_interpreter.run("print dir(test)", local);
  //      qDebug() << m_interpreter.run("print sys.path", local);
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
    }
  }

  QUndoCommand* PythonExtension::performAction( QAction *action, GLWidget *widget )
  {
    Q_UNUSED(widget);
    int i = action->data().toInt();

    // are we running a script?
    if(i >= ScriptIndex)
    {
      PythonScript script = m_scripts.at(i - ScriptIndex);
//      qDebug() << "Executing Script" << script.name();
      dict local;
      local[script.moduleName().toStdString()] = script.module();
      QString output = m_interpreter.exec(script.moduleName() + ".extension()", local);
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

Q_EXPORT_PLUGIN2( pythonextension, Avogadro::PythonExtensionFactory );
