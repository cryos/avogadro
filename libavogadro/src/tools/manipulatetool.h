/**********************************************************************
  ManipulateTool - Manipulation Tool for Avogadro

  Copyright (C) 2007 by Marcus D. Hanwell
  Copyright (C) 2007 by Geoffrey R. Hutchison
  Copyright (C) 2007 by Benoit Jacob

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.openmolecules.net/>

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

#ifndef MANIPULATETOOL_H
#define MANIPULATETOOL_H

#include <avogadro/glwidget.h>
#include <avogadro/tool.h>

#include <avogadro/molecule.h>

#include <QGLWidget>
#include <QObject>
#include <QStringList>
#include <QImage>
#include <QAction>
#include <QUndoCommand>

namespace Avogadro {

  /**
   * @class ManipulateTool
   * @brief Manipulate the position of atoms
   * @author Marcus D. Hanwell
   *
   * This tool enables the manipulation of the position of
   * the selected atoms.
   */
  class Eyecandy;
  class ManipulateTool : public Tool
  {
    Q_OBJECT
      AVOGADRO_TOOL("Manipulate", tr("Manipulate"),
                    tr("Translate, rotate, and adjust atoms and fragments"),
                    tr("Manipulate Settings"))

    public:
      //! Constructor
      ManipulateTool(QObject *parent = 0);
      //! Deconstructor
      virtual ~ManipulateTool();

      //! \name Tool Methods
      //@{
      //! \brief Callback methods for ui.actions on the canvas.
      /*!
      */
      virtual QUndoCommand* mousePressEvent(GLWidget *widget, QMouseEvent *event);
      virtual QUndoCommand* mouseReleaseEvent(GLWidget *widget, QMouseEvent *event);
      virtual QUndoCommand* mouseMoveEvent(GLWidget *widget, QMouseEvent *event);
      virtual QUndoCommand* mouseDoubleClickEvent(GLWidget *, QMouseEvent *) { return 0; }
      virtual QUndoCommand* wheelEvent(GLWidget *widget, QWheelEvent *event);

      virtual int usefulness() const;

      virtual bool paint(GLWidget *widget);

    protected:
      Atom *              m_clickedAtom;
      bool                m_leftButtonPressed;  // rotation
      bool                m_midButtonPressed;   // scale / zoom
      bool                m_rightButtonPressed; // translation
      Eigen::Vector3d     m_selectedPrimitivesCenter;    // centroid of selected atoms

      QPoint              m_lastDraggingPosition;
      Eyecandy            *m_eyecandy;
      double              m_yAngleEyecandy, m_xAngleEyecandy;

      void zoom(GLWidget *widget, const Eigen::Vector3d *goal,
                double delta) const;
      void translate(GLWidget *widget, const Eigen::Vector3d *what, const QPoint
                     &from, const QPoint &to) const;
      void rotate(GLWidget *widget, const Eigen::Vector3d *center, double deltaX,
                  double deltaY) const;
      void tilt(GLWidget *widget, const Eigen::Vector3d *center,
                double delta) const;
  };

  class ManipulateToolFactory : public QObject, public PluginFactory
  {
    Q_OBJECT
    Q_INTERFACES(Avogadro::PluginFactory)
    AVOGADRO_TOOL_FACTORY(ManipulateTool)
  };

} // end namespace Avogadro

#endif
