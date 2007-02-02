/**********************************************************************
  Engine - Qt Plugin Template

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

#ifndef __ENGINE_H
#define __ENGINE_H

#include "config.h"

#include <avogadro/primitives.h>

#include <QString>
#include <QVector>
#include <QList>

namespace Avogadro {

  //! Base class for our GL engines
 class A_EXPORT Engine
  {
    public:
      //! Deconstructor
      virtual ~Engine() {}

      //! \name Description methods
      //@{
      //! Engine Name (ie FoobarEngine)
      virtual QString name() = 0;
      //! Engine Description (ie. Ball and Stick / Wireframe)
      virtual QString description() = 0;
      //@}

      //! \name Render Methods
      //@{
      /*! Render a Molecule object

        Render an entire molecule all at once, rather than a specific atom
        or bond. Note that while this may provide for optimization,
        additional glue code may be needed to be sure glPushName is used 
        properly.
      */

      //! Render a primitive queue.
      virtual bool render(const PrimitiveQueue *) = 0;
      //@}

  };

  //! Generates instances of our Engine class
  class A_EXPORT EngineFactory
  {
    public:
      //! Desconstructor
      virtual ~EngineFactory() {}

      //! Create a new instance of our engine and return a pointer to it.
      virtual Engine *createInstance() = 0;
  };


} // end namespace Avogadro

Q_DECLARE_INTERFACE(Avogadro::EngineFactory, "net.sourceforge.avogadro.enginefactory/1.0")

#endif
