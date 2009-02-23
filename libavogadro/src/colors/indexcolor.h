/**********************************************************************
  IndexColor - Color atoms by numbering in the file

  Copyright (C) 2008 Geoffrey R. Hutchison

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

#ifndef INDEXCOLOR_H
#define INDEXCOLOR_H

#include <avogadro/global.h>
#include <avogadro/plugin.h>
#include <avogadro/color.h>

namespace Avogadro {

  /**
   * @class IndexColor indexcolor.h <avogadro/indexcolor.h>
   * @brief Color by atomic index (i.e., order in the file)
   * @author Geoff Hutchison
   *
   * Map atom colors based on atom numbering (with rainbow colors)
   */
  class IndexColor: public Color
  {
    Q_OBJECT
    AVOGADRO_COLOR("IndexColor", tr("Color by Index"),
                   tr("Color by Index (red, orange, yellow, green, blue, violet)."))

  public:
    IndexColor();
    virtual ~IndexColor();

    /**
     * Set the color based on the supplied Primitive
     * If NULL is passed, do nothing */
    void set(const Primitive *);
  };

  class IndexColorFactory : public QObject, public PluginFactory
  {
    Q_OBJECT
    Q_INTERFACES(Avogadro::PluginFactory)
    AVOGADRO_COLOR_FACTORY(IndexColor)
  };


}

#endif
