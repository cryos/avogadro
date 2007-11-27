/**********************************************************************
  WireEngine - Engine for wireframe display

  Copyright (C) 2006-2007 Geoffrey R. Hutchison
  Copyright (C) 2006-2007 Benoit Jacob

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

#ifndef __WIREENGINE_H
#define __WIREENGINE_H

#include <avogadro/global.h>
#include <avogadro/engine.h>

#include <openbabel/mol.h>

#include "ui_wiresettingswidget.h"

namespace Avogadro {
  
  class WireSettingsWidget;

  //! Wireframe Engine class.
  class WireEngine : public Engine
  {
    Q_OBJECT
    AVOGADRO_ENGINE(tr("Wireframe"))

    public:
      //! Constructor
      WireEngine(QObject *parent=0);
      //! Deconstructor
      ~WireEngine() {}

      //! \name Render Methods
      //@{
      //! Render an Atom.
      bool renderOpaque(PainterDevice *pd, const Atom *a);
      //! Render a Bond.
      bool renderOpaque(PainterDevice *pd, const Bond *b);

      bool renderOpaque(PainterDevice *pd);
      //@}
    
      //! Configuration options
      QWidget* settingsWidget();
    
  private:
    WireSettingsWidget *m_settingsWidget;
    
    bool m_showMulti; //!< show multiple bonds
    bool m_showDots;  //!< show dots for atoms

    double radius (const Atom *atom) const;
    
  private Q_SLOTS:
    void settingsWidgetDestroyed();
    
    /**
     * @param setting whether to show multiple bonds
     */
    void setShowMultipleBonds(int setting);
    
    /**
     * @param setting whether to show dots for atoms
     */
    void setShowDots(int setting);
  };
  
  class WireSettingsWidget : public QWidget, public Ui::WireSettingsWidget
  {
  public:
    WireSettingsWidget(QWidget *parent=0) : QWidget(parent) {
      setupUi(this);
    }
  };

  //! Generates instances of our WireEngine class
  class WireEngineFactory : public QObject, public EngineFactory
  {
    Q_OBJECT
    Q_INTERFACES(Avogadro::EngineFactory)
    AVOGADRO_ENGINE_FACTORY(WireEngine)
  };

} // end namespace Avogadro

#endif
