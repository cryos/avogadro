/**********************************************************************
  RibbonEngine - Engine for "ribbon" display

  Copyright (C) 2007 by Marcus D. Hanwell

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

#ifndef RIBBONENGINE_H
#define RIBBONENGINE_H

#include <avogadro/global.h>
#include <avogadro/engine.h>


#include "ui_ribbonsettingswidget.h"

namespace Avogadro {

  //! Ribbon Engine class.
  class RibbonSettingsWidget;
  class RibbonEngine : public Engine
  {
    Q_OBJECT
    AVOGADRO_ENGINE(tr("Ribbon"))

    public:
      //! Constructor
      RibbonEngine(QObject *parent=0);

      Engine *clone() const;

      //! Deconstructor
      ~RibbonEngine();

      //! \name Render Methods
      //@{
      bool renderOpaque(PainterDevice *pd);
      bool renderTransparent(PainterDevice *pd);
      bool renderQuick(PainterDevice *pd);
      //@}

      double transparencyDepth() const;
      EngineFlags flags() const;

      double radius(const PainterDevice *pd, const Primitive *p = 0) const;

      QWidget* settingsWidget();

      void setPrimitives(const PrimitiveList &primitives);

      /**
       * Write the engine settings so that they can be saved between sessions.
       */
      void writeSettings(QSettings &settings) const;

      /**
       * Read in the settings that have been saved for the engine instance.
       */
      void readSettings(QSettings &settings);

    public Q_SLOTS:
      void addPrimitive(Primitive *primitive);
      void updatePrimitive(Primitive *primitive);
      void removePrimitive(Primitive *primitive);

    private:
      void updateChains();

      RibbonSettingsWidget *m_settingsWidget;

      int m_type;      // Type of ribbon rendering to do
      double m_radius; // Ribbon radius
      bool m_update;   // Is an update of the chain necessary?
      int m_useNitrogens;
      QList< QVector<Eigen::Vector3d> > m_chains;
      QVector<Color> m_chainColors;

    private Q_SLOTS:
      void settingsWidgetDestroyed();

      /**
       * @param value opacity of the VdW spheres / 20
       */
      void setType(int value);
      void setRadius(int value);
      void setUseNitrogens(int setting);

  };

  class RibbonSettingsWidget : public QWidget, public Ui::RibbonSettingsWidget
  {
    public:
      RibbonSettingsWidget(QWidget *parent=0) : QWidget(parent) {
        setupUi(this);
      }
  };

  //! Generates instances of our RibbonEngine class
  class RibbonEngineFactory : public QObject, public PluginFactory
  {
    Q_OBJECT
    Q_INTERFACES(Avogadro::PluginFactory)
    AVOGADRO_ENGINE_FACTORY(RibbonEngine, tr("Render ribbons."))
  };

} // end namespace Avogadro

#endif
