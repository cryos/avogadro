/**********************************************************************
  Debug Engine - Engine for displaying debug information.

  Copyright (C) 2007 Ross Braith
  Copyright (C) 2007 Shahzad Ali
  Copyright (C) 2007 James Bunt

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

#ifndef __DEBUGENGINE_H
#define __DEBUGENGINE_H

#include <avogadro/global.h>
#include <avogadro/engine.h>

#include <openbabel/mol.h>

#include <QGLWidget>

namespace Avogadro {

  //! Debug Engine class.
  class DebugEngine : public Engine
  {
    Q_OBJECT

    public:
      //! Constructor
      DebugEngine(QObject *parent=0) : Engine(parent) {}
      //! Deconstructor
      ~DebugEngine() {}

      //! \name Description methods
      //@{
      //! @return engine name
      QString name() { return(QString(tr("Debug Info"))); }
      //! @return engine description
      QString description() {
        return(QString(tr("Renders debug information")));
      }
      //@}

      //! \name Render Methods
      //@{
      //! Render an Atom.
      bool render(const Atom *a);
      //! Render a Bond.
      bool render(const Bond *b);
      //! Render a Molecule.
      bool render(const Molecule *m);

      bool render(GLWidget *gl);
      //@}

      //! Display a window for the user to pick rendering options
      void options();
      
      private:
      inline double computeFramesPerSecond();
  };

  //! Generates instances of our LabelEngine class
  class DebugEngineFactory : public QObject, public EngineFactory
  {
    Q_OBJECT
    Q_INTERFACES(Avogadro::EngineFactory)

    public:
      Engine *createInstance(QObject *parent = 0) { 
        return new DebugEngine(parent); 
      }
  };

} // end namespace Avogadro

#endif
