/**********************************************************************
  SphereEngine - Engine for "spheres" display

  Copyright (C) 2006-2007 Geoffrey R. Hutchison
  Copyright (C) 2007      Benoit Jacob
  Copyright (C) 2007      Marcus D. Hanwell

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

#ifndef __SPHEREENGINE_H
#define __SPHEREENGINE_H

#include <avogadro/global.h>
#include <avogadro/engine.h>

#include <openbabel/mol.h>

#include <QGLWidget>
#include <QObject>
#include <QStringList>
#include <QImage>

namespace Avogadro {

  //! Sphere Engine class.
  class SphereEngine : public Engine
  {
    Q_OBJECT

    public:
      //! Constructor
      SphereEngine(QObject *parent=0) : Engine(parent), m_setup(false), m_glwidget(0) {}
      //! Deconstructor
      ~SphereEngine() {}

      //! \name Description methods
      //@{
      //! @return engine name
      QString name() { return(QString(tr("Sphere"))); }
      //! @return engine description
      QString description() { return(QString(tr("Renders atoms as spheres"))); }
      //@}

      //! \name Render Methods
      //@{
      //! Render an Atom.
      bool render(const Atom *a);

      bool render(GLWidget *gl);
      //@}

      double radius(const Primitive *p = 0);

    private:
      inline double radius(const Atom *a);

      GLWidget *m_glwidget;
      bool m_setup; //!< Whether the sphere objects have been setup
  };

  //! Generates instances of our SphereEngine class
  class SphereEngineFactory : public QObject, public EngineFactory
  {
    Q_OBJECT
    Q_INTERFACES(Avogadro::EngineFactory)

    public:
      Engine *createInstance(QObject *parent = 0) { return new SphereEngine(parent); }
  };

} // end namespace Avogadro

#endif
