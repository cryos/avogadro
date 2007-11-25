/**********************************************************************
  StickEngine - Engine for "stick" display

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

#ifndef __STICKENGINE_H
#define __STICKENGINE_H

#include <avogadro/global.h>
#include <avogadro/engine.h>

#include <openbabel/mol.h>

namespace Avogadro {

  //! Ball and Stick Engine class.
  class StickEngine : public Engine
  {
    Q_OBJECT

    public:
      //! Constructor
      StickEngine(QObject *parent=0);
      //! Deconstructor
      ~StickEngine();

      //! \name Render Methods
      //@{
      //! Render an Atom.
      bool renderOpaque(PainterDevice *pd, const Atom *a);
      //! Render a Bond.
      bool renderOpaque(PainterDevice *pd, const Bond *b);

      bool renderOpaque(PainterDevice *pd);
      //@}

      double transparencyDepth() const;
      EngineFlags flags() const;
      double radius(const PainterDevice *pd, const Primitive *p = 0) const;

    private:
      inline double radius(const Atom *a) const;
  };

  //! Generates instances of our StickEngine class
  class StickEngineFactory : public QObject, public EngineFactory
  {
    Q_OBJECT
    Q_INTERFACES(Avogadro::EngineFactory)

    public:
      Engine *createInstance(QObject *parent = 0) { return new StickEngine(parent); }
      QString className() { return StickEngine::staticMetaObject.className(); }
  };

} // end namespace Avogadro

#endif
