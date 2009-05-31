/**********************************************************************
  ElementColor - Default class for coloring atoms based on element

  Copyright (C) 2006 Benoit Jacob
  Copyright (C) 2007 Geoffrey R. Hutchison

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

#ifndef ELEMENTCOLOR_H
#define ELEMENTCOLOR_H

#include <avogadro/global.h>
#include <avogadro/plugin.h>
#include <avogadro/color.h>

namespace Avogadro {

  /**
   * @class ElementColor
   * @brief Default atom color scheme based on periodic table
   * @author Geoff Hutchison
   *
   * Map atom colors based on elements: Carbon = Grey, Oxygen = Red, etc.
   */
  class ElementColor: public Color
  {
    Q_OBJECT
    AVOGADRO_COLOR("ElementColor", tr("Color by Element"),
                   tr("Color by Element (carbon = grey, oxygen = red, ...)."))

  public:
    ElementColor();
    virtual ~ElementColor();

    /**
     * Set the color based on the supplied Primitive
     * If NULL is passed, do nothing */
    void setFromPrimitive(const Primitive *);
  };

  class ElementColorFactory : public QObject, public PluginFactory
  {
    Q_OBJECT
    Q_INTERFACES(Avogadro::PluginFactory)
    AVOGADRO_COLOR_FACTORY(ElementColor)
  };


}

#endif
