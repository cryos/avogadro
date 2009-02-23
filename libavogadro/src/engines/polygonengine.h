/**********************************************************************
 PolygonEngine - Engine for "polygon" display

 Copyright (C) 2007 by Marcus D. Hanwell

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

#ifndef POLYGONENGINE_H
#define POLYGONENGINE_H

#include <avogadro/global.h>
#include <avogadro/engine.h>

namespace Avogadro {

  class Atom;

  //! Polygon Engine class.
  class PolygonEngine : public Engine
    {
      Q_OBJECT
        AVOGADRO_ENGINE("Polygon", tr("Polygon"))

    public:
      //! Constructor
      PolygonEngine(QObject *parent=0);

      Engine *clone() const;

      //! Deconstructor
      ~PolygonEngine();

      //! \name Render Methods
      //@{
      bool renderOpaque(PainterDevice *pd);
      //@}

      PrimitiveTypes primitiveTypes() const;

      double radius(const PainterDevice *pd, const Primitive *p = 0) const;

    private:
      bool renderPolygon(PainterDevice *pd, Atom *a);

    };

  //! Generates instances of our PolygonEngine class
  class PolygonEngineFactory : public QObject, public PluginFactory
  {
    Q_OBJECT
    Q_INTERFACES(Avogadro::PluginFactory)
      AVOGADRO_ENGINE_FACTORY(PolygonEngine, tr("Renders atoms as tetrahedra, octahedra, and other polygons"))
  };

} // end namespace Avogadro

#endif
