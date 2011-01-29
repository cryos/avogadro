/**********************************************************************
  PluginManager - Class to handle dynamic loading/unloading of plugins

  Copyright (C) 2008 Donald Ephraim Curtis
  Copyright (C) 2008,2009 Tim Vandermeersch
  Copyright (C) 2008 Marcus D. Hanwell

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

#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#include <avogadro/global.h>
#include <avogadro/plugin.h>

#include <QStringList>

class QDir;
class QSettings;
class QFileInfo;

namespace Avogadro {

  class PluginFactory;
  class Tool;
  class Engine;
  class Extension;
  class Color;
  class PluginItemPrivate;

  class A_EXPORT PluginItem
  {
  public:
    PluginItem();
    //    PluginItem(int type, const QString &fileName, 
    //               const QString &filePath, PluginFactory *factory);
    PluginItem(const QString &name, const QString &identifier,
               const QString &description,
               Plugin::Type type, const QString &fileName, 
               const QString &filePath, PluginFactory *factory = 0, 
               bool enabled = true);
    ~PluginItem();

    /**
     * The plugin type (engine = 0, tool = 1, extension = 2)
     */
    int type() const;

    /**
     * The plugin name (Draw, Stick, ...)
     */
    QString name() const;

    /**
     * The plugin description (Draw, Stick, ...)
     */
    QString description() const;

    /**
     * The plugin unique, untranslated identifier
     */
    QString identifier() const;

    /**
     * The plugin filename (libdrawtool.so, libaligntool.dll, ...)
     */
    QString fileName() const;

    /**
     * The absolute file path
     */
    QString absoluteFilePath() const;

    /**
     * Should the plugin be loaded
     */
    bool isEnabled() const;

    PluginFactory *factory() const;

    /**
     * Set the plugin type (engine = 0, tool = 1, extension = 2)
     */
    void setType( Plugin::Type type );

    /**
     * Set the plugin name (Draw, Stick, ...)
     */
    void setName( const QString &name );

    /**
     * Set the plugin unique, untranslated identifier
     */
    void setIdentifier( const QString &id );

    /**
     * Set the plugin description
     */
    void setDescription( const QString &description );

    /**
     * The plugin filename (libdrawtool.so, libaligntool.dll, ...)
     */
    void setFileName( const QString &fileName );

    /**
     * The absolute file path
     */
    void setAbsoluteFilePath( const QString &filePath );

    /**
     * Should the plugin be loaded
     */
    void setEnabled( bool enable );

    void setFactory( PluginFactory *factory );

  private:
    PluginItemPrivate * const d;
  };

  class PluginManagerPrivate;
  class A_EXPORT PluginManager: public QObject
  {
  Q_OBJECT

  public:
    PluginManager(QObject *parent = 0);
    ~PluginManager();

    /**
     * Get the singleton instance of the PluginManager. Always use this function
     * when using the PluginManager class. This pointer never has to be deleted.
     */ 
    static PluginManager* instance();

    /**
     * Set the search path for plugins as list of directories.
     */
    void setPluginPath(const QStringList& path);

    /**
     * @overload
     * Set the search path for plugins.
     */
    void setPluginPath(const QString& path);

    /**
     * Add list of directories to the search path.
     */
    void addToPluginPath(const QStringList& path);

    /**
     * @overload
     * Add the supplied path to the search path.
     */
    void addToPluginPath(const QString& path);

    /**
     * Get the search path for plugins.
     */
    QStringList pluginPath() const;

    /**
     * Load statically linked plugins
     */
    void loadStaticFactories();

    /**
     * Find all plugins by looking through the plugin search path. If it was
     * not set, the next directories are used
     *    <bin_location>/../<lib_dir>/avogadro/engines
     *    <bin_location>/../<lib_dir>/avogadro/tools
     *    <bin_location>/../<lib_dir>/avogadro/extensions
     *    <bin_location>/../<lib_dir>/avogadro/colors
     *    <bin_location>/../<lib_dir>/avogadro/contrib (external plugins)
     *
     * You can set AVOGADRO_PLUGINS to designate a path at runtime to search.
     *
     * @param dir Path of directory to search for plugins instead of the default
     * paths
     * @sa setPluginPath, pluginPath
     */
    void loadFactories(const QString& dir = "");

    /**
     * Load plugins with file names listed in @a plugins from directory @a dir
     * and enable/disable them using @a settings
     */
    static void loadPluginList(const QDir &dir, const QStringList &plugins,
                               QSettings &settings);

    /**
     * Get all the PluginFactory obacjects for a given type.
     */
    QList<PluginFactory *> factories( Plugin::Type type );

    /**
     * Get the PluginFactory of type @p type with plugin identifier @p id.
     */
    PluginFactory *factory(const QString &id, Plugin::Type type);

    /**
     * Get a new instance of the extension with identifier @p id.
     * @return 0 if there is no extension plugin with identifier @p id
     */
    Extension *extension(const QString &id, QObject *parent = 0);

    /**
     * Get a new instance of the tool with identifier @p id.
     * @return 0 if there is no tool plugin with identifier @p id
     */
    Tool *tool(const QString &id, QObject *parent = 0);

    /**
     * Get a new instance of the color with identifier @p id.
     * @return 0 if there is no color plugin with identifier @p id
     */
    Color *color(const QString &id, QObject *parent = 0);

    /**
     * Get a new instance of the color with identifier @p id.
     * @return 0 if there is no engine plugin with identifier @p id
     */
    Engine *engine(const QString &id, QObject *parent = 0);

    /**
     * Get a list with the plugin names of type @p type.
     */
    QList<QString> names(Plugin::Type type);

    /**
     * Get a list with the plugin identifiers of type @p type.
     */
    QList<QString> identifiers(Plugin::Type type);

    /**
     * Get a list with the plugin descriptions of type @p type.
     */
    QList<QString> descriptions(Plugin::Type type);

    /**
     * Get a list of all extension (new instances).
     */
    QList<Extension *> extensions(QObject *parent=0);

    /**
     * Get a list of all tools (new instances).
     */
    QList<Tool *> tools(QObject *parent=0);

    /**
     * Get a list of all colors (new instances).
     */
    QList<Color *> colors(QObject *parent=0);

    /**
     * Write the settings of the PluginManager in order to save them to disk.
     */
    void writeSettings(QSettings &settings);
    /*static void readSettings(QSettings &settings);*/

    /**
     * Get a list of all python tool script filenames.
     */
    QList<QString> toolScripts();

    /**
     * Get a list of all python engine script filenames.
     */
    QList<QString> engineScripts();

    /**
     * Get a list of all python extension script filenames.
     */
    QList<QString> extensionScripts();

    /**
     * Use by the plugin manager settings dialog
     */
    QList<PluginItem *> pluginItems(Plugin::Type);

  public Q_SLOTS:
    void reload();

  Q_SIGNALS:
    void reloadPlugins();

  private:
    PluginManagerPrivate * const d;

    /**
     * Private function that traverses a directory loading plugins if they are
     * valid.
     */
    static void loadPluginDir(const QString &directory, QSettings &settings);
    static void loadFactory(PluginFactory *factory, QFileInfo &fileInfo,
                            QSettings &settings);
    static QList<QString> scripts(const QString &type);
    static void initializeSearchDirs(QStringList &searchDirs);
  };

}

#endif
