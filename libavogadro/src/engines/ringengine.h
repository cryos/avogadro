/**********************************************************************
  RingEngine - Engine for "ring" display

  Copyright (C) 2007 by Marcus D. Hanwell

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

#ifndef RINGENGINE_H
#define RINGENGINE_H

#include <avogadro/global.h>
#include <avogadro/engine.h>

#include <QList>

#include "ui_ringsettingswidget.h"

namespace Avogadro {

  //! Ring Engine class.
  class RingSettingsWidget;
  class RingEngine : public Engine
  {
    Q_OBJECT
      AVOGADRO_ENGINE("Ring", tr("Ring"))

    public:
      //! Constructor
      RingEngine(QObject *parent=0);

      Engine *clone() const;

      //! Deconstructor
      ~RingEngine();

      //! \name Render Methods
      //@{
      bool renderOpaque(PainterDevice *pd);
      bool renderTransparent(PainterDevice *pd);
      //@}

      double transparencyDepth() const;
      Layers layers() const;
      PrimitiveTypes primitiveTypes() const;
      ColorTypes colorTypes() const;

      double radius(const PainterDevice *pd, const Primitive *p = 0) const;

      QWidget* settingsWidget();
      /**
       * Write the engine settings so that they can be saved between sessions.
       */
      void writeSettings(QSettings &settings) const;

      /**
       * Read in the settings that have been saved for the engine instance.
       */
      void readSettings(QSettings &settings);


    private:
      RingSettingsWidget *m_settingsWidget;
      double m_alpha; // transparency of the VdW spheres

      bool renderRing(const QList<unsigned long> &ring, PainterDevice *pd); // Render the given ring

    private Q_SLOTS:
      void settingsWidgetDestroyed();

      /**
       * @param value opacity of the rings
      */
      void setOpacity(int value);

  };

  class RingSettingsWidget : public QWidget, public Ui::RingSettingsWidget
  {
    public:
      RingSettingsWidget(QWidget *parent=0) : QWidget(parent) {
        setupUi(this);
      }
  };


  //! Generates instances of our RingEngine class
  class RingEngineFactory : public QObject, public PluginFactory
  {
    Q_OBJECT
    Q_INTERFACES(Avogadro::PluginFactory)
      AVOGADRO_ENGINE_FACTORY(RingEngine, "Ring", tr("Ring"), tr("Render ring planes."))
  };

} // end namespace Avogadro

#endif
