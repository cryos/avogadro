/**********************************************************************
  DockExtension - interface for extensions which provide docks

  Copyright (C) 2010 Konstantin Tokarev

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

#include "dockextension.h"

namespace Avogadro {

  DockExtension::DockExtension(QObject *parent) : Extension(parent)
  {
  }

  DockExtension::~DockExtension()
  {
  }

  QList<QAction *> DockExtension::actions() const
  {
      return QList<QAction*>();
  }

  QUndoCommand* DockExtension::performAction(QAction *action, GLWidget *widget)
  {
      Q_UNUSED(action);
      Q_UNUSED(widget);
      return 0;
  }

  Qt::DockWidgetArea DockExtension::preferredDockArea() const
  {
      return Qt::RightDockWidgetArea;
  }

  int DockExtension::dockOrder() const
  {
      return 0;
  }
}
