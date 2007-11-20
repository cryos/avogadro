/**********************************************************************
 PolygonEngine - Engine for "polygon" display
 
 Copyright (C) 2007 by Marcus D. Hanwell
 
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

#ifndef __POLYGONENGINE_H
#define __POLYGONENGINE_H

#include <avogadro/global.h>
#include <avogadro/engine.h>

#include <openbabel/mol.h>

namespace Avogadro {
  
  //! Polygon Engine class.
  class PolygonEngine : public Engine
    {
      Q_OBJECT
      
    public:
      //! Constructor
      PolygonEngine(QObject *parent=0);
      //! Deconstructor
      ~PolygonEngine();
      
      //! \name Render Methods
      //@{
      bool renderOpaque(PainterDevice *pd);
      bool renderTransparent(PainterDevice *pd);
      //@}
      
      EngineFlags flags() const;
      
      double radius(const PainterDevice *pd, const Primitive *p = 0) const;

    private:
      bool renderPolygon(PainterDevice *pd, Atom *a);

    };

  //! Generates instances of our PolygonEngine class
  class PolygonEngineFactory : public QObject, public EngineFactory
  {
    Q_OBJECT
    Q_INTERFACES(Avogadro::EngineFactory)
    
  public:
    Engine *createInstance(QObject *parent = 0) { return new PolygonEngine(parent); }
  };
  
} // end namespace Avogadro

#endif
