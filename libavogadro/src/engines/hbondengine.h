/**********************************************************************
  HBondEngine - Hydrogen Bond Engine

  Copyright (C) 2007 by Marcus D. Hanwell
  Copyright (C) 2007 by Tim Vandermeersch

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

#ifndef __HBONDENGINE_H
#define __HBONDENGINE_H

#include <avogadro/global.h>
#include <avogadro/engine.h>


#include "ui_hbondsettingswidget.h"

namespace Avogadro {

  //! HBond Engine class.
  class HBondSettingsWidget;
  class HBondEngine : public Engine
  {
    Q_OBJECT
    AVOGADRO_ENGINE(tr("HBond"))

    public:
      //! Constructor
      HBondEngine(QObject *parent=0);
      //! Deconstructor
      ~HBondEngine();

      //! \name Render Methods
      //@{
      bool renderOpaque(PainterDevice *pd);
      //@}

      double radius(const PainterDevice *pd, const Primitive *p = 0) const;

      QWidget* settingsWidget();

    private:
      HBondSettingsWidget *m_settingsWidget;
      double m_width;
      double m_radius;
      double m_angle;
    
    private Q_SLOTS:
      void settingsWidgetDestroyed();
    
     /**
       * @param value width of the hydrogen bonds
      */
      void setWidth(int value);
      /**
       * @param value cut-off radius for HBond detection
      */
      void setRadius(double value);
      /**
       * @param value cut-off angle for HBond detection
      */
      void setAngle(double value);
  };

  class HBondSettingsWidget : public QWidget, public Ui::HBondSettingsWidget
  {
    public:
      HBondSettingsWidget(QWidget *parent=0) : QWidget(parent) {
        setupUi(this);
      }
  };

  //! Generates instances of our HBondEngine class
  class HBondEngineFactory : public QObject, public EngineFactory
  {
    Q_OBJECT
    Q_INTERFACES(Avogadro::EngineFactory)
    AVOGADRO_ENGINE_FACTORY(HBondEngine)
  };

} // end namespace Avogadro

#endif
