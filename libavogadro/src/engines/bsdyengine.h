/**********************************************************************
  BSDYEngine - Dynamic detail engine for "balls and sticks" display

  Copyright (C) 2007 Donald Ephraim Curtis

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

#ifndef BSDYENGINE_H
#define BSDYENGINE_H

#include <avogadro/global.h>
#include <avogadro/engine.h>

#include "ui_bsdysettingswidget.h"

namespace Avogadro {

  //! Ball and Stick Engine class.
  class Atom;
  class BSDYSettingsWidget;
  class BSDYEngine : public Engine
  {
    Q_OBJECT
    AVOGADRO_ENGINE("Ball and Stick", tr("Ball and Stick"),
                      tr("Renders primitives using Balls (atoms) and Sticks (bonds)"))

    public:
      //! Constructor
      BSDYEngine(QObject *parent=0);

      Engine *clone() const;

      //! Deconstructor
      ~BSDYEngine();

      bool renderOpaque(PainterDevice *pd);
      bool renderTransparent(PainterDevice *pd);
      bool renderQuick(PainterDevice *pd);
      bool renderPick(PainterDevice *pd);

      double transparencyDepth() const;

      Engine::Layers layers() const;

      double radius(const PainterDevice *pd, const Primitive *p = 0) const;

      /**
       * @return a pointer to the BSDYEngine settings widget
       */
      QWidget *settingsWidget();
      /**
       * Write the engine settings so that they can be saved between sessions.
       */
      void writeSettings(QSettings &settings) const;

      /**
       * Read in the settings that have been saved for the engine instance.
       */
      void readSettings(QSettings &settings);


    private:
      double radius(const Atom *atom) const;

      BSDYSettingsWidget *m_settingsWidget;

      double m_atomRadiusPercentage;
      double m_bondRadius;
      int m_showMulti;

      double m_alpha; // transparency of the balls & sticks

   private Q_SLOTS:
      void settingsWidgetDestroyed();

      /**
       * @param percent percentage of the VdwRad
       */
      void setAtomRadiusPercentage(int percent);

      /**
       * @param value radius of the bonds * 10
       */
      void setBondRadius(int value);

      /**
       * @param value determines where multiple bonds are shown
       */
      void setShowMulti(int value);

      /**
       * @param value opacity of the balls & sticks / 20
       */
      void setOpacity(int value);

  };

  class BSDYSettingsWidget : public QWidget, public Ui::BSDYSettingsWidget
  {
    public:
      BSDYSettingsWidget(QWidget *parent=0) : QWidget(parent) {
        setupUi(this);
      }
  };

  class BSDYEngineFactory : public QObject, public PluginFactory
  {
    Q_OBJECT
    Q_INTERFACES(Avogadro::PluginFactory)
    AVOGADRO_ENGINE_FACTORY(BSDYEngine);
  };

} // end namespace Avogadro

#endif
