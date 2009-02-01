/**********************************************************************
  DipoleEngine - Engine to display a 3D vector such as the dipole moment

  Copyright (C) 2008      Geoffrey R. Hutchison

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

#ifndef DIPOLEENGINE_H
#define DIPOLEENGINE_H

#include <avogadro/global.h>
#include <avogadro/engine.h>

#include "ui_dipolesettingswidget.h"

namespace Avogadro {

  //! Settings class
  class DipoleSettingsWidget;
  
  //! Dipole Engine class.
  class DipoleEngine : public Engine
  {
    Q_OBJECT
    AVOGADRO_ENGINE(tr("Dipole"))

    public:
      //! Constructor
      DipoleEngine(QObject *parent=0);
      //! Deconstructor
      ~DipoleEngine();

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

      //! Display a window for the user to pick rendering options
      QWidget *settingsWidget();
      
      private:
        Molecule *m_molecule;
        int m_dipoleType; // type of dipole to render
        Eigen::Vector3d m_dipole; // cached dipole moment
        DipoleSettingsWidget* m_settingsWidget;

      private Q_SLOTS:
        void setDipoleType(int value);
        void settingsWidgetDestroyed();
        void updateDipole(double = 0.0);
      
  };

  class DipoleSettingsWidget : public QWidget, public Ui::DipoleSettingsWidget
  {
    public:
      DipoleSettingsWidget(QWidget *parent=0) : QWidget(parent) {
        setupUi(this);
      }
  };

  //! Generates instances of our AxesEngine class
  class DipoleEngineFactory : public QObject, public PluginFactory
  {
    Q_OBJECT
    Q_INTERFACES(Avogadro::PluginFactory)
    AVOGADRO_ENGINE_FACTORY(DipoleEngine, tr("Dipole"), tr("Render dipole moments."))
  };

} // end namespace Avogadro

#endif
