/**********************************************************************
  SurfaceEngine - Engine for display of isosurfaces

  Copyright (C) 2006-2007 Geoffrey R. Hutchison
  Copyright (C) 2008 Marcus D. Hanwell
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

#ifndef __SURFACEENGINE_H
#define __SURFACEENGINE_H

#include <avogadro/global.h>
#include <avogadro/engine.h>

#include "iso.h"
#include "ui_surfacesettingswidget.h"

namespace Avogadro {

  class SurfacePrivateData;
  class SurfaceSettingsWidget;

  //! Surface Engine class.
  class SurfaceEngine : public Engine
  {
    Q_OBJECT
    AVOGADRO_ENGINE(tr("Surface"))

    public:
      //! Constructor
      SurfaceEngine(QObject *parent=0);
      //! Deconstructor
      ~SurfaceEngine();

      //! \name Render Methods
      //@{
      bool renderOpaque(PainterDevice *pd);
      //@}
      
      double transparencyDepth() const;
      EngineFlags flags() const;
      
      Engine *clone() const;
      
      QWidget* settingsWidget();

      //void writeSettings(QSettings &settings) const;
      //void readSettings(QSettings &settings);


    protected:
      SurfaceSettingsWidget *m_settingsWidget;
      Grid *m_grid;
      IsoGen *m_isoGen;
      Eigen::Vector3f m_min;
      Color  m_color;
      double m_alpha;
      double m_stepSize;
      double m_padding;
      int    m_renderMode;
      int    m_colorMode;
      
      void VDWSurface(Molecule *mol);
      Color espColor(Molecule *mol, Eigen::Vector3f &pos);
    
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
       * @param d step size for the surface polygonization
       */
      void setStepSize(double d);
      /**
       * @param d padding for the surface polygonization (how far do we look for parts of the surface)
       */
      void setPadding(double d);
      /**
       * @param value coloring mode (0 = RGB, 1 = ESP)
       */
      void setColorMode(int value);
      /**
       * @param r red
       */
      void setRed(double r);
      /**
       * @param g green
       */
      void setGreen(double g);
      /**
       * @param b blue
       */
      void setBlue(double b);
  };
  
  class SurfaceSettingsWidget : public QWidget, public Ui::SurfaceSettingsWidget
  {
    public:
      SurfaceSettingsWidget(QWidget *parent=0) : QWidget(parent) {
        setupUi(this);
      }
  };

  //! Generates instances of our SurfaceEngine class
  class SurfaceEngineFactory : public QObject, public EngineFactory
  {
    Q_OBJECT
    Q_INTERFACES(Avogadro::EngineFactory)
    AVOGADRO_ENGINE_FACTORY(SurfaceEngine);

  };

} // end namespace Avogadro

#endif
