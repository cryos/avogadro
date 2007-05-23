/**********************************************************************
  ManipulateTool - Manipulation Tool for Avogadro

  Copyright (C) 2007 by Marcus D. Hanwell
  Copyright (C) 2007 by Geoffrey R. Hutchison
  Copyright (C) 2007 by Benoit Jacob

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

#ifndef __MANIPULATETOOL_H
#define __MANIPULATETOOL_H

#include <avogadro/glwidget.h>
#include <avogadro/tool.h>

#include <openbabel/mol.h>

#include <QGLWidget>
#include <QObject>
#include <QStringList>
#include <QImage>
#include <QAction>

namespace Avogadro {

  /**
   * @class ManipulateTool
   * @brief Manipulate the position of atoms
   * @author Marcus D. Hanwell
   *
   * This tool enables the manipulation of the position of
   * the selected atoms.
   */
  class ManipulateTool : public Tool
  {
    Q_OBJECT

    public:
      //! Constructor
      ManipulateTool(QObject *parent = 0);
      //! Deconstructor
      virtual ~ManipulateTool();

      //! \name Description methods
      //@{
      //! Tool Name (ie Draw)
      virtual QString name() const { return(tr("Manipulate")); }
      //! Tool Description (ie. Draws atoms and bonds)
      virtual QString description() const { return(tr("Manipulation Tool")); }
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

      virtual bool paint(GLWidget *widget);

    protected:
      GLWidget *          m_glwidget;
      Atom *              m_clickedAtom;
      bool                m_leftButtonPressed;  // rotation
      bool                m_midButtonPressed;   // scale / zoom
      bool                m_rightButtonPressed; // translation
      Eigen::Vector3d     m_selectionCenter;    // centroid of selected atoms

      //! Temporary var for adding selection box
      GLuint              m_selectionDL;

      QPoint              m_lastDraggingPosition;

      void drawSphere(GLWidget *widget,  const Eigen::Vector3d &center, double radius, float alpha);

      void computeClickedAtom(const QPoint& p);
      void zoom( const Eigen::Vector3d &goal, double delta ) const;
      void translate( const Eigen::Vector3d &what, const QPoint &from, const QPoint &to ) const;
      void rotate( const Eigen::Vector3d &center, double deltaX, double deltaY ) const;
      void tilt( const Eigen::Vector3d &center, double delta ) const;
  };

  class ManipulateToolFactory : public QObject, public ToolFactory
    {
      Q_OBJECT
      Q_INTERFACES(Avogadro::ToolFactory)

      public:
        Tool *createInstance(QObject *parent = 0) { return new ManipulateTool(); }
    };

  // Defined in moveatomcommand.h
  //  class MoveAtomCommand;

} // end namespace Avogadro

#endif
