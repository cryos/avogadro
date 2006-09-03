/**********************************************************************
  StickEngine - Engine for "sticks" display

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

#ifndef __STICKENGINE_H
#define __STICKENGINE_H

#include <QGLWidget>
#include <QObject>
#include <QStringList>
#include <QImage>

#include <openbabel/mol.h>

#include "Engine.h"
#include "vertexarray.h"

namespace Avogadro {

  //! Stick Engine class.
  class StickEngine : public QObject, public Engine
  {
    Q_OBJECT

    public:
      //! Constructor
      StickEngine() : Engine() {}
      //! Deconstructor
      ~StickEngine() {}

      //! \name Description methods
      //@{
      //! Engine Name; "StickEngine"
      QString name() { return(QString(tr("StickEngine"))); }
      //! Return Engine description; "Sticks"
      QString description() { return(QString(tr("Sticks"))); }
      //@}

      //! \name Render Methods
      //@{
      bool render(Atom *a);
      bool render(Bond *b);
      //@}

    private:
      Sphere m_sphere;
      Cylinder m_cylinder;
  };

  //! Generates instances of our StickEngine class
  class StickEngineFactory : public QObject, public EngineFactory
  {
    Q_OBJECT
    Q_INTERFACES(Avogadro::EngineFactory)

    public:
      Engine *createInstance() { return new StickEngine(); }
  };

} // end namespace Avogadro

#endif
