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
#include <avogadro/color.h>

#include "iso.h"
#include "ui_surfacesettingswidget.h"

namespace Avogadro {

  class SurfacePrivateData;
  class SurfaceSettingsWidget;
  
  //! VDWGridThread
  class VDWGridThread : public QThread
  {
    Q_OBJECT

    public:
      VDWGridThread(QObject *parent=0);
      ~VDWGridThread();

      void init(Molecule *molecule, PrimitiveList &primitives, const PainterDevice* pd, 
                double stepSize = 0.0);
      void run();
      Grid* grid();
      double stepSize();

    private:
      QMutex m_mutex;
      Molecule *m_molecule;
      PrimitiveList m_primitives;
      Grid *m_grid;
      double m_stepSize;
      double m_padding;
  };


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
      void doWork(Molecule *mol);
      //@}

      double transparencyDepth() const;
      EngineFlags flags() const;

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

    protected:
      SurfaceSettingsWidget *m_settingsWidget;
      //Grid *m_grid;
      VDWGridThread *m_vdwThread;
      IsoGen *m_isoGen;
      //Eigen::Vector3f m_min;
      PainterDevice *m_pd;
      Color  m_color;
      double m_alpha;
      double m_stepSize;
      double m_padding;
      int    m_renderMode;
      int    m_colorMode;
      bool   m_surfaceValid;

      inline double radius(const Atom *a) const;
      //void VDWSurface(Molecule *mol);
      Color espColor(Molecule *mol, Eigen::Vector3f &pos);

      // clipping stuff
      bool m_clip;
      double m_clipEqA, m_clipEqB, m_clipEqC, m_clipEqD;
      // clipping stuff

    private Q_SLOTS:
      void vdwThreadFinished();
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
       * @param value coloring mode (0 = RGB, 1 = ESP)
       */
      void setColorMode(int value);
      /**
       * @param color the new color to use
       */
      void setColor(const QColor& color);

      // clipping stuff
      void setClipEnabled(int value) 
      { 
        m_clip = value; 
        emit changed();
      }
      void setClipEqA(double value) 
      {
        m_clipEqA = value; 
        emit changed();
      } 
      void setClipEqB(double value) 
      {
        m_clipEqB = value; 
        emit changed();
      }
      void setClipEqC(double value) 
      {
        m_clipEqC = value; 
        emit changed();
      }
      void setClipEqD(double value) 
      {
        m_clipEqD = value; 
        emit changed();
      }
      // clipping stuff

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
    AVOGADRO_ENGINE_FACTORY(SurfaceEngine)

  };

} // end namespace Avogadro

#endif
