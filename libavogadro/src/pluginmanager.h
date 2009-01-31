/**********************************************************************
  PluginManager - Class to handle dynamic loading/unloading of plugins

  Copyright (C) 2008 Donald Ephraim Curtis
  Copyright (C) 2008,2009 Tim Vandermeersch
  Copyright (C) 2008 Marcus D. Hanwell

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

#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#include <avogadro/global.h>
#include <avogadro/plugin.h>

class QDir;
class QSettings;

namespace Avogadro {

  class PluginFactory;
  class Tool;
  class Engine;
  class Extension;
  class Color;
  class PluginItemPrivate;

  class PluginItem
  {
  public:
    PluginItem();
    PluginItem(int type, const QString &fileName, const QString &filePath, PluginFactory *factory);
    PluginItem( const QString &name, const QString &description, Plugin::Type type, const QString &fileName, const QString &filePath, PluginFactory *factory = 0, bool enabled = true);
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

    static PluginManager* instance();

    /**
     * Find all plugins by looking through the search paths:
     *    /usr/(local/)lib/avogadro/engines
     *    /usr/(local/)lib/avogadro/tools
     *    /usr/(local/)lib/avogadro/extensions
     *    /usr/(local/)lib/avogadro/colors
     *
     * You can set AVOGADRO_PLUGINS to designate a path at runtime to search.
     *
     * WIN32: look in the applications working dir ( ./engines, ...)
     */
    static void loadFactories();

    /**
     * Get all the PluginFactory obacjects for a given type.
     */
    static QList<PluginFactory *> factories( Plugin::Type type );

    /**
     * Get the PluginFactory of type @p type with plugin name @p name.
     */
    static PluginFactory *factory(const QString &name, Plugin::Type type);

    /**
     * Get a new instance of the extension with name @name
     * @return 0 if there is no extension plugin with name @nam
     */
    Extension *extension(const QString &name, QObject *parent = 0);

    /**
     * Get a new instance of the tool with name @name
     * @return 0 if there is no tool plugin with name @nam
     */
    Tool *tool(const QString &name, QObject *parent = 0);

    /**
     * Get a new instance of the color with name @name
     * @return 0 if there is no color plugin with name @nam
     */
    Color *color(const QString &name, QObject *parent = 0);

    /**
     * Get a new instance of the color with name @name
     * @return 0 if there is no engine plugin with name @nam
     */
    Engine *engine(const QString &name, QObject *parent = 0);

    /**
     * Get a list with the plugin names of type @type
     */
    QList<QString> names(Plugin::Type type);

    /**
     * Get a list with the plugin descriptions of type @type
     */
    QList<QString> descriptions(Plugin::Type type);

    /**
     * Get a list of all extension (new instances)
     */
    QList<Extension *> extensions(QObject *parent=0) const;

    /**
     * Get a list of all tools (new instances)
     */
    QList<Tool *> tools(QObject *parent=0) const;

    /**
     * Get a list of all colors (new instances)
     */
    QList<Color *> colors(QObject *parent=0) const;

    /**
     * Write the settings of the PluginManager in order to save them to disk.
     */
    static void writeSettings(QSettings &settings);
    /*static void readSettings(QSettings &settings);*/

    /**
     * Get a list of all toolScripts.
     */
    static QList<QString> toolScripts();

    /**
     * Use by the plugin manager dialog
     */
    static QList<PluginItem *> pluginItems(Plugin::Type);

  public Q_SLOTS:
    void showDialog();
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

  };

}

#endif
