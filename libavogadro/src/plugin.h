/**********************************************************************
  Plugin - Avogadro Plugin Interface Base Class

  Copyright (C) 2008 Tim Vandermeersch
  Copyright (C) 2008 Donald Ephraim Curtis

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

#ifndef PLUGIN_H
#define PLUGIN_H

#include <avogadro/global.h>

#include <QObject>
#include <QSettings> // do not remove!!

namespace Avogadro {

  /**
   * @class Plugin plugin.h <avogadro/plugin.h>
   * @brief Interface for plugins
   * @author Tim Vandermeersch
   *
   * This is a template class for plugins like tools, engines,
   * extensions, colors, ...
   */

  /*class A_EXPORT PluginMeta*/
  /*{*/
    /*public:*/
      /*QString className();*/
      /*QString type();*/
  /*}*/

  //class PluginPrivate;
  class A_EXPORT Plugin : public QObject
  {
    Q_OBJECT

    public:
      enum Type
      {
        EngineType = 0,
        ToolType,
        ExtensionType,
        ColorType,
        OtherType,
        TypeCount // this needs to be last always
      };

    public:
      /**
       * Constructor
       */
      Plugin(QObject *parent = NULL);

      /**
       * Destructor
       */
      virtual ~Plugin();

      /**
       * @return the type of the plugin.
       */
      virtual Plugin::Type type() const = 0;

      /**
       * @return the unique, untranslated identifier for the plugin.
       */
      virtual QString identifier() const = 0;

      /**
       * @return the translated name of the plugin.
       */
      virtual QString name() const = 0;

      /**
       * @return a description of the plugin.
       */
      virtual QString description() const;

      /**
       * @return The license applied to the plugin.
       * @note This defaults to GPL2+, and must be a license compatible with a
       * GPL2 only library, as all plugins link to Avogadro and OpenBabel.\
       */
      virtual QString license() const;

      /**
       * @return a QWidget containing the engine settings or 0
       * if no settings widget is available.
       */
      virtual QWidget *settingsWidget();

      /**
       * Write the engine settings so that they can be saved between sessions.
       */
      virtual void writeSettings(QSettings &settings) const;

      /**
       * Read in the settings that have been saved for the engine instance.
       */
      virtual void readSettings(QSettings &settings);

    protected:
      //PluginPrivate *const d;
  };

  /**
   * @class PluginFactory plugin.h <avogadro/plugin.h>
   * @brief Generates new instances of the Plugin class for which it is defined.
   *
   * Generates new instances of the Plugin class for which it is defined.
   */
  class A_EXPORT PluginFactory
  {
    public:
      /**
       * Destructor.
       */
      virtual ~PluginFactory() {}

      /**
       * @return pointer to a new instance of an Engine subclass object.
       */
      virtual Plugin *createInstance(QObject *parent=0) = 0;

      /**
       * @return the type of the plugin.
       */
      virtual Plugin::Type type() const = 0;

      /**
       * @return the untranslated identifier of the plugin
       */
      virtual QString identifier() const = 0;

      /**
       * @return the translated name of the plugin.
       */
      virtual QString name() const = 0;

      /**
       * @return a description of the plugin.
       */
      virtual QString description() const = 0;
  };

} // end namespace Avogadro

Q_DECLARE_METATYPE(Avogadro::Plugin*)
Q_DECLARE_INTERFACE(Avogadro::PluginFactory, "net.sourceforge.avogadro.pluginfactory/1.2")

#endif
