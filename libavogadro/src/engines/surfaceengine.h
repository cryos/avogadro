/**********************************************************************
  SurfaceEngine - Engine for display of isosurfaces

  Copyright (C) 2006-2007 Geoffrey R. Hutchison
  Copyright (C) 2008 Marcus D. Hanwell
  Copyright (C) 2008 Tim Vandermeersch

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

#ifndef SURFACEENGINE_H
#define SURFACEENGINE_H

#include <avogadro/global.h>
#include <avogadro/engine.h>
#include <avogadro/color.h>
#include <avogadro/point.h>
#include <avogadro/line.h>

#include "ui_surfacesettingswidget.h"

namespace Avogadro {

  class Atom;
  class Mesh;
  class SurfaceSettingsWidget;

  //! Surface Engine class.
  class SurfaceEngine : public Engine
  {
    Q_OBJECT
    AVOGADRO_ENGINE("Surface", tr("Surface"),
                    tr("Renders computed molecular surfaces"))

    public:
      //! Constructor
      SurfaceEngine(QObject *parent=0);
      //! Destructor
      ~SurfaceEngine();

      //! \name Render Methods
      //@{
      bool renderOpaque(PainterDevice *pd);
      bool renderTransparent(PainterDevice *pd);
      bool renderQuick(PainterDevice *pd);
      bool renderPick(PainterDevice *pd);
      //@}

      double transparencyDepth() const;
      Layers layers() const;
      PrimitiveTypes primitiveTypes() const;
      ColorTypes colorTypes() const;

      Engine *clone() const;

      QWidget* settingsWidget();

      double radius(const PainterDevice *pd, const Primitive *p = 0) const;

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
      void setDrawBox(int);

    protected:
      SurfaceSettingsWidget *m_settingsWidget;
      Mesh *m_mesh;
      PainterDevice *m_pd;
      Color  m_color;
      double m_alpha;
      int    m_renderMode;
      int    m_colorMode;
      bool   m_drawBox;

    private Q_SLOTS:
      void settingsWidgetDestroyed();
      /**
       * @param value opacity of the surface / 20
       */
      void setOpacity(int value);
      /**
       * @param value render mode (0 = fill, 1 = wireframe, 2 = points)
       */
      void setRenderMode(int value);
      /**
       * @param value coloring mode (0 = RGB, 1 = ESP)
       */
      void setColorMode(int value);
      /**
       * @param color the new color to use
       */
      void setColor(const QColor& color);
  };

  class SurfaceSettingsWidget : public QWidget, public Ui::SurfaceSettingsWidget
  {
    public:
      SurfaceSettingsWidget(QWidget *parent=0) : QWidget(parent) {
        setupUi(this);
      }
  };

  //! Generates instances of our SurfaceEngine class
  class SurfaceEngineFactory : public QObject, public PluginFactory
  {
    Q_OBJECT
    Q_INTERFACES(Avogadro::PluginFactory)
    AVOGADRO_ENGINE_FACTORY(SurfaceEngine)
  };

} // end namespace Avogadro

#endif
