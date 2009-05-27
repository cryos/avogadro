/**********************************************************************
  DistanceColor - Color atoms by distance from atom 1

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

#ifndef DISTANCECOLOR_H
#define DISTANCECOLOR_H

#include <avogadro/global.h>
#include <avogadro/plugin.h>
#include <avogadro/color.h>

namespace Avogadro {

  /**
   * @class DistanceColor
   * @brief Color by distance from the first atom
   * @author Geoff Hutchison
   *
   * Map atom colors based on atom distance from the first atom
   */
  class DistanceColor: public Color
  {
    Q_OBJECT
    AVOGADRO_COLOR("DistanceColor", tr("Color by Distance"),
                   tr("Color by distance from the first atom."))

  public:
    DistanceColor();
    virtual ~DistanceColor();

    /**
     * Set the color based on the supplied Primitive
     * If NULL is passed, do nothing */
    void set(const Primitive *);
  };

  class DistanceColorFactory : public QObject, public PluginFactory
  {
    Q_OBJECT
    Q_INTERFACES(Avogadro::PluginFactory)
    AVOGADRO_COLOR_FACTORY(DistanceColor)
  };


}

#endif
