/**********************************************************************
  PluginManager - Class to handle dynamic loading/unloading of plugins

  Copyright (C) 2008 Tim Vandermeersch

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.sourceforge.net/>

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

#include <config.h>
#include "pluginmanager.h"
#include "plugindialog.h"

// Include static headers
#include "engines/bsdyengine.h"
#include "colors/elementcolor.h"

#include <QDir>
#include <QList>
#include <QStringList>
#include <QPluginLoader>
#include <QDebug>

namespace Avogadro {

  class PluginItemPrivate
  {
    public:
      int type;
      QString name;
      QString description;
      QString fileName;
      QString absoluteFilePath;
      bool enabled;
  };

  PluginItem::PluginItem() : d(new PluginItemPrivate)
  {
    d->type = -1;
    d->enabled = false;
  }
  
  PluginItem::PluginItem(int type, const QString &fileName, const QString &filePath) : d(new PluginItemPrivate)
  {
    d->type = type;
    d->fileName = fileName;
    d->absoluteFilePath = filePath;
    d->enabled = false;
  }
  
  PluginItem::~PluginItem()
  {
    delete d;
  }

  int PluginItem::type() const
  {
    return d->type;
  }
  
  QString PluginItem::name() const
  {
    return d->name;
  }
 
  QString PluginItem::description() const
  {
    return d->description;
  }
  
  QString PluginItem::fileName() const
  {
    return d->fileName;
  }
  
  QString PluginItem::absoluteFilePath() const
  {
    return d->absoluteFilePath;
  }
  
  bool PluginItem::isEnabled() const
  {
    return d->enabled;
  }

  void PluginItem::setType( int type )
  {
    d->type = type;
  }
  
  void PluginItem::setName( const QString &name )
  {
    d->name = name;
  }
 
  void PluginItem::setDescription( const QString &description )
  {
    d->description = description;
  }
  
  void PluginItem::setFileName( const QString &fileName )
  {
    d->fileName = fileName;
  }
  
  void PluginItem::setAbsoluteFilePath( const QString &filePath )
  {
    d->absoluteFilePath = filePath;
  }
  
  void PluginItem::setEnabled( bool enable )
  {
    d->enabled = enable;
  }

  class PluginManagerPrivate
  {
    public:
      PluginManagerPrivate() {}
      ~PluginManagerPrivate() {}

      static QList<PluginItem *> plugins;

      static QList<PluginFactory *> engineFactories;
      static QHash<QString, PluginFactory *> engineClassFactory;

      static QList<Tool *> tools;
      static QList<Extension *> extensions;
      static QList<Color *> colors;
      
      static PluginDialog *dialog;
  };
  
  // the static members
  QList<PluginItem *> PluginManagerPrivate::plugins;
  QList<PluginFactory *> PluginManagerPrivate::engineFactories;
  QHash<QString, PluginFactory *> PluginManagerPrivate::engineClassFactory;
  QList<Tool *> PluginManagerPrivate::tools;
  QList<Extension *> PluginManagerPrivate::extensions;
  QList<Color *> PluginManagerPrivate::colors;
  PluginDialog *PluginManagerPrivate::dialog;

  PluginManager::PluginManager(QObject *parent) : QObject(parent), d(new PluginManagerPrivate)
  {
    d->dialog = 0;
  }

  PluginManager::~PluginManager()
  {
    delete(d);
  }

  void PluginManager::loadPlugins()
  {
    ////////////////////////////// 
    // load static plugins first
    //////////////////////////////
    PluginFactory *bsFactory = qobject_cast<PluginFactory *>(new BSDYEngineFactory);
    if (bsFactory) {
      d->engineFactories.append(bsFactory);
      d->engineClassFactory[bsFactory->name()] = bsFactory;
    }
    else
      qDebug() << "Instantiation of the static ball and sticks plugin failed.";

    PluginFactory *elementFactory = qobject_cast<PluginFactory *>(new ElementColorFactory);
    if (elementFactory) 
    {
      Color *color = (Color*) elementFactory->createInstance(this);
      d->colors.append(color);
    }
    else
      qDebug() << "Instantiation of the static element color plugin failed.";

    ////////////////////////////// 
    // setup the paths
    //////////////////////////////
    QStringList pluginPaths;
 
    // Krazy: Use QProcess:
    // http://doc.trolltech.com/4.3/qprocess.html#systemEnvironment
    if (getenv("AVOGADRO_ENGINES") != NULL)
    {
      pluginPaths << QString(getenv("AVOGADRO_ENGINES")).split(':');
    }
    else
    {
      QString prefixPath = QString(INSTALL_PREFIX) + '/'
        + QString(INSTALL_LIBDIR) + "/avogadro/engines";
      pluginPaths << prefixPath;

      #ifdef WIN32
        pluginPaths << QCoreApplication::applicationDirPath() + "/engines";
      #endif
    }

    if(getenv("AVOGADRO_TOOLS") != NULL)
    {
      pluginPaths << QString(getenv("AVOGADRO_TOOLS")).split(':');
    }
    else
    {
      QString prefixPath = QString(INSTALL_PREFIX) + '/'
        + QString(INSTALL_LIBDIR) + "/avogadro/tools";
      pluginPaths << prefixPath;

      #ifdef WIN32
        pluginPaths << QCoreApplication::applicationDirPath() + "/tools";
      #endif
    }

    if (getenv("AVOGADRO_EXTENSIONS") != NULL) 
    {
      pluginPaths << QString(getenv("AVOGADRO_EXTENSIONS") ).split(':');
    }
    else
    {
      QString prefixPath = QString(INSTALL_PREFIX) + '/'
        + QString(INSTALL_LIBDIR) + "/avogadro/extensions";
      pluginPaths << prefixPath;

      #ifdef WIN32
        pluginPaths << QCoreApplication::applicationDirPath() + "/extensions";
      #endif
    }

    if(getenv("AVOGADRO_COLORS") != NULL)
    {
      pluginPaths << QString(getenv("AVOGADRO_TOOLS")).split(':');
    }
    else
    {
      QString prefixPath = QString(INSTALL_PREFIX) + '/'
        + QString(INSTALL_LIBDIR) + "/avogadro/colors";
      pluginPaths << prefixPath;

      #ifdef WIN32
        pluginPaths << QCoreApplication::applicationDirPath() + "/colors";
      #endif
    }

    ////////////////////////////// 
    // load the plugins 
    //////////////////////////////
    QSettings settings;
    settings.beginGroup("plugins");
    foreach (const QString& path, pluginPaths)
    {
      QDir dir(path);
#ifdef Q_WS_X11
      QStringList dirFilters;
      dirFilters << "*.so";
      dir.setNameFilters(dirFilters);
      dir.setFilter(QDir::Files | QDir::Readable);
#endif
      qDebug() << "Searching for plugins in" << path;
      foreach (const QString& fileName, dir.entryList(QDir::Files))
      {
        QPluginLoader loader(dir.absoluteFilePath(fileName));
        QObject *instance = loader.instance();
        PluginFactory *factory = qobject_cast<PluginFactory *>(instance);
        if (factory)
        {
          PluginItem *plugin = new PluginItem(factory->type(), fileName, dir.absoluteFilePath(fileName));
 
          if (factory->type() == Plugin::EngineType)
          {
            settings.beginGroup("engines");
            
            if (settings.value(factory->name(), true).toBool()) 
            {
              d->engineFactories.append(factory);
              d->engineClassFactory[factory->name()] = factory;
              plugin->setEnabled(true);
            } 
            else
              plugin->setEnabled(false);

            settings.endGroup();
          }
 
          if (factory->type() == Plugin::ToolType)
          {
            settings.beginGroup("tools");
            
            if (settings.value(factory->name(), true).toBool()) 
            {
              Tool *tool = (Tool*) factory->createInstance(this);
              d->tools.append(tool);
              plugin->setEnabled(true);
            } 
            else
              plugin->setEnabled(false);

            settings.endGroup();
          }
          else if (factory->type() == Plugin::ExtensionType)
          {
            settings.beginGroup("extensions");
            
            if (settings.value(factory->name(), true).toBool()) 
            {
              Extension *extension = (Extension*) factory->createInstance(/*this*/);
              d->extensions.append(extension);
              plugin->setEnabled(true);
            } 
            else
              plugin->setEnabled(false);

            settings.endGroup();
          }
          else if (factory->type() == Plugin::ColorType)
          {
            settings.beginGroup("colors");
            
            if (settings.value(factory->name(), true).toBool()) 
            {
              Color *color = (Color*) factory->createInstance(this);
              d->colors.append(color);
              plugin->setEnabled(true);
            } 
            else
              plugin->setEnabled(false);

            settings.endGroup();
          }
 
          if (plugin)
          {
            plugin->setName(factory->name());
            plugin->setDescription(factory->description());
            d->plugins.append(plugin);
          }
        }
        else
          qDebug() << fileName << "failed to load. " << loader.errorString();
 
      }
    }
    settings.endGroup();
  }

  QList<PluginItem *> PluginManager::plugins( int type )
  {
    QList<PluginItem *> list;

    foreach (PluginItem *plugin, d->plugins)
    {
      if (plugin->type() == type)
      {
        list.append(plugin);
      }
    }
 
    return list;
  }
    
  void PluginManager::showDialog()
  {
    if (!d->dialog)
      d->dialog = new PluginDialog();

    d->dialog->show();
  }
  
  const QList<PluginFactory *>& PluginManager::engineFactories() const
  {
    return d->engineFactories;
  }
    
  const QHash<QString, PluginFactory *>& PluginManager::engineClassFactory() const
  {
    return d->engineClassFactory;
  }

  const QList<Tool *>& PluginManager::tools() const
  {
    return d->tools;
  }
  
  const QList<Extension *>& PluginManager::extensions() const
  {
    return d->extensions;
  }
    
  const QList<Color *>& PluginManager::colors() const
  {
    return d->colors;
  }
  
  void PluginManager::writeSettings(QSettings &settings) const
  {
    // write the engine's isEnabled()
    settings.beginGroup("engines");
    foreach(PluginItem *plugin, d->plugins) 
    {
      if (plugin->type() == Plugin::EngineType)
        settings.setValue(plugin->name(), plugin->isEnabled());
    }
    settings.endGroup();
 
    // write the tool's isEnabled()
    settings.beginGroup("tools");
    foreach(PluginItem *plugin, d->plugins) 
    {
      if (plugin->type() == Plugin::ToolType)
        settings.setValue(plugin->name(), plugin->isEnabled());
    }
    settings.endGroup();

    // write the extension's isEnabled()
    settings.beginGroup("extensions");
    foreach(PluginItem *plugin, d->plugins) 
    {
      if (plugin->type() == Plugin::ExtensionType)
        settings.setValue(plugin->name(), plugin->isEnabled());
    }
    settings.endGroup();
 
  }

  PluginManager pluginManager; // global instance
}

#include "pluginmanager.moc"
