/**********************************************************************
  RibbonEngine - Engine for "ribbon" display

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

#ifndef __RINGENGINE_H
#define __RINGENGINE_H

#include <avogadro/global.h>
#include <avogadro/engine.h>

#include <openbabel/mol.h>

namespace Avogadro {

  //! Ring Engine class.
  class RingEngine : public Engine
  {
    Q_OBJECT

    public:
      //! Constructor
      RingEngine(QObject *parent=0);
      //! Deconstructor
      ~RingEngine();

      //! \name Render Methods
      //@{
      bool renderOpaque(PainterDevice *pd);
      bool renderTransparent(PainterDevice *pd);
      //@}

      double transparencyDepth() const;
      EngineFlags flags() const;

      double radius(const PainterDevice *pd, const Primitive *p = 0) const;

    private:
      double m_alpha; // transparency of the VdW spheres
	  bool renderRing(const std::vector<int> &ring, PainterDevice *pd); // Render the given ring

  };

  //! Generates instances of our RingEngine class
  class RingEngineFactory : public QObject, public EngineFactory
  {
    Q_OBJECT
    Q_INTERFACES(Avogadro::EngineFactory)

    public:
      Engine *createInstance(QObject *parent = 0) { return new RingEngine(parent); }
  };

} // end namespace Avogadro

#endif
