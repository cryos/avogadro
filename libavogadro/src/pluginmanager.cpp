/**********************************************************************
  PluginManager - Class to handle dynamic loading/unloading of plugins

  Copyright (C) 2008 Donald Ephraim Curtis
  Copyright (C) 2008,2009 Tim Vandermeersch
  Copyright (C) 2008,2009 Marcus D. Hanwell

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

#include "pluginmanager.h"
#include "config.h" // krazy:exclude=includes

#ifdef ENABLE_PYTHON
  #include "pythontool_p.h"
  #include "pythonengine_p.h"
  #include "pythonextension_p.h"
#endif

#include <avogadro/engine.h>
#include <avogadro/tool.h>
#include <avogadro/extension.h>
#include <avogadro/color.h>

// Include static headers
#include "engines/bsdyengine.h"
#include "colors/elementcolor.h"

#include <QSettings>
#include <QDir>
#include <QList>
#include <QStringList>
#include <QPluginLoader>
#include <QDebug>
#include <QProcess>
#include <QFileInfo>

namespace Avogadro {

  class PluginItemPrivate
  {
    public:
      QString name;
      QString identifier;
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
      const QString &identifier,
      const QString &description,
      Plugin::Type type,
      const QString &fileName,
      const QString &filePath,
      PluginFactory *factory,
      bool enabled) : d(new PluginItemPrivate)
  {
    d->name = name;
    d->identifier = identifier;
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

  QString PluginItem::identifier() const
  {
    return d->identifier;
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

  void PluginItem::setIdentifier( const QString &identifier )
  {
    d->identifier = identifier;
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

      QStringList searchDirs;

      bool toolsLoaded;
      QList<Tool *> tools;
      bool extensionsLoaded;
      QList<Extension *> extensions;
      bool colorsLoaded;
      QList<Color *> colors;

      static bool factoriesLoaded;
      static QVector<QList<PluginItem *> > &m_items();
      static QVector<QList<PluginFactory *> > &m_enabledFactories();
      static QVector<QList<PluginFactory *> > &m_disabledFactories();

  };

  bool PluginManagerPrivate::factoriesLoaded = false;

  // Sort tools based on "usefulness" (currently unused)
  // defined in tool.cpp
  extern bool toolGreaterThan(const Tool *first, const Tool *second);

  // Sort extensions based on "usefulness" (currently unused)
  bool extensionGreaterThan(const Extension *first, const Extension *second)
  {
    return first->usefulness() < second->usefulness();
  }

  // Sort colors based on their translated names
  bool colorGreaterThan(const Color *first, const Color *second)
  {
    if (first->identifier() == "ElementColor")
      return true; // always the top!
    else if (second->identifier() == "ElementColor")
      return false; // always the top!

    if (first->identifier() == "CustomColor")
      return false; // always the bottom!
    else if (second->identifier() == "CustomColor")
      return true; // always the bottom!

    // locale aware returns less-than, greater-than, or 0 
    // Required for sorting lists.
    return (QString::localeAwareCompare(first->name(), second->name()) < 0);
  }

  PluginManager::PluginManager(QObject *parent) : QObject(parent),
                                                  d(new PluginManagerPrivate)
  {
    // This is where we compile a list of directories that will be searched
    // for plugins. This is quite dependent up on operating system

    // Environment variables can override default paths
    foreach (const QString &variable, QProcess::systemEnvironment()) {
      QStringList split1 = variable.split('=');
      if (split1[0] == "AVOGADRO_PLUGINS")
        foreach (const QString &path, split1[1].split(':'))
          d->searchDirs << path;
    }
    // If no environment variables are set then find the plugins
    if (!d->searchDirs.size()) {
      // Make it relative
      d->searchDirs << QCoreApplication::applicationDirPath()
                     + "/../" + QString(INSTALL_LIB_DIR)
                     + "/" + QString(INSTALL_PLUGIN_DIR);
    }

    // Now to search for the plugins in home directories
#if defined(Q_WS_X11)
    d->searchDirs << QDir::homePath() + "/."
                   + QString(INSTALL_PLUGIN_DIR) + "/plugins";
#elif defined(Q_WS_MAC)
    d->searchDirs << QDir::homePath() + "/Library/Application Support/"
                   + QString(INSTALL_PLUGIN_DIR) + "/Plugins";
#elif defined(WIN32)
    const QString appdata = qgetenv("APPDATA");
    d->searchDirs << appdata + "/" + QString(INSTALL_PLUGIN_DIR);
#endif
  }

  PluginManager::~PluginManager()
  {
    QSettings settings;
    writeSettings(settings);
    delete(d);
  }

  PluginManager* PluginManager::instance()
  {
    static PluginManager *obj = 0;

    if (!obj)
      obj = new PluginManager();

    return obj;
  }

  QList<Extension *> PluginManager::extensions(QObject *parent)
  {
    loadFactories();
    if(d->extensionsLoaded)
      return d->extensions;

    foreach(PluginFactory *factory, factories(Plugin::ExtensionType)) {
      Extension *extension = static_cast<Extension *>(factory->createInstance(parent));
      d->extensions.append(extension);
    }

    //    qSort(d->extensions.begin(), d->extensions.end(), extensionGreaterThan);

    d->extensionsLoaded = true;

    return d->extensions;
  }

  QList<Tool *> PluginManager::tools(QObject *parent)
  {
    loadFactories();
    if(d->toolsLoaded)
      return d->tools;

    foreach(PluginFactory *factory, factories(Plugin::ToolType)) {
      Tool *tool = static_cast<Tool *>(factory->createInstance(parent));
      d->tools.append(tool);
    }

    qSort(d->tools.begin(), d->tools.end(), toolGreaterThan);

    d->toolsLoaded = true;
    return d->tools;
  }

  QList<QString> PluginManager::scripts(const QString &type)
  {
    QList<QString> scripts;

    // create this directory for the user if it does not exist
    QDir dir = QDir::home();
    QStringList filters;
    filters << "*.py";
    dir.setNameFilters(filters);
    dir.setFilter(QDir::Files | QDir::Readable);

    bool failed = false;
#ifdef Q_WS_MAC
    dir.cd("Library/Application Support");
    if (!dir.cd("Avogadro")) {
      if (!dir.mkdir("Avogadro")) failed = true;
      if (!dir.cd("Avogadro")) failed = true;
    }
#else
  #ifdef WIN32
    dir = QCoreApplication::applicationDirPath();
  #else
    if(!dir.cd(".avogadro")) {
      if (!dir.mkdir(".avogadro")) failed = true;
      if (!dir.cd(".avogadro")) failed = true;
    }
  #endif
#endif

    if(!dir.cd(type + "Scripts")) {
      if (!dir.mkdir(type + "Scripts")) failed = true;
      if (!dir.cd(type + "Scripts")) failed = true;
    }

    foreach (const QString& file, dir.entryList())
      scripts.append(QString(dir.canonicalPath() + '/' + file));

#ifndef WIN32
    // Now for the system wide Python scripts
    QString systemScriptsPath = QString(INSTALL_PREFIX) + '/'
      + "share/libavogadro/" + type + "Scripts";
    if (dir.cd(systemScriptsPath))
      foreach (const QString& file, dir.entryList())
        scripts.append(QString(dir.canonicalPath() + '/' + file));
#endif

    return scripts;
  }

  QList<QString> PluginManager::toolScripts()
  {
    return scripts("tool");
  }

  QList<QString> PluginManager::engineScripts()
  {
    return scripts("engine");
  }

  QList<QString> PluginManager::extensionScripts()
  {
    return scripts("extension");
  }

  QList<Color *> PluginManager::colors(QObject *parent)
  {
    loadFactories();
    if(d->colorsLoaded)
      return d->colors;

    foreach(PluginFactory *factory, factories(Plugin::ColorType))  {
      Color *color = static_cast<Color *>(factory->createInstance(parent));
      d->colors.append(color);
    }

    qSort(d->colors.begin(), d->colors.end(), colorGreaterThan);

    d->colorsLoaded = true;
    return d->colors;
  }

  PluginFactory * PluginManager::factory(const QString &id, Plugin::Type type)
  {
    loadFactories();
    if(type < Plugin::TypeCount) {
      foreach(PluginFactory *factory,
              PluginManagerPrivate::m_enabledFactories()[type]) {
        if(factory->identifier() == id) {
          return factory;
        }
      }
    }

    return 0;
  }

  Extension* PluginManager::extension(const QString &id, QObject *parent)
  {
    loadFactories();
    foreach(PluginFactory *factory, factories(Plugin::ExtensionType)) {
      if (factory->identifier() == id) {
        Extension *extension = static_cast<Extension *>(factory->createInstance(parent));
        return extension;
      }
    }

    return 0;
  }

  Tool* PluginManager::tool(const QString &id, QObject *parent)
  {
    loadFactories();
    foreach(PluginFactory *factory, factories(Plugin::ToolType)) {
      if (factory->identifier() == id) {
        Tool *tool = static_cast<Tool *>(factory->createInstance(parent));
        return tool;
      }
    }

    return 0;
  }

  Color* PluginManager::color(const QString &id, QObject *parent)
  {
    loadFactories();
    foreach(PluginFactory *factory, factories(Plugin::ColorType)) {
      if (factory->identifier() == id) {
        Color *color = static_cast<Color *>(factory->createInstance(parent));
        return color;
      }
    }

    return 0;
  }

  Engine* PluginManager::engine(const QString &id, QObject *parent)
  {
    loadFactories();

    foreach(PluginFactory *factory, factories(Plugin::EngineType)) {
      if (factory->identifier() == id) {
        Engine *engine = static_cast<Engine *>(factory->createInstance(parent));
        return engine;
      }
    }

    return 0;
  }

  QList<QString> PluginManager::names(Plugin::Type type)
  {
    loadFactories();

    QList<QString> names;
    foreach(PluginFactory *factory, factories(type))
      names.append(factory->name());

    return names;
  }

  QList<QString> PluginManager::identifiers(Plugin::Type type)
  {
    loadFactories();

    QList<QString> ids;
    foreach(PluginFactory *factory, factories(type))
      ids.append(factory->identifier());

    return ids;
  }

  QList<QString> PluginManager::descriptions(Plugin::Type type)
  {
    loadFactories();

    QList<QString> descriptions;
    foreach(PluginFactory *factory, factories(type))
      descriptions.append(factory->description());

    return descriptions;
  }

  QVector<QList<PluginItem *> > &PluginManagerPrivate::m_items()
  {
    static QVector<QList<PluginItem *> > items;

    if(items.size() < Plugin::TypeCount)
      items.resize(Plugin::TypeCount);

    return items;
  }

  QVector<QList<PluginFactory *> > &PluginManagerPrivate::m_enabledFactories()
  {
    static QVector<QList<PluginFactory *> > factories;

    if(factories.size() < Plugin::TypeCount)
      factories.resize(Plugin::TypeCount);

    return factories;
  }

  QVector<QList<PluginFactory *> > &PluginManagerPrivate::m_disabledFactories()
  {
    static QVector<QList<PluginFactory *> > factories;

    if(factories.size() < Plugin::TypeCount)
      factories.resize(Plugin::TypeCount);

    return factories;
  }

  void PluginManager::loadFactories(const QString& dir)
  {
    if (PluginManagerPrivate::factoriesLoaded)
      return;

    if (!dir.isEmpty()) {
      QSettings settings;
      settings.beginGroup("ExtraPlugins");
      loadPluginDir(dir, settings);
      settings.endGroup(); // ExtraPlugins
    }

    QVector< QList<PluginFactory *> > &ef = PluginManagerPrivate::m_enabledFactories();

    // Load the static plugins first
    PluginFactory *bsFactory = qobject_cast<PluginFactory *>(new BSDYEngineFactory);
    if (bsFactory)
      ef[bsFactory->type()].append(bsFactory);
    else
      qDebug() << "Instantiation of the static ball and sticks plugin failed.";

    PluginFactory *elementFactory = qobject_cast<PluginFactory *>(new ElementColorFactory);
    if (elementFactory)
      ef[elementFactory->type()].append(elementFactory);
    else
      qDebug() << "Instantiation of the static element color plugin failed.";

    QSettings settings;
    settings.beginGroup("Plugins");

#ifndef Q_WS_MAC
    QFileInfo info(QCoreApplication::applicationDirPath()
                   + "/../CMakeCache.txt");
    if (info.exists()) {// In a build directory
      qDebug() << "In a build directory - loading alternative...";
      loadPluginDir(QCoreApplication::applicationDirPath() + "/../lib",
                    settings);
    }
#else
    // If we are in a Mac build dir things are a little different - if the
    // expected relative path does not exist try the build dir path
    QFileInfo info(QCoreApplication::applicationDirPath()
                   + "/../CMakeCache.txt");
    if (info.exists()) {// In a build directory
        loadPluginDir(QCoreApplication::applicationDirPath()
                      + "/../../../../lib", settings);
    }
#endif

    // Load the plugins
    foreach (const QString& path, d->searchDirs) {
      qDebug() << "Loading plugins:" << path;
      loadPluginDir(path + "/colors", settings);
      loadPluginDir(path + "/engines", settings);
      loadPluginDir(path + "/extensions", settings);
      loadPluginDir(path + "/tools", settings);
      loadPluginDir(path + "/contrib", settings);
    }

#ifdef ENABLE_PYTHON
    // Load the python tools
    QList<QString> scripts = toolScripts();
    foreach(const QString &script, scripts) {
      PluginFactory *factory = qobject_cast<PluginFactory *>(new PythonToolFactory(script));
      if (factory) {
        QFileInfo info(script);
        loadFactory(factory, info, settings);
      }
      else {
        qDebug() << script << "failed to load. ";
      }
    }

    // Load the python engines
    QList<QString> enginescripts = engineScripts();
    foreach(const QString &script, enginescripts) {
      PluginFactory *factory = qobject_cast<PluginFactory *>(new PythonEngineFactory(script));
      if (factory) {
        QFileInfo info(script);
        loadFactory(factory, info, settings);
      }
      else {
        qDebug() << script << "failed to load. ";
      }
    }

    // Load the python extensions
    QList<QString> extensionscripts = extensionScripts();
    foreach(const QString &script, extensionscripts) {
      PluginFactory *factory = qobject_cast<PluginFactory *>(new PythonExtensionFactory(script));
      if (factory) {
        QFileInfo info(script);
        loadFactory(factory, info, settings);
      }
      else {
        qDebug() << script << "failed to load. ";
      }
    }
#endif

    settings.endGroup(); // Plugins
    PluginManagerPrivate::factoriesLoaded = true;
  }

  void PluginManager::loadFactory(PluginFactory *factory, QFileInfo &fileInfo, QSettings &settings)
  {
    settings.beginGroup(QString::number(factory->type()));

    QVector< QList<PluginFactory *> > &ef = PluginManagerPrivate::m_enabledFactories();
    QVector< QList<PluginFactory *> > &df = PluginManagerPrivate::m_disabledFactories();

    // create the PluginItem
    PluginItem *item = new PluginItem(factory->name(), factory->identifier(),
                                      factory->description(),
        factory->type(), fileInfo.fileName(), fileInfo.absoluteFilePath(), factory);
    // add the factory to the correct list
    if(settings.value(factory->identifier(), true).toBool()) {
      ef[factory->type()].append(factory);
      item->setEnabled(true);
    } else {
      df[factory->type()].append(factory);
      item->setEnabled(false);
    }
    // Store the PluginItem
    PluginManagerPrivate::m_items()[factory->type()].append(item);

    settings.endGroup();
  }

  QList<PluginFactory *> PluginManager::factories( Plugin::Type type )
  {
    if (type < PluginManagerPrivate::m_enabledFactories().size()) {
      loadFactories();
      return PluginManagerPrivate::m_enabledFactories()[type];
    }

    return QList<PluginFactory *>();
  }

  void PluginManager::reload()
  {
    // make sure to write the settings before reloading
    QSettings settings;
    writeSettings(settings); // the isEnabled settings for all plugins

    // write the tool settings
    settings.beginGroup("tools");
    foreach(Tool *tool, d->tools) {
      tool->writeSettings(settings);
      tool->deleteLater(); // and delete the tool, this will inform the
                           // ToolGroup which will inform the GLWidget.
    }
    settings.endGroup();

    // set toolsLoaded to false and clear the tools list
    d->toolsLoaded = false;
    d->tools.clear();

    // write the extension settings
    settings.beginGroup("extensions");
    foreach(Extension *extension, d->extensions) {
      extension->writeSettings(settings);
      extension->deleteLater(); // and delete the extension, when the QACtions
                                // are deleted, they are removed from the menu.
    }
    settings.endGroup();

    // set extensionsLoaded to false and clear the extensions list
    d->extensionsLoaded = false;
    d->extensions.clear();

    // Also handle colors
    settings.beginGroup("colors");
    foreach(Color *color, d->colors) {
      color->writeSettings(settings);
      color->deleteLater();
    }
    settings.endGroup();

    // Clear the color list too
    d->colorsLoaded = false;
    d->colors.clear();

    PluginManagerPrivate::factoriesLoaded = false;

    // delete the ProjectItem objects and clear the list
    for(int i=0; i<Plugin::TypeCount; i++) {
      foreach(PluginItem *item, PluginManagerPrivate::m_items()[i])
        delete item;
    }
    PluginManagerPrivate::m_items().clear();

    // delete the enabled PluginFactory objects and clear the list
    for(int i=0; i<Plugin::TypeCount; i++) {
      foreach(PluginFactory *factory, PluginManagerPrivate::m_enabledFactories()[i])
        delete factory;
    }
    PluginManagerPrivate::m_enabledFactories().clear();

    // delete the disabled PluginFactory objects and clear the list
    for(int i=0; i<Plugin::TypeCount; i++) {
      foreach(PluginFactory *factory, PluginManagerPrivate::m_disabledFactories()[i])
        delete factory;
    }
    PluginManagerPrivate::m_disabledFactories().clear();

    // refresh the model in the settings widget
    loadFactories();

    emit reloadPlugins();
  }


  QList<PluginItem *> PluginManager::pluginItems(Plugin::Type type)
  {
    return PluginManagerPrivate::m_items()[type];
  }

  void PluginManager::writeSettings(QSettings &settings)
  {
    // write the plugin item's isEnabled()
    settings.beginGroup("Plugins");
    for(int i=0; i<Plugin::TypeCount; i++) {
      settings.beginGroup(QString::number(i));
      foreach(PluginItem *item, PluginManagerPrivate::m_items()[i]) {
        settings.setValue(item->identifier(), item->isEnabled());
      }
      settings.endGroup();
    }
    settings.endGroup();
  }

  inline void PluginManager::loadPluginDir(const QString &directory,
                                           QSettings &settings)
  {
    QDir dir(directory);
#ifdef Q_WS_X11
    QStringList dirFilters;
    dirFilters << "*.so";
    dir.setNameFilters(dirFilters);
    dir.setFilter(QDir::Files | QDir::Readable);
#endif
    qDebug() << "Searching for plugins in" << directory;
    foreach (const QString& fileName, dir.entryList(QDir::Files)) {
#ifdef Q_WS_X11
      if ((fileName.indexOf("libavogadro.so") != -1) || (fileName.indexOf("Avogadro.so") != -1))
        continue;
#endif
      // load the factory
      QPluginLoader loader(dir.absoluteFilePath(fileName));
      QObject *instance = loader.instance();
      PluginFactory *factory = qobject_cast<PluginFactory *>(instance);

      if (factory) {
        QFileInfo info(fileName);
        loadFactory(factory, info, settings);
      } else {
        qDebug() << fileName << "failed to load. " << loader.errorString();
      }
    }
  }

}

#include "pluginmanager.moc"
