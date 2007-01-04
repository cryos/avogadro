/**********************************************************************
  Tool - Qt Plugin Template

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

#ifndef __TOOL_H
#define __TOOL_H

#include <avogadro/primitives.h>

#include <QString>
#include <QGLWidget>
#include <QVector>
#include <QList>

namespace Avogadro {

  class GLWidget;

  //! Base class for our tools
  class Tool
  {
    public:
      //! Deconstructor
      virtual ~Tool() {}

      //! \name Description methods
      //@{
      //! Tool Name (ie Draw)
      virtual QString name() = 0;
      //! Tool Description (ie. Draws atoms and bonds)
      virtual QString description() = 0;
      //@}

      //! \name Tool Methods
      //@{
      //! \brief Callback methods for actions on the canvas.
      /*! Random calls.  */
      virtual void init() {}
      virtual void cleanup() {}

      virtual void mousePress(GLWidget *widget, const QMouseEvent *event) {}
      virtual void mouseRelease(GLWidget *widget, const QMouseEvent *event) {}
      virtual void mouseMove(GLWidget *widget, const QMouseEvent *event) {}
      //@}

//X:     protected:
//X:       //! \brief Select a region of the widget.
//X:       //! (x,y) top left coordinate of region.
//X:       //! (w,h) width and heigh of region.
//X:       QList<GLHit> selectRegion(GLWidget *widget, int x, int y, int w, int h);

  };

} // end namespace Avogadro

Q_DECLARE_INTERFACE(Avogadro::Tool, "net.sourceforge.avogadro/1.0")

#endif
