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

#include <Eigen/Core>
#include <Eigen/Geometry>

#include "ui_bsdysettingswidget.h"

namespace OpenBabel {
  class OBUnitCell;
}

namespace Avogadro {

  //! Ball and Stick Engine class.
  class Atom;
  class Bond;
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

      bool hasSettings() { return true; }

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

      /**
       * Find where @a bond intersects the @a planes in such a way that the
       * bonds must be rendered differently. Returns the number of valid
       * intersections.
       */
      int findCellIntersections(const Bond *bond,
                                Eigen::Vector3d &imageVector,
                                Eigen::Vector3d &shortestBondVector,
                                OpenBabel::OBUnitCell *cell,
                                Eigen::Hyperplane<double, 3> planes[6],
                                double intParams[6],
                                int validIntersections[3]);

      bool renderIntracellBond(const Bond *bond, PainterDevice *pd,
                               Color *map, double extraRadius = 0.0);
      bool renderIntercellBond(const Bond *bond, PainterDevice *pd,
                               Color *map,
                               OpenBabel::OBUnitCell *cell,
                               const Eigen::Vector3d &imageVector,
                               const Eigen::Vector3d &shortestVector,
                               Eigen::Hyperplane<double, 3> planes[6],
                               double intParams[6],
                               int validIntersections[3],
                               int numValidIntersections,
                               double extraRadius = 0.0);

      BSDYSettingsWidget *m_settingsWidget;

      double m_atomRadiusPercentage;
      const double m_atomRadiusScale;
      double m_bondRadius;
      const double m_bondRadiusScale;
      int m_atomRadiusType;
      int m_showMulti;

      double m_alpha; // transparency of the balls & sticks

      /**
       * Function pointer for the radius function to be used for rendering.
       */
      double (*pRadius)(const Atom *atom);

   private Q_SLOTS:
      void settingsWidgetDestroyed();

      /**
       * @param percent percentage of the VdwRad
       */
      void setAtomRadiusPercentage(int percent);
      /**
       * @param value determines if covalent or VdW radii are used
       */
      void setAtomRadiusType(int value);
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
