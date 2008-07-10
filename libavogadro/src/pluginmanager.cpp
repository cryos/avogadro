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

// Include static engine headers
#include "engines/bsdyengine.h"

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

      static QList<EngineFactory *> engineFactories;
      static QHash<QString, EngineFactory *> engineClassFactory;

      static QList<Tool *> tools;
      static QList<Extension *> extensions;
      
      static PluginDialog *dialog;
  };
  
  // the static members
  QList<PluginItem *> PluginManagerPrivate::plugins;
  QList<EngineFactory *> PluginManagerPrivate::engineFactories;
  QHash<QString, EngineFactory *> PluginManagerPrivate::engineClassFactory;
  QList<Tool *> PluginManagerPrivate::tools;
  QList<Extension *> PluginManagerPrivate::extensions;
  PluginDialog *PluginManagerPrivate::dialog;

  PluginManager::PluginManager(QObject *parent) : QObject(parent), d(new PluginManagerPrivate)
  {
    d->dialog = 0;
  }

  PluginManager::~PluginManager()
  {
    delete(d);
  }

  void PluginManager::loadEngineFactories()
  {
    // set the search paths
    QString prefixPath = QString(INSTALL_PREFIX) + '/'
      + QString(INSTALL_LIBDIR) + "/avogadro/engines";
    QStringList pluginPaths;
    pluginPaths << prefixPath;

    #ifdef WIN32
      pluginPaths << QCoreApplication::applicationDirPath() + "/engines";
    #endif

    // Krazy: Use QProcess:
    // http://doc.trolltech.com/4.3/qprocess.html#systemEnvironment
    if (getenv("AVOGADRO_ENGINES") != NULL)
      pluginPaths = QString(getenv("AVOGADRO_ENGINES")).split(':');

    // load static plugins first
    EngineFactory *bsFactory = qobject_cast<EngineFactory *>(new BSDYEngineFactory);
    if (bsFactory) {
      d->engineFactories.append(bsFactory);
      d->engineClassFactory[bsFactory->className()] = bsFactory;
    }
    else
      qDebug() << "Instantiation of the static ball and sticks plugin failed.";

    // now load plugins from paths
    QSettings settings;
    settings.beginGroup("plugins");
    settings.beginGroup("engines");
    foreach(const QString& path, pluginPaths)
    {
      QDir dir(path);
      qDebug() << "Searching for engines in" << path;
      foreach(const QString& fileName, dir.entryList(QDir::Files))
      {
        QPluginLoader loader(dir.absoluteFilePath(fileName));
        QObject *instance = loader.instance();
        EngineFactory *factory = qobject_cast<EngineFactory *>(instance);
        if (factory)
        {
          PluginItem *plugin = new PluginItem(PluginItem::EngineType, fileName, dir.absoluteFilePath(fileName));
          plugin->setName(factory->type());
         
          // add the engine to the engineFactories list only when it the plugin is enabled 
          if (settings.value(factory->type(), true).toBool())
          {
            plugin->setEnabled(true);
            d->engineFactories.append(factory);
            d->engineClassFactory[factory->className()] = factory;
          }
          else
            plugin->setEnabled(false);
          
          d->plugins.append(plugin);
        }
        else
          qDebug() << fileName << "failed to load." << loader.errorString();
      }
    }
    settings.endGroup();
    settings.endGroup();
  }
 
  void PluginManager::findTools()
  {
    QString prefixPath = QString(INSTALL_PREFIX) + '/'
      + QString(INSTALL_LIBDIR) + "/avogadro/tools";
    QStringList pluginPaths;
    pluginPaths << prefixPath;

    #ifdef WIN32
      pluginPaths << QCoreApplication::applicationDirPath() + "/tools";
    #endif

    // Krazy: Use QProcess:
    // http://doc.trolltech.com/4.3/qprocess.html#systemEnvironment
    if(getenv("AVOGADRO_TOOLS") != NULL)
    {
      pluginPaths = QString(getenv("AVOGADRO_TOOLS")).split(':');
    }

    foreach (const QString& path, pluginPaths)
    {
      QDir dir(path);
      foreach (const QString& fileName, dir.entryList(QDir::Files))
      {
        PluginItem *tool = new PluginItem(PluginItem::ToolType, fileName, dir.absoluteFilePath(fileName));
        d->plugins.append(tool);
      }
    }
  }
  
  void PluginManager::loadTools()
  {
    findTools();

    QSettings settings;
    settings.beginGroup("plugins");
    settings.beginGroup("tools");
    foreach (PluginItem *plugin, d->plugins)
    {
      if (plugin->type() == PluginItem::ToolType)
      {
        QPluginLoader loader(plugin->absoluteFilePath());
        QObject *instance = loader.instance();
        ToolFactory *factory = qobject_cast<ToolFactory *>(instance);
        if (factory)
        {
          Tool *tool = factory->createInstance(this);
          plugin->setName(tool->name());
          if (settings.value(tool->name(), true).toBool()) 
          {
            plugin->setEnabled(true);
            qDebug() << "Found Tool: " << tool->name() << " - " << tool->description() << " (enabled)";
            d->tools.append(tool);
          } 
          else
          {
            plugin->setEnabled(false);
            qDebug() << "Found Tool: " << tool->name() << " - " << tool->description() << " (diabled)";
            delete tool;
          }
        }
      }
    }
    settings.endGroup();
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
  
  const QList<EngineFactory *>& PluginManager::engineFactories() const
  {
    return d->engineFactories;
  }
    
  const QHash<QString, EngineFactory *>& PluginManager::engineClassFactory() const
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
    
  void PluginManager::writeSettings(QSettings &settings) const
  {
    qDebug() << "PluginManager::writeSettings()";

    // write the engine's isEnabled()
    settings.beginGroup("engines");
    foreach(PluginItem *plugin, d->plugins) 
    {
      if (plugin->type() == PluginItem::EngineType)
        settings.setValue(plugin->name(), plugin->isEnabled());
    }
    settings.endGroup();
 
    // write the tool's isEnabled()
    settings.beginGroup("tools");
    foreach(PluginItem *plugin, d->plugins) 
    {
      if (plugin->type() == PluginItem::ToolType)
        settings.setValue(plugin->name(), plugin->isEnabled());
    }
    settings.endGroup();
  }

  void PluginManager::readSettings(QSettings &settings)
  {
    Q_UNUSED(settings);

    qDebug() << "PluginManager::readwriteSettings()";
  }

  PluginManager pluginManager; // global instance
}

#include "pluginmanager.moc"
