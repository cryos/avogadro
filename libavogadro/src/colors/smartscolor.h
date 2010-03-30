/**********************************************************************
  Smartscolor -  Map atom colors based on atom partial charge

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

#ifndef SMARTSCOLOR_H
#define SMARTSCOLOR_H

#include <avogadro/global.h>
#include <avogadro/plugin.h>
#include <avogadro/color.h>

#include <QString>

// forward declaration
namespace OpenBabel {
  class OBSmartsPattern;
}

namespace Avogadro {

  /**
   * @class Smartscolor 
   * @brief Color by highlighting a SMARTS pattern match
   * @author Geoff Hutchison
   */
  class SmartsColor: public Color
  {
    Q_OBJECT
    AVOGADRO_COLOR("SmartsColor", tr("Color by SMARTS Pattern"),
                   tr("Highlight particular features which match a SMARTS pattern."))

  public:
    SmartsColor();
    virtual ~SmartsColor();

    /**
     * Set the color based on the supplied Primitive
     * If NULL is passed, do nothing */
    void setFromPrimitive(const Primitive *);

    virtual QWidget* settingsWidget();
    virtual void writeSettings(QSettings &settings) const;
    virtual void readSettings(QSettings &settings);

  private Q_SLOTS:
      void settingsWidgetDestroyed();
      void smartsChanged(QString);
      void colorChanged(QColor);

  private:
    OpenBabel::OBSmartsPattern *_pattern;
    QString                     _smartsString;
    QColor                      _highlightColor;
    QWidget                    *_settingsWidget;
  };

  class SmartsColorFactory : public QObject, public PluginFactory
  {
    Q_OBJECT
    Q_INTERFACES(Avogadro::PluginFactory)
    AVOGADRO_COLOR_FACTORY(SmartsColor)
  };


}

#endif
