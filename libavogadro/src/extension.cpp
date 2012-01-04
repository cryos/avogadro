/**********************************************************************
  Extension - Extension Class Interface

  Copyright (C) 2007-2008 Donald Ephraim Curtis

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

#include "extension.h"

#include "dockwidget.h"

namespace Avogadro {

  Extension::Extension(QObject *parent) : Plugin(parent)
  {
  }

  Extension::~Extension()
  {
  }

  Plugin::Type Extension::type() const
  {
    return Plugin::ExtensionType;
  }

  QString Extension::typeName() const
  {
    return tr("Extensions");
  }

  QString Extension::menuPath(QAction *action) const
  {
    Q_UNUSED(action); return QString();
  }

  bool Extension::usesNetwork(QAction *action) const
  {
    Q_UNUSED(action); return false;
  }

  QDockWidget * Extension::dockWidget()
  {
    if (!m_dockWidgets.isEmpty()) {
      return qobject_cast<QDockWidget*>(m_dockWidgets.first());
    }

    return NULL;
  }

  QList<DockWidget*> Extension::dockWidgets() const
  {
    return m_dockWidgets;
  }

  int Extension::numDockWidgets() const
  {
    return m_dockWidgets.size();
  }

  void Extension::setMolecule(Molecule *molecule)
  {
    Q_UNUSED(molecule);
  }

  void Extension::writeSettings(QSettings &settings) const
  {
    Q_UNUSED(settings);
  }

  void Extension::readSettings(QSettings &settings)
  {
    Q_UNUSED(settings);
  }

  int Extension::usefulness() const
  {
    return 0;
  }

}
