/**********************************************************************
  ResidueColor - Class for coloring based on residues (if available)

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

#ifndef RESIDUECOLOR_H
#define RESIDUECOLOR_H

#include <avogadro/global.h>
#include <avogadro/plugin.h>
#include <avogadro/color.h>

#include "ui_residuecolorsettings.h"

namespace Avogadro {

  class ResidueColorSettingsWidget;

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
    Q_OBJECT
    AVOGADRO_COLOR("ResidueColor", tr("Color by Residue"),
                     tr("Color by Residue (amino acid type, hydrophobicity, ..."))

  public:
    ResidueColor();
    virtual ~ResidueColor();

    /**
     * Set the color based on the supplied Primitive
     * If NULL is passed, do nothing */
    virtual void set(const Primitive *);

    virtual QWidget* settingsWidget();

  private Q_SLOTS:
      void settingsWidgetDestroyed();
      void setColorScheme(int colorScheme);

  private:
    ResidueColorSettingsWidget *m_settingsWidget;
    int      m_colorScheme;
  };

  class ResidueColorSettingsWidget : 
    public QWidget, 
    public Ui::ResidueColorSettings
  {
    public:
      ResidueColorSettingsWidget(QWidget *parent=0) : QWidget(parent) {
        setupUi(this);
      }
  };

  class ResidueColorFactory : public QObject, public PluginFactory
  {
    Q_OBJECT
    Q_INTERFACES(Avogadro::PluginFactory)
    AVOGADRO_COLOR_FACTORY(ResidueColor)
  };


}

#endif
