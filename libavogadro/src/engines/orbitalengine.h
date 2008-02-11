/**********************************************************************
  OrbitalEngine - Engine for display of isosurfaces

  Copyright (C) 2008 Geoffrey R. Hutchison
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

#ifndef __ORBITALENGINE_H
#define __ORBITALENGINE_H

#include <avogadro/global.h>
#include <avogadro/engine.h>

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
      //@}
      
      double transparencyDepth() const;
      EngineFlags flags() const;
      
      Engine *clone() const;
      
      QWidget* settingsWidget();

      //void writeSettings(QSettings &settings) const;
      //void readSettings(QSettings &settings);


    protected:
      OrbitalSettingsWidget *m_settingsWidget;
      Grid *m_grid;
      IsoGen *m_isoGen;
      Eigen::Vector3f m_min;
      Color  m_color;
      double m_alpha;
      double m_stepSize;
      double m_iso;
      int    m_renderMode;
      int    m_colorMode;
    
    private Q_SLOTS:
      void isoGenFinished();
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
      void setIso(double d);
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
