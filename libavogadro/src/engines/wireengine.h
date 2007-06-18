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

namespace Avogadro {

  //! Wireframe Engine class.
  class WireEngine : public Engine
  {
    Q_OBJECT

    public:
      //! Constructor
      WireEngine(QObject *parent=0);
      //! Deconstructor
      ~WireEngine() {}

      //! \name Render Methods
      //@{
      //! Render an Atom.
      bool renderOpaque(const Atom *a);
      //! Render a Bond.
      bool renderOpaque(const Bond *b);
      //! Render a Molecule.
      bool renderOpaque(const Molecule *m);

      bool renderOpaque(GLWidget *gl);
      //@}

//      double radius(const Primitive *p = 0);

    private:
//      inline double radius(const Atom *a);

      GLWidget *m_glwidget;
  };

  //! Generates instances of our WireEngine class
  class WireEngineFactory : public QObject, public EngineFactory
  {
    Q_OBJECT
    Q_INTERFACES(Avogadro::EngineFactory)

    public:
      Engine *createInstance(QObject *parent = 0) { return new WireEngine(parent); }
  };

} // end namespace Avogadro

#endif
