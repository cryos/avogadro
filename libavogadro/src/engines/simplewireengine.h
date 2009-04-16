/**********************************************************************
  WireEngine - Simple engine for fast wire frame display

  Copyright (C) 2009 Marcus D. Hanwell

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

#ifndef SIMPLEWIREENGINE_H
#define SIMPLEWIREENGINE_H

#include <avogadro/global.h>
#include <avogadro/engine.h>

namespace Avogadro {

  //! SimpleWireframe Engine class.
  class SimpleWireEngine : public Engine
  {
    Q_OBJECT
    AVOGADRO_ENGINE("Simple Wireframe", tr("Simple Wireframe"),
                    tr("Renders bonds as wires (lines), ideal for large molecules"))

    public:
      //! Constructor
      SimpleWireEngine(QObject *parent=0);
      //! Destructor
      ~SimpleWireEngine();

      //! Copy
      Engine *clone() const;

      //! \name Render Methods
      //@{
      bool renderOpaque(PainterDevice *pd);
      //@}
  };

  //! Generates instances of our WireEngine class
  class SimpleWireEngineFactory : public QObject, public PluginFactory
  {
    Q_OBJECT
    Q_INTERFACES(Avogadro::PluginFactory)
    AVOGADRO_ENGINE_FACTORY(SimpleWireEngine)
  };

} // end namespace Avogadro

#endif
