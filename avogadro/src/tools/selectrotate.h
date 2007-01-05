/**********************************************************************
  SelectRotate - Selection and Rotation Tool for Avogadro

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

#ifndef __SELECTROTATE_H
#define __SELECTROTATE_H

#include <QGLWidget>
#include <QObject>
#include <QStringList>
#include <QImage>

#include <openbabel/mol.h>

#include <avogadro/glwidget.h>
#include "../tool.h"

const double ROTATION_SPEED = 0.005;

namespace Avogadro {

 class SelectRotate : public QObject, public Tool
  {
    Q_OBJECT
    Q_INTERFACES(Avogadro::Tool)
    public:
      //! Constructor
      SelectRotate();
      //! Deconstructor
      virtual ~SelectRotate() {}

      //! \name Description methods
      //@{
      //! Tool Name (ie Draw)
      virtual QString name() { return(QString(tr("Select/Rotate"))); }
      //! Tool Description (ie. Draws atoms and bonds)
      virtual QString description() { return(QString(tr("Selection and Rotation Tool"))); }
      //@}

      //! \name Tool Methods
      //@{
      //! \brief Callback methods for actions on the canvas.
      /*!
        */
      virtual void initialize();
      virtual void cleanup();

      virtual void mousePress(GLWidget *widget, const QMouseEvent *event);
      virtual void mouseRelease(GLWidget *widget, const QMouseEvent *event);
      virtual void mouseMove(GLWidget *widget, const QMouseEvent *event);

    protected:
      void selectionBox(float sx, float sy, float ex, float ey);

      bool                _leftButtonPressed;  // rotation
      bool                _rightButtonPressed; // translation
      bool                _midButtonPressed;   // scale / zoom
      bool                _movedSinceButtonPressed;

      //! Temporary var for adding selection box
      GLuint _selectionDL;

      QPoint              _initialDraggingPosition;
      QPoint              _lastDraggingPosition;

      QList<GLHit> _hits;

  };

} // end namespace Avogadro

#endif
