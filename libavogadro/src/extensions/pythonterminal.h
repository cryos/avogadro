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

#ifndef PYTHONTERMINAL_H
#define PYTHONTERMINAL_H

#include <avogadro/extension.h>
#include <avogadro/primitive.h>
#include <avogadro/glwidget.h>

#include "../pythoninterpreter.h"
#include "../pythonscript.h"

//#include "ui_pythonterminalwidget.h"

#include <QWidget>
#include <QTextEdit>
#include <QList>
#include <QDir>
#include <QString>
#include <QDateTime>
#include <QFileInfo>
#include <QHash>

class QDockWidget;

namespace Avogadro {

  class PythonTerminalEdit;
  class PythonTerminal : public Extension
  {
    Q_OBJECT
      AVOGADRO_EXTENSION("Python Terminal", tr("Python Terminal"),
                         tr("Interactive python scripting terminal"))

    public:
      //! Constructor
      PythonTerminal(QObject *parent=0);
      //! Deconstructor
      virtual ~PythonTerminal();

      virtual QList<QAction *> actions() const;
      virtual QString menuPath(QAction *action) const;

      virtual QDockWidget * dockWidget();
      virtual QUndoCommand* performAction(QAction *action, GLWidget *widget);

      void setMolecule(Molecule *molecule);

    private:
      Molecule *m_molecule;
      QDockWidget *m_terminalDock;
      PythonTerminalEdit *m_terminalEdit;

  };

  class PythonTerminalEdit : public QTextEdit
  {
    Q_OBJECT

    public:
      PythonTerminalEdit(QWidget *parent = 0);
      virtual void keyPressEvent ( QKeyEvent * event );

      void setMolecule(Molecule *molecule); 

    private Q_SLOTS:
      void runCommand();
      void printPrompt();

      void setTextCursorToEnd();

    private:
      PythonInterpreter m_interpreter;
      Molecule *m_molecule;
      
      QList<QString> m_commandStack;
      int m_current; //!< current command
      
      QString m_lines;
      int m_cursorPos, m_indent;

  };

  class PythonTerminalFactory : public QObject, public PluginFactory
  {
      Q_OBJECT
      Q_INTERFACES(Avogadro::PluginFactory)
      AVOGADRO_EXTENSION_FACTORY(PythonTerminal)
  };

} // end namespace Avogadro

#endif
