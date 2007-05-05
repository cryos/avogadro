/**********************************************************************
  StickEngine - Engine for "stick" cylinder display

  Copyright (C) 2006-2007 by Geoffrey R. Hutchison
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

#ifndef __STICKENGINE_H
#define __STICKENGINE_H

#include <avogadro/global.h>
#include <avogadro/engine.h>
#include <avogadro/sphere.h>
#include <avogadro/cylinder.h>

#include <openbabel/mol.h>

#include <QGLWidget>
#include <QObject>
#include <QStringList>
#include <QImage>

namespace Avogadro {

  //! Ball and Stick Engine class.
  class StickEngine : public Engine
  {
    Q_OBJECT

    public:
      //! Constructor
      StickEngine(QObject *parent=0) : Engine(parent), m_setup(false), m_update(true), m_dl(0) {}
      //! Deconstructor
      ~StickEngine();

      //! \name Description methods
      //@{
      //! @return engine name
      QString name() { return(QString(tr("Stick"))); }
      //! @return engine description
      QString description() { return(QString(tr("Renders as Cylinders"))); }
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

      void addPrimitive(Primitive *);
      void updatePrimitive(Primitive *);
      void removePrimitive(Primitive *);

      double radius(const Primitive *p = 0);

      //! Display a window for the user to pick rendering options
      void options();

    private:
      QList<Sphere *> m_spheres;
      QList<Cylinder *> m_cylinders;

      bool m_setup; //!< Whether the sphere and cylinder objects have been setup

      GLuint m_dl;
      bool m_update;
  };

  //! Generates instances of our StickEngine class
  class StickEngineFactory : public QObject, public EngineFactory
  {
    Q_OBJECT
    Q_INTERFACES(Avogadro::EngineFactory)

    public:
      Engine *createInstance(QObject *parent = 0) { return new StickEngine(parent); }
  };

} // end namespace Avogadro

#endif
