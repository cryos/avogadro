/**********************************************************************
  SelectRotateTool - Selection and Rotation Tool for Avogadro

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

#ifndef __SELECTROTATETOOL_H
#define __SELECTROTATETOOL_H

#include <avogadro/tool.h>
#include <avogadro/glwidget.h>

#include <QObject>
#include <QString>
#include <QPoint>
#include <QAction>

class QMouseEvent;
class QWheelEvent;

namespace Avogadro {

  class Molecule;


  class SelectRotateTool : public Tool
  {
    Q_OBJECT

    public:
      //! Constructor
      SelectRotateTool(QObject *parent = 0);
      //! Deconstructor
      virtual ~SelectRotateTool();

      //! \name Description methods
      //@{
      //! Tool Name (ie Draw)
      virtual QString name() const { return(tr("Select/Rotate")); }
      //! Tool Description (ie. Draws atoms and bonds)
      virtual QString description() const { return(tr("Selection and Rotation Tool")); }
      //@}

      //! \name Tool Methods
      //@{
      //! \brief Callback methods for ui.actions on the canvas.
      /*!
      */
      virtual QUndoCommand* mousePress(GLWidget *widget, const QMouseEvent *event);
      virtual QUndoCommand* mouseRelease(GLWidget *widget, const QMouseEvent *event);
      virtual QUndoCommand* mouseMove(GLWidget *widget, const QMouseEvent *event);
      virtual QUndoCommand* wheel(GLWidget *widget, const QWheelEvent *event);

      virtual int usefulness() const;

    protected:
      void selectionBox(float sx, float sy, float ex, float ey);

      bool                _leftButtonPressed;  // rotation
      bool                _rightButtonPressed; // translation
      bool                _midButtonPressed;   // scale / zoom
      bool                _movedSinceButtonPressed;
      bool                _manipulateMode;     // shift

      //! Temporary var for adding selection box
      GLuint _selectionDL;

      QPoint              _initialDraggingPosition;
      QPoint              _lastDraggingPosition;

      QList<GLHit> _hits;

  };

  class SelectRotateToolFactory : public QObject, public ToolFactory
    {
      Q_OBJECT;
      Q_INTERFACES(Avogadro::ToolFactory);

      public:
        Tool *createInstance(QObject *parent = 0) { return new SelectRotateTool(parent); }
    };

  } // end namespace Avogadro

#endif
