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
      Eigen::Vector3d     m_selectedPrimitivesCenter;    // centroid of selected atoms

      QUndoCommand *      m_undo; // The current undo command

      QPoint              m_lastDraggingPosition;

      void computeClickedAtom(const QPoint& p);
      void zoom( const Eigen::Vector3d &goal, double delta ) const;
      void translate( const Eigen::Vector3d &what, const QPoint &from, const QPoint &to ) const;
      void rotate( const Eigen::Vector3d &center, double deltaX, double deltaY ) const;
      void tilt( const Eigen::Vector3d &center, double delta ) const;
  };

 class MoveAtomCommand : public QUndoCommand
  {
    public:
      explicit MoveAtomCommand(Molecule *molecule, QUndoCommand *parent = 0);
      MoveAtomCommand(Molecule *molecule, Atom *atom, Eigen::Vector3d pos, QUndoCommand *parent = 0);

      void redo();
      void undo();
      bool mergeWith ( const QUndoCommand * command );
      int id() const;

    private:
      Molecule m_moleculeCopy;
      Molecule *m_molecule;
      int m_atomIndex;
      Eigen::Vector3d m_pos;
      bool undone;
  };

  class ManipulateToolFactory : public QObject, public ToolFactory
    {
      Q_OBJECT
      Q_INTERFACES(Avogadro::ToolFactory)

      public:
        Tool *createInstance(QObject *parent = 0) { return new ManipulateTool(parent); }
    };

  // Defined in moveatomcommand.h
  //  class MoveAtomCommand;

} // end namespace Avogadro

#endif
