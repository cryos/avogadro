/**********************************************************************
  GLEngine - Qt Plugin Template

  Copyright (C) 2006 by Geoffrey R. Hutchison
  Some portions Copyright (C) 2006 by Donald E. Curtis

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.sourceforge.net/>

  Some code is based on Open Babel
  For more information, see <http://openbabel.sourceforge.net/>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation version 2 of the License.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
 ***********************************************************************/

#ifndef __GLENGINE_H
#define __GLENGINE_H

#include "Primitives.h"

#include <QString>

namespace Avogadro {

  //! Base class for our GL engines
 class GLEngine
  {
    public:
      //! Deconstructor
      virtual ~GLEngine() {}

      //! \name Description methods
      //@{
      //! Engine Name (ie FoobarEngine)
      virtual QString name() = 0;
      //! Engine Description (ie. Ball and Stick / Wireframe)
      virtual QString description() = 0;
      //@}

      //! \name Render Methods
      //@{
      //! \brief Render a Primitive object.
      /*!
        Engine developers need not implement this function.  It is left here as
        a precautionary measure in the case we try to render a subclass of
        Primitive we have not yet declared.
        */
      virtual bool render(Primitive *) { return false; }
      /*! Render a Molecule object

        Render an entire molecule all at once, rather than a specific atom
        or bond. Note that while this may provide for optimization,
        additional glue code may be needed to be sure glPushName is used 
        properly.
      */
      virtual bool render(Molecule *)  { return false; }
      //! Render an Atom object.
      virtual bool render(Atom *)      { return false;}
      //! Render a Bond object.
      virtual bool render(Bond *)      { return false; }
      //! Render a Residue object.
      virtual bool render(Residue *)   { return false;}
      //@}
  };

  //! Generates instances of our GLEngine class
  class GLEngineFactory
  {
    public:
      //! Desconstructor
      virtual ~GLEngineFactory() {}

      //! Create a new instance of our engine and return a pointer to it.
      virtual GLEngine *createInstance() = 0;
  };

} // end namespace Avogadro

Q_DECLARE_INTERFACE(Avogadro::GLEngineFactory, "net.sourceforge.avogadro/1.0")

#endif
