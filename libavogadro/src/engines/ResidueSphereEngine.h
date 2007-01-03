/**********************************************************************
  ResidueSphereEngine - Engine for "balls" for each residue

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

#ifndef __BSENGINE_H
#define __BSENGINE_H

#include <QGLWidget>
#include <QObject>
#include <QStringList>
#include <QImage>

#include <openbabel/mol.h>

#include "Engine.h"
#include "vertexarray.h"

namespace Avogadro {

  //! Ball and Stick Engine class.
  class ResidueSphereEngine : public QObject, public Engine
  {
    Q_OBJECT

    public:
      //! Constructor
  ResidueSphereEngine() : Engine() {}
      //! Deconstructor
      ~ResidueSphereEngine() {}

      //! \name Description methods
      //@{
      //! Engine Name; "ResidueSphereEngine"
      QString name() { return(QString(tr("ResidueSphereEngine"))); }
      //! Return Engine description; "Ball and Stick"
      QString description() { return(QString(tr("Residue Spheres"))); }
      //@}

      //! \name Render Methods
      //@{
      bool render(Molecule *m);
      //@}

    private:
      Sphere m_sphere;
  };

  //! Generates instances of our ResidueSphereEngine class
  class ResidueSphereEngineFactory : public QObject, public EngineFactory
  {
    Q_OBJECT
      Q_INTERFACES(Avogadro::EngineFactory)

    public:
      Engine *createInstance() { return new ResidueSphereEngine(); }
  };

} // end namespace Avogadro

#endif
