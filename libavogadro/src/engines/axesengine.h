/**********************************************************************
  AxesEngine - Engine for "axes" display

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

#ifndef __AXESENGINE_H
#define __AXESENGINE_H

#include <avogadro/global.h>
#include <avogadro/engine.h>

#include <QGLWidget>
#include <QObject>

namespace Avogadro {

  //! Axes Engine class.
  class AxesEngine : public Engine
  {
    Q_OBJECT

    public:
      //! Constructor
      AxesEngine(QObject *parent=0);
      //! Destructor
      ~AxesEngine() {}

      //! \name Render Method
      //@{
      bool render(GLWidget *gl);
      //@}
  };

  //! Generates instances of our AxesEngine class
  class AxesEngineFactory : public QObject, public EngineFactory
  {
    Q_OBJECT
    Q_INTERFACES(Avogadro::EngineFactory)

    public:
      Engine *createInstance(QObject *parent = 0) { return new AxesEngine(parent); }
  };

} // end namespace Avogadro

#endif
