/**********************************************************************
  NavigateTool - Navigation Tool for Avogadro

  Copyright (C) 2007 by Marcus D. Hanwell
  Copyright (C) 2006,2007 by Benoit Jacob

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

#ifndef __NAVIGATETOOL_H
#define __NAVIGATETOOL_H

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
   * @class NavigateTool
   * @brief Navigation tool for moving the camera around the molecule.
   * @author Marcus D. Hanwell
   *
   * This class implements navigation of the camera around the molecule being
   * displayed. It attempts to do this in the most intuitive way possible
   * offering both atom centric (if an atom has been clicked on) and scene
   * centric (if no atom has been clicked on) navigation.
   */
  class Eyecandy;
  class NavigateTool : public Tool
  {
    Q_OBJECT

  public:
    /**
     * Constructor.
     */
    NavigateTool(QObject *parent = 0);
    /**
     * Destructor.
     */
    virtual ~NavigateTool();

    /**
     * @name Description methods
     * @{
     * Tool Name (i.e. Navigate)
     */
     virtual QString name() const { return(tr("Navigate")); }

    /**
     *Tool Description (i.e. Navigation Tool)
     */
    virtual QString description() const { return(tr("Navigation Tool")); }
    //@}

    /** \name Tool Methods
     * @{
     * @brief Callback methods for ui.actions on the canvas.
     */
    virtual QUndoCommand* mousePress(GLWidget *widget, const QMouseEvent *event);
    virtual QUndoCommand* mouseRelease(GLWidget *widget, const QMouseEvent *event);
    virtual QUndoCommand* mouseMove(GLWidget *widget, const QMouseEvent *event);
    virtual QUndoCommand* wheel(GLWidget *widget, const QWheelEvent *event);
    //@}

    /**
     * @return the relative usefulness of the tool - affects the order in which
     * the tools are displayed.
     */
    virtual int usefulness() const;

    /**
     * The paint method of the tool which is used to paint any tool specific
     * visuals to the GLWidget.
     */
    virtual bool paint(GLWidget *widget);

  protected:
    Atom *              m_clickedAtom;
    Eigen::Vector3d     m_referencePoint; // the reference point for movement
                                          // i.e the center of the clicked atom,
                                          // or of the visible part of the molecule
    bool                m_leftButtonPressed;  // rotation
    bool                m_midButtonPressed;   // scale / zoom
    bool                m_rightButtonPressed; // translation
    double m_yAngleEyecandy, m_xAngleEyecandy;

    QPoint              m_lastDraggingPosition;

    Eyecandy * m_eyecandy;
  };

  /**
   * @class NavigateToolFactory navigatetool.h
   * @brief Factory class to create instances of the NavigateTool class.
   */
  class NavigateToolFactory : public QObject, public ToolFactory
  {
    Q_OBJECT
    Q_INTERFACES(Avogadro::ToolFactory)

  public:
    /**
     * Creates an instance of the NavigateTool class.
     */
    Tool *createInstance(QObject *parent = 0) { return new NavigateTool(parent); }
  };

} // end namespace Avogadro

#endif
