/**********************************************************************
  OrbitalEngine - Engine for display of molecular orbitals

  Copyright (C) 2008 Marcus D. Hanwell
  Copyright (C) 2008 Geoffrey R. Hutchison
  Copyright (C) 2008 Tim Vandermeersch

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

#ifndef __ORBITALENGINE_H
#define __ORBITALENGINE_H

#include <avogadro/global.h>
#include <avogadro/engine.h>
#include <avogadro/color.h>

#include "iso.h"
#include "ui_orbitalsettingswidget.h"

namespace Avogadro {

  class OrbitalSettingsWidget;

  //! Surface Engine class.
  class OrbitalEngine : public Engine
  {
    Q_OBJECT
    AVOGADRO_ENGINE(tr("Orbitals"))

    public:
      //! Constructor
      OrbitalEngine(QObject *parent=0);
      //! Deconstructor
      ~OrbitalEngine();

      //! \name Render Methods
      //@{
      bool renderOpaque(PainterDevice *pd);
      bool renderTransparent(PainterDevice *pd);
      bool renderQuick(PainterDevice *pd);
      //@}

      double transparencyDepth() const;
      EngineFlags flags() const;

      Engine *clone() const;

      QWidget* settingsWidget();

      //void writeSettings(QSettings &settings) const;
      //void readSettings(QSettings &settings);

      void setPrimitives(const PrimitiveList &primitives);

    public Q_SLOTS:
      void addPrimitive(Primitive *primitive);
      void updatePrimitive(Primitive *primitive);
      void removePrimitive(Primitive *primitive);

    protected:
      OrbitalSettingsWidget *m_settingsWidget;
      Grid *m_grid;
      Grid *m_grid2;
      IsoGen *m_isoGen;
      IsoGen *m_isoGen2;
      Eigen::Vector3f m_min;
      Color  m_posColor;
      Color  m_negColor;
      double m_alpha;
      double m_stepSize;
      double m_iso;
      int    m_renderMode;
      bool   m_interpolate;
      bool   m_update;

      void updateSurfaces(PainterDevice *pd);

    private Q_SLOTS:
      void isoGenFinished();
      void settingsWidgetDestroyed();
      /**
       * @param value orbital index
       */
      void setOrbital(int value);
      /**
       * @param value opacity of the surface / 20
       */
      void setOpacity(int value);
      /**
       * @param value render mode (0 = fill, 1 = wireframe, 2 = points)
       */
      void setRenderMode(int value);
      /**
       * @param value interpolate (0 = no, 1 = yes)
       */
      void setInterpolate(int value);
      /**
       * @param d the value of the iso surface to be rendered
       */
      void setIso(double d);
      /**
       * @param color the color for the positive iso surface
       */
      void setPosColor(QColor color);
      /**
       * @param color the color for the negative iso surface
       */
      void setNegColor(QColor color);
  };

  class OrbitalSettingsWidget : public QWidget, public Ui::OrbitalSettingsWidget
  {
    public:
      OrbitalSettingsWidget(QWidget *parent=0) : QWidget(parent) {
        setupUi(this);
      }
  };

  //! Generates instances of our OrbitalEngine class
  class OrbitalEngineFactory : public QObject, public EngineFactory
  {
    Q_OBJECT
    Q_INTERFACES(Avogadro::EngineFactory)
    AVOGADRO_ENGINE_FACTORY(OrbitalEngine);

  };

} // end namespace Avogadro

#endif
