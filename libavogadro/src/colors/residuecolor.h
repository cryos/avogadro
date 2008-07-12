/**********************************************************************
  ResidueColor - Class for coloring based on residues (if available)

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

#ifndef RESIDUECOLOR_H
#define RESIDUECOLOR_H

#include <avogadro/global.h>
#include <avogadro/plugin.h>
#include <avogadro/color.h>
#include <avogadro/colorplugin.h>

namespace Avogadro {

  /**
   * @class ResidueColor residuecolor.h <avogadro/residuecolor.h>
   * @brief Atom coloring based on residue for biomolecules
   * @author Geoff Hutchison
   *
   * This class maps colors based on residues
   * http://jmol.sourceforge.net/jscolors/
   */
  class ResidueColor: public Color
  {
  public:
    ResidueColor();
    virtual ~ResidueColor();

    /**
     * Set the color based on the supplied Primitive
     * If NULL is passed, do nothing */
    virtual void set(const Primitive *);

    virtual QString type() const { return "Color by Residue"; }
  };

  class ResidueColorPlugin : public ColorPlugin
  {
    public:
      ResidueColorPlugin(QObject *parent = 0);
      ~ResidueColorPlugin();

      QString name() const { return(tr("Color by Residue")); }
      Color* color() const;

    private:
      Color *m_color;
  };

  class ResidueColorFactory : public QObject, public PluginFactory
  {
    Q_OBJECT
    Q_INTERFACES(Avogadro::PluginFactory)
    AVOGADRO_COLOR_FACTORY(ResidueColorPlugin, tr("Color by Residue"), 
        tr("Color by Residue."))
  };


}

#endif
