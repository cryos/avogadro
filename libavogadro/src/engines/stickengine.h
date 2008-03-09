/**********************************************************************
  StickEngine - Engine for "stick" display

  Copyright (C) 2006-2007 Geoffrey R. Hutchison

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

#ifndef __STICKENGINE_H
#define __STICKENGINE_H

#include <avogadro/global.h>
#include <avogadro/engine.h>

#include "ui_sticksettingswidget.h"

namespace Avogadro {

  //! Stick / Licorice Engine class.
  class StickSettingsWidget;
  class StickEngine : public Engine
  {
    Q_OBJECT
    AVOGADRO_ENGINE(tr("Stick"))

    public:
      //! Constructor
      StickEngine(QObject *parent=0);
      //! Deconstructor
      ~StickEngine();
      
      //! Copy 
      Engine *clone() const;
 
      //! \name Render Methods
      //@{
      //! Render an Atom.
      bool renderOpaque(PainterDevice *pd, const Atom *a);
      //! Render a Bond.
      bool renderOpaque(PainterDevice *pd, const Bond *b);

      bool renderOpaque(PainterDevice *pd);
      //@}

      double transparencyDepth() const;
      EngineFlags flags() const;
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
      inline double radius(const Atom *a) const;

      StickSettingsWidget *m_settingsWidget;

			double m_radius; //!< The radius of the stick bonds
			
		private Q_SLOTS:
	    void settingsWidgetDestroyed();


	    /**
	     * @param value radius of the sticks / 20
	     */
	    void setRadius(int value);
  };

  class StickSettingsWidget : public QWidget, public Ui::StickSettingsWidget
  {
    public:
      StickSettingsWidget(QWidget *parent=0) : QWidget(parent) {
        setupUi(this);
      }
  };

  //! Generates instances of our StickEngine class
  class StickEngineFactory : public QObject, public EngineFactory
  {
    Q_OBJECT
    Q_INTERFACES(Avogadro::EngineFactory)
    AVOGADRO_ENGINE_FACTORY(StickEngine)
  };

} // end namespace Avogadro

#endif
