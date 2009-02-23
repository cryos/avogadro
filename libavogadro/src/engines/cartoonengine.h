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


//#include "ui_ribbonsettingswidget.h"

namespace Avogadro {

  //! CartoonEngine class.
  class CartoonEngine : public Engine
  {
    Q_OBJECT
      AVOGADRO_ENGINE("Cartoon", tr("Cartoon"))

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
      void updateChains(PainterDevice *pd);

      int m_type;      // Type of ribbon rendering to do
      double m_radius; // Cartoon radius
      bool m_update;   // Is an update of the chain necessary?
      int m_useNitrogens;
      QList< QVector<Eigen::Vector3d> > m_chains;
      
      QList<Eigen::Vector3d> m_triangles;
      QList<Eigen::Vector3d> m_normals;

      QList<Eigen::Vector3d> m_debugPoints;
      
      QList< QVector<Eigen::Vector3d> > m_helixes3;
      QList< QVector<Eigen::Vector3d> > m_helixes4;
      QList< QVector<Eigen::Vector3d> > m_helixes5;

  };

  //! Generates instances of our CartoonEngine class
  class CartoonEngineFactory : public QObject, public PluginFactory
  {
    Q_OBJECT
    Q_INTERFACES(Avogadro::PluginFactory)
    AVOGADRO_ENGINE_FACTORY(CartoonEngine, "Cartoon", tr("Cartoon"), tr("Render protein secundary structure."))
  };

} // end namespace Avogadro

#endif
