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

      connect(m_terminalWidget->ui.inputLine, SIGNAL(returnPressed()), this, SLOT(runCommand()));
    }

    return m_terminalDock;
  }

  void PythonExtension::setMolecule(Molecule *molecule)
  {
    m_interpreter.setMolecule(molecule);
  }

  void PythonExtension::runCommand()
  {
    QString text = m_terminalWidget->ui.inputLine->text();
    if(!text.isEmpty())
    {
      m_terminalWidget->ui.outputText->append(">>> " + text);
      QString result = m_interpreter.run(text);
      if(!result.isEmpty()) {
        m_terminalWidget->ui.outputText->append(result);
      }
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
  }
}

#include "pythonextension.moc"

Q_EXPORT_PLUGIN2( pythonextension, Avogadro::PythonExtensionFactory );
