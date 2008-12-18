/**********************************************************************
  SphereEngine - Engine for "spheres" display

  Copyright (C) 2007-2008 Marcus D. Hanwell
  Copyright (C) 2006-2007 Geoffrey R. Hutchison
  Copyright (C) 2007      Benoit Jacob

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

#ifndef SPHEREENGINE_H
#define SPHEREENGINE_H

#include <avogadro/global.h>
#include <avogadro/engine.h>


#include "ui_spheresettingswidget.h"

namespace Avogadro {

  //! Sphere Engine class.
  class Atom;
  class SphereSettingsWidget;
  class SphereEngine : public Engine
  {
    Q_OBJECT
    AVOGADRO_ENGINE(tr("Van der Waals Spheres"))

    public:
      //! Constructor
      SphereEngine(QObject *parent=0);
      //! Deconstructor
      ~SphereEngine();

      //! Copy
      Engine *clone() const;

      //! \name Render Methods
      //@{
      bool renderOpaque(PainterDevice *pd);
      bool renderTransparent(PainterDevice *pd);
      bool renderQuick(PainterDevice *pd);
      //! Render an Atom.
      bool render(PainterDevice *pd, const Atom *a);
      //@}

      double transparencyDepth() const;
      Layers layers() const;
      PrimitiveTypes primitiveTypes() const;

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

      SphereSettingsWidget *m_settingsWidget;

      double m_alpha; // transparency of the VdW spheres

    private Q_SLOTS:
      void settingsWidgetDestroyed();


      /**
       * @param value opacity of the VdW spheres / 20
       */
      void setOpacity(int value);

  };

  class SphereSettingsWidget : public QWidget, public Ui::SphereSettingsWidget
  {
    public:
      SphereSettingsWidget(QWidget *parent=0) : QWidget(parent) {
        setupUi(this);
      }
  };

  //! Generates instances of our SphereEngine class
  class SphereEngineFactory : public QObject, public PluginFactory
  {
    Q_OBJECT
    Q_INTERFACES(Avogadro::PluginFactory)
    AVOGADRO_ENGINE_FACTORY(SphereEngine, tr("Render Van der Waals spheres."))
  };

} // end namespace Avogadro

#endif
