/**********************************************************************
  ColorPlugin - Avogadro Color Interface

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

#ifndef COLORPLUGIN_H
#define COLORPLUGIN_H

#include <avogadro/color.h>
#include <avogadro/global.h>
#include "plugin.h"

#include <QSettings>
#include <QtPlugin>

#define AVOGADRO_COLOR_FACTORY(c,n,d) \
  public: \
    Plugin *createInstance(QObject *parent = 0) { return new c(parent); } \
    int type() const { return Plugin::ColorType; }; \
    QString name() const { return n; }; \
    QString description() const { return d; }; 

namespace Avogadro {

  /**
   * @class ColorPlugin colorplugin.h <avogadro/colorplugin.h>
   * @brief Interface for color plugins
   * @author Tim Vandermeersch
   *
   * This is a template class for color plugins.
   */
  class A_EXPORT ColorPlugin : public Plugin
  {
    Q_OBJECT

    public:
      /**
       * Constructor
       */
      ColorPlugin(QObject *parent = 0);

      /**
       * Destructor
       */
      virtual ~ColorPlugin();

      /** 
       * Plugin Type 
       */
      int type() const;
 
      /** 
       * Plugin Type Name (Colors)
       */
      QString typeName() const;
      
      /**
       * @return the widget for controlling settings for this color map
       * or NULL if none exists. */
      virtual QWidget *settingsWidget() { return NULL; }
 
      /** 
       * @return the Color for this plugin
       */
      virtual Color* color() const = 0;
  };

} // end namespace Avogadro

#endif
