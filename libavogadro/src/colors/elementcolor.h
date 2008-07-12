/**********************************************************************
  ElementColor - Default class for coloring atoms based on element

  Copyright (C) 2006 Benoit Jacob
  Copyright (C) 2007 Geoffrey R. Hutchison

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

#ifndef ELEMENTCOLOR_H
#define ELEMENTCOLOR_H

#include <avogadro/global.h>
#include <avogadro/plugin.h>
#include <avogadro/color.h>
#include <avogadro/colorplugin.h>

namespace Avogadro {

  /**
   * @class ElementColor elementcolor.h <avogadro/elementcolor.h>
   * @brief Default atom color scheme based on periodic table
   * @author Geoff Hutchison
   *
   * Map atom colors based on elements: Carbon = Grey, Oxygen = Red, etc.
   */
  class ElementColor: public Color
  {
  public:
    ElementColor();
    virtual ~ElementColor();

    /**
     * Set the color based on the supplied Primitive
     * If NULL is passed, do nothing */
    void set(const Primitive *);
    
    QString type() const { return "Color by Element"; }
  };

  class ElementColorPlugin : public ColorPlugin
  {
    public:
      ElementColorPlugin(QObject *parent = 0);
      ~ElementColorPlugin();

      QString name() const { return(tr("Color by Element")); }
      Color* color() const;

    private:
      Color *m_color;
  };

  class ElementColorFactory : public QObject, public PluginFactory
  {
    Q_OBJECT
    Q_INTERFACES(Avogadro::PluginFactory)
    AVOGADRO_COLOR_FACTORY(ElementColorPlugin, tr("Color by Element"), 
        tr("Color by Element (carbon = grey, oxygen = red, ...)."))
  };


}

#endif
