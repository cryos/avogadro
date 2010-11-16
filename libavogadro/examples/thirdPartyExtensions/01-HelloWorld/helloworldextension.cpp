/**********************************************************************
  HelloWorldExtension

  Copyright (C) 2010 David C. Lonie

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

#include "helloworldextension.h"
#include "helloworlddialog.h"

#include <QtGui/QAction>

#include "helloworlddialog.h"

using namespace Avogadro;

namespace HelloWorld {

  HelloWorldExtension::HelloWorldExtension(QObject *parent)
    : Extension(parent),
      m_dialog(0) // Initialize the dialog pointer to 0, it will be
                  // created only when requested
  {
    // This block sets the text for menu entry
    QAction *action = new QAction(this);
    // Wrap all user visible strings in tr() so they can be translated
    action->setText(tr("0&1: Hello world..."));
    m_actions.append(action);
  }

  QList<QAction *> HelloWorldExtension::actions() const
  {
    return m_actions;
  }

  QString HelloWorldExtension::menuPath(QAction *) const
  {
    return tr("E&xtensions") + '>' + tr("&Tutorial");
  }

  QUndoCommand* HelloWorldExtension::performAction( QAction *, GLWidget * )
  {
    // Create the dialog if needed
    if (!m_dialog) {
      m_dialog = new HelloWorldDialog(qobject_cast<QWidget*>(parent()));
    }
    m_dialog->show();
    return NULL;
  }
}

// Include Qt moc'd headers
#include "helloworldextension.moc"

// Set up for the plugin to work correctly
Q_EXPORT_PLUGIN2(helloworldextension, HelloWorld::HelloWorldExtensionFactory)
