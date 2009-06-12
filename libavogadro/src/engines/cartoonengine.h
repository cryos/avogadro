/**********************************************************************
  CartoonEngine - Engine for protein structures.

  Copyright (C) 2009 by Tim Vandermeersch
  Some portions Copyright (C) 2007-2008 by Marcus D. Hanwell

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

#ifndef CARTOONENGINE_H
#define CARTOONENGINE_H

#include <avogadro/global.h>
#include <avogadro/engine.h>

#include <QPointer>
#include "ui_cartoonsettingswidget.h"

namespace Avogadro {

  class Mesh;
  class CartoonSettingsWidget;

  //! CartoonEngine class.
  class CartoonEngine : public Engine
  {
    Q_OBJECT
      AVOGADRO_ENGINE("Cartoon", tr("Cartoon"),
                      tr("Renders protein secondary structure"))

    public:
      //! Constructor
      CartoonEngine(QObject *parent=0);

      Engine *clone() const;

      //! Deconstructor
      ~CartoonEngine();

      //! \name Render Methods
      //@{
      bool renderOpaque(PainterDevice *pd);
      bool renderQuick(PainterDevice *pd);
      //@}

      PrimitiveTypes primitiveTypes() const;
      ColorTypes colorTypes() const;

      double radius(const PainterDevice *pd, const Primitive *p = 0) const;

      void setPrimitives(const PrimitiveList &primitives);
      
      QWidget* settingsWidget();

      bool hasSettings() { return true; }

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

    private:
      void updateMesh(PainterDevice *pd);
      bool m_update;   // Is an update of the mesh necessary?
      
      // store the mesh as QPointer so the pointer will always be
      // set to 0 when the object gets deleted.
      QPointer<Mesh> m_mesh;
      CartoonSettingsWidget *m_settingsWidget;

      // shape parameters
      double m_aHelix, m_bHelix, m_cHelix;
      double m_aSheet, m_bSheet, m_cSheet;
      double m_aLoop, m_bLoop, m_cLoop;

      // colors
      QColor m_helixColor, m_sheetColor, m_loopColor;
    
    private Q_SLOTS:
      void settingsWidgetDestroyed();
      void setHelixA(double value);
      void setHelixB(double value);
      void setHelixC(double value);
      void setSheetA(double value);
      void setSheetB(double value);
      void setSheetC(double value);
      void setLoopA(double value);
      void setLoopB(double value);
      void setLoopC(double value);
      
      void setHelixColor(QColor); 
      void setSheetColor(QColor); 
      void setLoopColor(QColor); 
  };

  class CartoonSettingsWidget : public QWidget, public Ui::CartoonSettingsWidget
  {
    public:
      CartoonSettingsWidget(QWidget *parent=0) : QWidget(parent) {
        setupUi(this);
      }
  };

  //! Generates instances of our CartoonEngine class
  class CartoonEngineFactory : public QObject, public PluginFactory
  {
    Q_OBJECT
    Q_INTERFACES(Avogadro::PluginFactory)
    AVOGADRO_ENGINE_FACTORY(CartoonEngine)
  };

} // end namespace Avogadro

#endif
