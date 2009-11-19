/**********************************************************************
  AxesEngine - Engine to display the x, y, z axes

  Copyright (C) 2008      Marcus D. Hanwell

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

#ifndef AXESENGINE_H
#define AXESENGINE_H

#include <avogadro/global.h>
#include <avogadro/engine.h>

#include "ui_axessettingswidget.h"

namespace Avogadro {

  //! Settings class
  class AxesSettingsWidget;

  //! Axes Engine class.
  class AxesEngine : public Engine
  {
    Q_OBJECT
    AVOGADRO_ENGINE("Axes", tr("Axes"), 
                    tr("Renders the x, y, and z axes at the origin"))

    public:
      //! Constructor
      AxesEngine(QObject *parent=0);
      //! Deconstructor
      ~AxesEngine();

      //! Copy
      Engine *clone() const;

      //! \name Render Methods
      //@{
      bool renderOpaque(PainterDevice *pd);
      //@}

      double transparencyDepth() const;
      Layers layers() const;
      PrimitiveTypes primitiveTypes() const;
      ColorTypes colorTypes() const;

      double radius(const PainterDevice *pd, const Primitive *p = 0) const;
	  
      //! Display a window for the user to pick axes' options
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
      int m_axesType;
	  Eigen::Vector3d m_origin;
	  Eigen::Vector3d m_axis1;
	  Eigen::Vector3d m_axis2;
	  Eigen::Vector3d m_axis3;
	  bool m_preserveNorms;
	  AxesSettingsWidget* m_settingsWidget;

	private Q_SLOTS:
	  void setAxesType(int value);
	  void settingsWidgetDestroyed();
	  void updateAxes(double = 0.0);  
	  void updateVectors();
	  void updateOrigin(double = 0.0);
	  void updateValues1(double = 0.0);
	  void updateValues2(double = 0.0);
	  void updateValues3(double = 0.0);
	  void preserveNormsChanged(int value);
  };
  
  class AxesSettingsWidget : public QWidget, public Ui::AxesSettingsWidget
  {
    public:
      AxesSettingsWidget(QWidget *parent=0) : QWidget(parent) {
        setupUi(this);
      }
  };
  //! Generates instances of our AxesEngine class
  class AxesEngineFactory : public QObject, public PluginFactory
  {
    Q_OBJECT
    Q_INTERFACES(Avogadro::PluginFactory)
    AVOGADRO_ENGINE_FACTORY(AxesEngine)
  };

} // end namespace Avogadro

#endif
