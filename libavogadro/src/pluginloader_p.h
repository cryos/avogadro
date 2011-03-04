/**********************************************************************
  PluginLoader -

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

#ifndef PLUGINLOADER_H
#define PLUGINLOADER_H

#include <QtCore/QList>
#include <QtCore/QSettings>

namespace Avogadro {

  class PluginItem;
  class PluginLoader : public QObject
  {
    Q_OBJECT
  public:
    static PluginLoader * instance();

    PluginItem * loadItem(const QString & fileName);
    void registerPlugin(const QString & fileName);

  private:
    PluginLoader();
//    QList<PluginFactory *> m_proxyFactories;
    //QSettings m_pluginInfo;
    bool m_checkEnabled;
    QSettings m_pluginInfo;
  };

}
#endif
