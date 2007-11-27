/**********************************************************************
  SurfaceEngine - Engine for display of isosurfaces

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

#ifndef __SURFACEENGINE_H
#define __SURFACEENGINE_H

#include <avogadro/global.h>
#include <avogadro/engine.h>

namespace Avogadro {

  class SurfacePrivateData;
  
  //! Surface Engine class.
  class SurfaceEngine : public Engine
  {
    Q_OBJECT
    AVOGADRO_ENGINE(tr("Surface"))

    public:
      //! Constructor
      SurfaceEngine(QObject *parent=0);
      //! Deconstructor
      ~SurfaceEngine();

      //! \name Render Methods
      //@{
      bool renderOpaque(PainterDevice *pd);
      //@}

    protected:
    SurfacePrivateData *d;
    
    void SurfaceEngine::VDWSurface(Molecule *mol);
  };

  //! Generates instances of our SurfaceEngine class
  class SurfaceEngineFactory : public QObject, public EngineFactory
  {
    Q_OBJECT
    Q_INTERFACES(Avogadro::EngineFactory)
    AVOGADRO_ENGINE_FACTORY(SurfaceEngine);

  };

} // end namespace Avogadro

#endif
