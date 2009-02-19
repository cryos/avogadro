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

#include <avogadro/pythoninterpreter.h>
#include <avogadro/pythonscript.h>

#include "ui_pythonterminalwidget.h"

#include <QWidget>
#include <QLineEdit>
#include <QList>
#include <QDir>
#include <QString>
#include <QDateTime>
#include <QFileInfo>
#include <QHash>

class QDockWidget;

namespace Avogadro {

  class PythonTerminalWidget;
  class PythonTerminal : public Extension
  {
    Q_OBJECT
      AVOGADRO_EXTENSION("Python Terminal", tr("Python Terminal"))

    public:
      //! Constructor
      PythonTerminal(QObject *parent=0);
      //! Deconstructor
      virtual ~PythonTerminal();

      virtual QString description() const { return QObject::tr("Python Terminal"); };

      virtual QList<QAction *> actions() const;
      virtual QString menuPath(QAction *action) const;

      virtual QDockWidget * dockWidget();
      virtual QUndoCommand* performAction(QAction *action, GLWidget *widget);

      void setMolecule(Molecule *molecule);

    private:
      Molecule *m_molecule;
      QDockWidget *m_terminalDock;
      PythonTerminalWidget *m_terminalWidget;
      PythonInterpreter m_interpreter;
      QString m_lines;

    private Q_SLOTS:
      void runCommand();

  };

  class PythonTerminalLineEdit : public QLineEdit
  {
    Q_OBJECT

    public:
    PythonTerminalLineEdit(QWidget *parent = 0);
    virtual void keyPressEvent ( QKeyEvent * event );

    private:
      QList<QString> m_commandStack;
      int m_current;

  };

  class PythonTerminalWidget : public QWidget
  {
    Q_OBJECT

    public:
      PythonTerminalWidget( QWidget *parent = 0 );
      Ui::PythonTerminalWidget ui;
      PythonTerminalLineEdit * inputLine;
  };

  class PythonTerminalFactory : public QObject, public PluginFactory
  {
      Q_OBJECT
      Q_INTERFACES(Avogadro::PluginFactory)

      AVOGADRO_EXTENSION_FACTORY(PythonTerminal,
                                 "Python Terminal",
        tr("Python Terminal"),
        tr("Extension to provide an interactive python terminal."))
  };

} // end namespace Avogadro

#endif
