/**********************************************************************
  Plugin - Avogadro Plugin Interface

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

#include "plugin.h"
#include <config.h>

#include <QAction>
#include <QIcon>

namespace Avogadro {

  /*
  class PluginPrivate
  {
    public:
      PluginPrivate() {}
  };
  */

  Plugin::Plugin() /*, d(new PluginPrivate)*/
  {
  }

  Plugin::~Plugin()
  {
    //delete d;
  }

  /*
  Plugin& Plugin::operator= (const Plugin &plugin)
  {
    Q_UNUSED(plugin)

    return *this;
  }
  */

  QString Plugin::description() const
  {
    return QObject::tr("No Description");
  }
  
  QWidget *Plugin::settingsWidget()
  {
    return NULL; // no settings widget
  }

  void Plugin::writeSettings(QSettings &settings) const
  {
    Q_UNUSED(settings)
    // needs to be implemented by the plugin
  }

  void Plugin::readSettings(QSettings &settings)
  {
    Q_UNUSED(settings)
    // needs to be implemented by the plugin
  }

} // end namespace Avogadro
