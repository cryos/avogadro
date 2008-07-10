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

#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#include <avogadro/global.h>
#include <avogadro/engine.h>
#include <avogadro/tool.h>
#include <avogadro/extension.h>

#include <QSettings>

namespace Avogadro {

  
  class PluginItemPrivate;
  class PluginItem 
  {
    public:
      enum Type
      {
        EngineType = 0,
        ToolType,
        ExtensionType
      };

    public:
      PluginItem();
      PluginItem(int type, const QString &fileName, const QString &filePath);
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

      /**
       * Set the plugin type (engine = 0, tool = 1, extension = 2)
       */
      void setType( int type );
      /**
       * Set the plugin name (Draw, Stick, ...)
       */
      void setName( const QString &name );
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
     * Get all the PluginItems for a given type
     */
    QList<PluginItem *> plugins( int type );
 
    /**
     * Find all the engine plugins by looking through the search paths:
     *    /usr/lib/avogadro/engines
     *    /usr/local/lib/avogadro/engines
     *
     * You can set the AVOGADRO_ENGINES to designate a path
     * at runtime.
     */
    void loadEngineFactories();
    /**
     * Get the loaded engine factories
     */
    const QList<EngineFactory *>& engineFactories() const;
    /**
     * Get the QHash to translate an engine className to 
     * a EngineFactory* pointer.
     */
    const QHash<QString, EngineFactory *>& engineClassFactory() const;
    
    /**
     * Find all the tool plugins by looking through the search paths:
     *    /usr/lib/avogadro/tools
     *    /usr/local/lib/avogadro/tools
     *
     * You can set the AVOGADRO_TOOLS to designate a path
     * at runtime.
     */
    void findTools();
    /**
     * Load the tools 
     */
    void loadTools();
    /**
     * Get the loaded tools
     */
    const QList<Tool *>& tools() const;
    
    /**
     * Find all the extension plugins by looking through the search paths:
     *    /usr/lib/avogadro/extensions
     *    /usr/local/lib/avogadro/extensions
     *
     * You can set the AVOGADRO_EXTENSIONS to designate a path
     * at runtime.
     */
    void findExtensions();
    /**
     * Load the tools 
     */
    void loadExtensions();
    /**
     * Get the loaded extensions
     */
    const QList<Extension *>& extensions() const;

    /**
     * Write the settings of the PluginManager in order to save them to disk.
     */
    void writeSettings(QSettings &settings) const;
  
  public Q_SLOTS:
    void showDialog();
 
  private:
    PluginManagerPrivate * const d;
  };

  A_DECL_EXPORT extern PluginManager pluginManager;

}

#endif
