/**********************************************************************
  CustomColor - Class for using arbitrary colors

  Copyright (C) 2009 Geoffrey R. Hutchison

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

#ifndef CUSTOMCOLOR_H
#define CUSTOMCOLOR_H

#include <avogadro/global.h>
#include <avogadro/plugin.h>
#include <avogadro/color.h>

namespace Avogadro {

  /**
   * @class CustomColor 
   * @brief Set custom colors for atoms
   * @author Geoff Hutchison
   *
   */
  class CustomColor: public Color
  {
    Q_OBJECT
    AVOGADRO_COLOR("CustomColor", tr("Custom Color"),
                   tr("Set custom colors for objects"))

  public:
    CustomColor();
    virtual ~CustomColor();

    // In this case, the settings are everything!
    // We set our color based on the settings
    virtual QWidget* settingsWidget();
    virtual void writeSettings(QSettings &settings) const;
    virtual void readSettings(QSettings &settings);

  private Q_SLOTS:
      void settingsWidgetDestroyed();
      void colorChanged(QColor);

  private:
      QWidget *m_settingsWidget;
  };

  class CustomColorFactory : public QObject, public PluginFactory
  {
    Q_OBJECT
    Q_INTERFACES(Avogadro::PluginFactory)
    AVOGADRO_COLOR_FACTORY(CustomColor)
  };

}

#endif
