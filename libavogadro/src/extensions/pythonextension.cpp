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

  PythonExtension::PythonExtension( QObject *parent ) : Extension( parent ), m_terminalDock(0)
  {
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
    return tr("&Extensions");
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
      QString result = m_interpreter.run(text);
      if(!result.isEmpty()) {
        m_terminalWidget->ui.outputText->append(result);
      }
      m_terminalWidget->inputLine->clear();
    }
  }

  QUndoCommand* PythonExtension::performAction( QAction *action, GLWidget *widget )
  {
    Q_UNUSED(action);
    Q_UNUSED(widget);
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
