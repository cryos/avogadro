/**********************************************************************
  BSEngine - Engine for "balls and sticks" display

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

#ifndef __BSRENDER_H
#define __BSRENDER_H

#include <QGLWidget>
#include <QObject>
#include <QStringList>
#include <QImage>

#include <openbabel/mol.h>

#include <GLEngine.h>

namespace Avogadro {

  //! Ball and Stick Engine class.
  class BSEngine : public QObject, public GLEngine
  {
    Q_OBJECT

    public:
      //! Constructor
      BSEngine() : GLEngine(), atomDL(0), bondDL(0) {}
      //! Deconstructor
      ~BSEngine() {}

      //! \name Description methods
      //@{
      //! Engine Name; "BSEngine"
      QString name() { return(QString(tr("BSEngine"))); }
      //! Return Engine description; "Ball and Stick"
      QString description() { return(QString(tr("Ball and Stick"))); }
      //@}

      //! \name Render Methods
      //@{
      //! Render an Atom.
      void render(Atom *a);
      //! Render a Bond.
      void render(Bond *b);
      //@}

    private:
      GLuint dlist;

      void initAtomDL();
      GLuint atomDL;
      GLuint bondDL;

  };

  //! Generates instances of our BSEngine class
  class BSEngineFactory : public QObject, public GLEngineFactory
  {
    Q_OBJECT
      Q_INTERFACES(Avogadro::GLEngineFactory)

    public:
      GLEngine *createInstance() { return new BSEngine(); }
  };

} // end namespace Avogadro

#endif
