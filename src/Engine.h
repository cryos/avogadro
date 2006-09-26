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

#include "Primitives.h"

#include <QString>
#include <QVector>
#include <QList>

namespace Avogadro {

  class PrimitiveQueue
  {
    public:
      PrimitiveQueue() { for( int i=0; i<numTypes; i++ ) { _queue.append(new QList<Primitive *>()); } }

      QList<Primitive *>* getTypeQueue(int t) { 
        return(_queue[t]); 
      }

      void add(Primitive *p) { _queue[p->getType()]->append(p); }
      void clear() {
        for( int i=0; i<_queue.size(); i++ ) {
          _queue[i]->clear();
        }
      }

    private:
      QList< QList<Primitive *>* > _queue;
  };

  //! Base class for our GL engines
 class Engine
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
      //! Render a primitive queue.
      virtual bool render(PrimitiveQueue *)   { return false;}
      //@}

  };

  //! Generates instances of our Engine class
  class EngineFactory
  {
    public:
      //! Desconstructor
      virtual ~EngineFactory() {}

      //! Create a new instance of our engine and return a pointer to it.
      virtual Engine *createInstance() = 0;
  };


} // end namespace Avogadro

Q_DECLARE_INTERFACE(Avogadro::EngineFactory, "net.sourceforge.avogadro/1.0")

#endif
