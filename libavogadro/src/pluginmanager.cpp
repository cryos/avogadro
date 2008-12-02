/**********************************************************************
  PluginManager - Class to handle dynamic loading/unloading of plugins

  Copyright (C) 2008 Donald Ephraim Curtis
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
      QString name;
      QString description;
      QString fileName;
      QString absoluteFilePath;
      Plugin::Type type;
      PluginFactory *factory;
      bool enabled;
  };

  PluginItem::PluginItem() : d(new PluginItemPrivate)
  {
    d->type = Plugin::OtherType;
    d->enabled = false;
  }
  
  PluginItem::PluginItem(
      const QString &name, 
      const QString &description,
      Plugin::Type type, 
      const QString &fileName, 
      const QString &filePath, 
      PluginFactory *factory,
      bool enabled
      ) : d(new PluginItemPrivate)
  {
    d->name = name;
    d->description = description;
    d->type = type;
    d->fileName = fileName;
    d->absoluteFilePath = filePath;
    d->enabled = enabled;
    d->factory = factory;
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

  PluginFactory *PluginItem::factory() const
  {
    return d->factory;
  }

  void PluginItem::setType( Plugin::Type type )
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

  void PluginItem::setFactory( PluginFactory *factory)
  {
    d->factory = factory;
  }

  class PluginManagerPrivate
  {
    public:
      PluginManagerPrivate() : 
        toolsLoaded(false), 
        extensionsLoaded(false),
        colorsLoaded(false) {}
      ~PluginManagerPrivate() {}

      static QList<PluginItem *> items;

      bool toolsLoaded;
      QList<Tool *> tools;
      bool extensionsLoaded;
      QList<Extension *> extensions;
      bool colorsLoaded;
      QList<Color *> colors;

      PluginDialog *dialog;

      static QVector<QList<PluginItem *> > &m_items();
      static QVector<QList<PluginFactory *> > &m_enabledFactories();
      static QVector<QList<PluginFactory *> > &m_disabledFactories();

  };

  PluginManager::PluginManager(QObject *parent) : QObject(parent), d(new PluginManagerPrivate)
  {
    d->dialog = 0;
  }

  PluginManager::~PluginManager()
  {
    if(d->dialog) {
      d->dialog->deleteLater();
    }

    QSettings settings;
    writeSettings(settings);
    delete(d);
  }

  QList<Extension *> PluginManager::extensions(QObject *parent) const
  {
    loadFactories();
    if(d->extensionsLoaded)
    {
      return d->extensions;
    }

    foreach(PluginFactory *factory, factories(Plugin::ExtensionType))
    {
      Extension *extension = static_cast<Extension *>(factory->createInstance(parent));
      d->extensions.append(extension);
    }
    
    d->extensionsLoaded = true;

    return d->extensions;
  }

  QList<Tool *> PluginManager::tools(QObject *parent) const
  {
    loadFactories();
    if(d->toolsLoaded)
    {
      return d->tools;
    }

    foreach(PluginFactory *factory, factories(Plugin::ToolType))
    {
      Tool *tool = static_cast<Tool *>(factory->createInstance(parent));
      d->tools.append(tool);
    }

    d->toolsLoaded = true;
    return d->tools;
  }

  QList<Color *> PluginManager::colors(QObject *parent) const
  {
    loadFactories();
    if(d->colorsLoaded)
    {
      return d->colors;
    }

    foreach(PluginFactory *factory, factories(Plugin::ColorType))
    {
      Color *color = static_cast<Color *>(factory->createInstance(parent));
      d->colors.append(color);
    }

    d->colorsLoaded = true;
    return d->colors;
  }

  PluginFactory * PluginManager::factory(const QString &name, Plugin::Type type)
  {
    loadFactories();
    if(type < Plugin::TypeCount)
    {
      foreach(PluginFactory *factory, PluginManagerPrivate::m_enabledFactories()[type])
      {
        if(factory->name() == name)
        {
          return factory;
        }
      }
    }

    return 0;
  }

  QVector<QList<PluginItem *> > &PluginManagerPrivate::m_items()
  {
    static QVector<QList<PluginItem *> > items;

    if(items.size() < Plugin::TypeCount)
    {
      items.resize(Plugin::TypeCount);
    }

    return items;
  }

  QVector<QList<PluginFactory *> > &PluginManagerPrivate::m_enabledFactories()
  {
    static QVector<QList<PluginFactory *> > factories;

    if(factories.size() < Plugin::TypeCount)
    {
      factories.resize(Plugin::TypeCount);
    }

    return factories;
  }

  QVector<QList<PluginFactory *> > &PluginManagerPrivate::m_disabledFactories()
  {
    static QVector<QList<PluginFactory *> > factories;

    if(factories.size() < Plugin::TypeCount)
    {
      factories.resize(Plugin::TypeCount);
    }

    return factories;
  }

  void PluginManager::loadFactories()
  {
    static bool factoriesLoaded = false;
    if(factoriesLoaded)
    {
      return;
    }

    QVector<QList<PluginFactory *> > &ef = PluginManagerPrivate::m_enabledFactories();
    QVector<QList<PluginFactory *> > &df = PluginManagerPrivate::m_disabledFactories();

    ////////////////////////////// 
    // load static plugins first
    //////////////////////////////
    PluginFactory *bsFactory = qobject_cast<PluginFactory *>(new BSDYEngineFactory);
    if (bsFactory) {
      ef[bsFactory->type()].append(bsFactory);
      //d->engineClassFactory[bsFactory->name()] = bsFactory;
    }
    else {
      qDebug() << "Instantiation of the static ball and sticks plugin failed.";
    }

    PluginFactory *elementFactory = qobject_cast<PluginFactory *>(new ElementColorFactory);
    if (elementFactory) 
    {
      ef[elementFactory->type()].append(elementFactory);
    }
    else {
      qDebug() << "Instantiation of the static element color plugin failed.";
    }

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
      pluginPaths << QString(getenv("AVOGADRO_COLORS")).split(':');
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
    settings.beginGroup("Plugins");
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
          settings.beginGroup(QString::number(factory->type()));
          PluginItem *item = new PluginItem(factory->name(), factory->description(), factory->type(), fileName, dir.absoluteFilePath(fileName), factory);
          if(settings.value(factory->name(), true).toBool())
          {
            ef[factory->type()].append(factory);
            item->setEnabled(true);
          }
          else
          {
            df[factory->type()].append(factory);
            item->setEnabled(false);
          }
          PluginManagerPrivate::m_items()[factory->type()].append(item);
          settings.endGroup();
        }
        else
        {
          qDebug() << fileName << "failed to load. " << loader.errorString();
        }
      }
    }
    settings.endGroup();
    factoriesLoaded = true;
  }

  QList<PluginFactory *> PluginManager::factories( Plugin::Type type )
  {
    if (type < PluginManagerPrivate::m_enabledFactories().size() )
    {
      loadFactories();
      return PluginManagerPrivate::m_enabledFactories()[type];
    }

    return QList<PluginFactory *>();
  }
    
  void PluginManager::showDialog()
  {
    if (!d->dialog)
      d->dialog = new PluginDialog();

    d->dialog->show();
  }
  

  QList<PluginItem *> PluginManager::pluginItems(Plugin::Type type)
  {
    return PluginManagerPrivate::m_items()[type];
  }
  
  void PluginManager::writeSettings(QSettings &settings)
  {
    // write the engine's isEnabled()
    settings.beginGroup("Plugins");
    for(int i=0; i<Plugin::TypeCount; i++)
    {
      settings.beginGroup(QString::number(i));
      foreach(PluginItem *item, PluginManagerPrivate::m_items()[i])
      {
        settings.setValue(item->name(), item->isEnabled());
      }
      settings.endGroup();
    }
    settings.endGroup();
  }

}

#include "pluginmanager.moc"
