/**********************************************************************
  NavigateTool - Navigation Tool for Avogadro

  Copyright (C) 2007 by Marcus D. Hanwel
  Copyright (C) 2006,2007 by Benoit Jacob

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

#ifndef __NAVIGATETOOL_H
#define __NAVIGATETOOL_H

#include <avogadro/glwidget.h>
#include <avogadro/tool.h>

#include <openbabel/mol.h>

#include <QGLWidget>
#include <QObject>
#include <QStringList>
#include <QImage>

//const double ROTATION_SPEED = 0.005;
//const double TRANSLATION_SPEED = 0.02;

namespace Avogadro {

  /**
   * @class NavigateTool
   * @brief Port of Navigation from Kalzium
   * @author Marcus D. Hanwel
   *
   * This class is an attempt to port the navigation system in
   * Kalzium to an Avogadro plugin.
   */
  class NavigateTool : public Tool
  {
    Q_OBJECT

    public:
      //! Constructor
      NavigateTool(QObject *parent = 0);
      //! Deconstructor
      virtual ~NavigateTool();

      //! \name Description methods
      //@{
      //! Tool Name (ie Draw)
      virtual QString name() const { return(tr("Navigate")); }
      //! Tool Description (ie. Draws atoms and bonds)
      virtual QString description() const { return(tr("Navigation Tool")); }
      //@}

      //! \name Tool Methods
      //@{
      //! \brief Callback methods for ui.actions on the canvas.
      /*!
      */
      virtual void mousePress(GLWidget *widget, const QMouseEvent *event);
      virtual void mouseRelease(GLWidget *widget, const QMouseEvent *event);
      virtual void mouseMove(GLWidget *widget, const QMouseEvent *event);
      virtual void wheel(GLWidget *widget, const QWheelEvent *event);

    protected:

      bool                _leftButtonPressed;  // rotation
      bool                _rightButtonPressed; // translation
      bool                _midButtonPressed;   // scale / zoom
      bool                _clickedAtom;
      Eigen::Vector3d     _clickedAtomPos;
      double              _clickedAtomRadius;

      //! Temporary var for adding selection box
      GLuint _selectionDL;

      QPoint              _lastDraggingPosition;

      QList<GLHit> _hits;

      const double ROTATION_SPEED;
      const double TRANSLATION_SPEED;
  };

  class NavigateToolFactory : public QObject, public ToolFactory
    {
      Q_OBJECT;
      Q_INTERFACES(Avogadro::ToolFactory);

      public:
        Tool *createInstance(QObject *parent = 0) { return new NavigateTool(); }
    };

} // end namespace Avogadro

#endif
