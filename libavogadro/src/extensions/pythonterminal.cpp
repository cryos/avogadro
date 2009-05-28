/**********************************************************************
  Python - Gives us some Python helper stuff

  Copyright (C) 2008 by Donald Ephraim Curtis
  Copyright (C) 2008 by Tim Vandermeersch

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.openmolecules.net/>

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

#include "pythonterminal.h"
#include "../config.h"

#include <avogadro/molecule.h>

#include <QDockWidget>
#include <QCompleter>
#include <QKeyEvent>
#include <QUndoCommand>

#include <QDebug>

#include "highlighter.h"

using namespace std;
using namespace boost::python;

namespace Avogadro
{

  PythonTerminal::PythonTerminal( QObject *parent ) : Extension( parent ), m_molecule(0), m_terminalDock(0)
  {
  }


  PythonTerminal::~PythonTerminal()
  {
    //if (m_terminalDock)
      //m_terminalDock->deleteLater();
  }

  QList<QAction *> PythonTerminal::actions() const
  {
    return QList<QAction*>();
  }

  // allows us to set the intended menu path for each action
  QString PythonTerminal::menuPath(QAction *action) const
  {
    Q_UNUSED(action);
    return QString();
  }

  QDockWidget * PythonTerminal::dockWidget()
  {
    if(!m_terminalDock)
    {
      m_terminalDock = new QDockWidget( tr("Python Terminal"), qobject_cast<QWidget *>(parent()) );
      m_terminalDock->setObjectName( tr("pythonTerminalDock") );
      m_terminalEdit = new PythonTerminalEdit();
      m_terminalDock->setWidget(m_terminalEdit);

      new Highlighter(m_terminalEdit->document());
    }

    return m_terminalDock;
  }

  void PythonTerminal::setMolecule(Molecule *molecule)
  {
    m_molecule = molecule;
    m_terminalEdit->setMolecule(molecule);
  }

  QUndoCommand* PythonTerminal::performAction( QAction *action, GLWidget *widget )
  {
    Q_UNUSED(action);
    Q_UNUSED(widget);
    return 0;
  }

  /*
  PythonTerminalWidget::PythonTerminalWidget( QWidget *parent ) : QWidget(parent)
  {
    ui.setupUi(this);
    QFont font;
    font.setFamily(QString::fromUtf8("Courier New"));
    inputLine = new PythonTerminalLineEdit(this);
    inputLine->setObjectName(QString::fromUtf8("inputLine"));
    inputLine->setFont(font);

     ui.outputText->setFont(font);

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
  */

  PythonTerminalEdit::PythonTerminalEdit(QWidget *parent) : QTextEdit(parent), 
      m_current(0), m_cursorPos(0), m_indent(0)
  {
    // Load the saved commands
    QSettings settings;
    int size = settings.beginReadArray("pythonCommands");
    for (int i = 0; i < size; ++i) {
      settings.setArrayIndex(i);
      m_commandStack.append( settings.value("command").toString() );
    }
    settings.endArray();
 
    // set the font
    QFont font;
    font.setFamily(QString::fromUtf8("Courier New"));
    setFont(font);
         
    printPrompt();

    connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(setTextCursorToEnd()));
  }

  void PythonTerminalEdit::setMolecule(Molecule *molecule)
  {
    m_molecule = molecule;
    m_interpreter.setMolecule(m_molecule);
  }
      
  void PythonTerminalEdit::setTextCursorToEnd()
  {
    QTextCursor cursor(textCursor());
    if (cursor.position() < m_cursorPos)
      cursor.setPosition(m_cursorPos);
    setTextCursor(cursor);
  }

  void PythonTerminalEdit::runCommand()
  {
    int indent = 0;
    QString text = toPlainText();
    text = text.right(text.size() - m_cursorPos);

    QString indentString;

    if(!text.trimmed().isEmpty()) {
      QString line = text;
      while (line.startsWith(' ')) {
        line.remove(0, 1);
        indent++;
      }
      line += text.trimmed();
 
      if (line.endsWith(':')) {
        indent += 2;
        m_lines.append(text);
        append(QLatin1String(""));
      } else {
        if (indent && !m_lines.isEmpty()) {
          m_lines.append(text);
          append(QLatin1String(""));
        } else {
          QString result = m_interpreter.exec(text);
          append(result);
        }
      }
      for (int i = 0; i < indent; ++i)
        indentString += QLatin1String(" ");
      // Always update the molecule when running commands from the terminal widget
      m_molecule->update();
    } else {
      QString result = m_interpreter.exec(m_lines);
      append(result);
      m_lines.clear();
    }

    m_indent = indent;
    printPrompt();
  }

  void PythonTerminalEdit::printPrompt()
  {
    QTextCursor cursor(textCursor());
    cursor.clearSelection();
    cursor.movePosition(QTextCursor::End);

    if (m_lines.isEmpty()) {
      cursor.insertText(QLatin1String(">>> "));
    } else  {
      QString indentString;
      for (int i = 0; i < m_indent; ++i)
        indentString += QLatin1String(" ");
      cursor.insertText(QLatin1String("... ") + indentString);
    }
    
    m_cursorPos = cursor.position();
    m_cursorPos -= m_indent;
    setTextCursor(cursor);
  }

  void PythonTerminalEdit::keyPressEvent(QKeyEvent *event)
  {
    if(event->key() == Qt::Key_Up)
    {
      if(m_commandStack.size())
      {
        m_current--;
        if(m_current < 0) {
          m_current = m_commandStack.size();
        }

        if(m_current == m_commandStack.size()) {
          // we've reached the first command, display empty prompt
          setText(toPlainText().left(m_cursorPos));
          QTextCursor cursor(textCursor());
          cursor.movePosition(QTextCursor::End);
          setTextCursor(cursor);
        } else {
          // diplay cached command
          setText(toPlainText().left(m_cursorPos));
          QTextCursor cursor(textCursor());
          cursor.movePosition(QTextCursor::End);
          cursor.insertText(m_commandStack.at(m_current));
          cursor.movePosition(QTextCursor::End);
          setTextCursor(cursor);
        }

      }
      event->accept();
      return;
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
          // we've reached the last command, display empty prompt
          setText(toPlainText().left(m_cursorPos));
          QTextCursor cursor(textCursor());
          cursor.movePosition(QTextCursor::End);
          setTextCursor(cursor);
        }
        else
        {
          // diplay cached command
          setText(toPlainText().left(m_cursorPos));
          QTextCursor cursor(textCursor());
          cursor.movePosition(QTextCursor::End);
          cursor.insertText(m_commandStack.at(m_current));
          cursor.movePosition(QTextCursor::End);
          setTextCursor(cursor);
        }

      }
      event->accept();
    }
    else if(event->key() == Qt::Key_Return)
    {
      QString text = toPlainText();
      QString t = text.right(text.size() - m_cursorPos);

      if(!t.isEmpty())
      {
        m_commandStack.append(t);
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
      runCommand();
      event->accept();
      return;
    }
    else if(event->key() == Qt::Key_Backspace)
    {
      QTextCursor cursor(textCursor());
      if (cursor.position() <= m_cursorPos) {
        event->accept();
        return;
      }
    }

    QTextEdit::keyPressEvent(event);
  }
}

Q_EXPORT_PLUGIN2(pythonterminal, Avogadro::PythonTerminalFactory)

