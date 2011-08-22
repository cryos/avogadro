/**********************************************************************
  NavigateTool - Navigation Tool for Avogadro

  Copyright (C) 2007-2009 by Marcus D. Hanwell
  Copyright (C) 2006,2007 by Benoit Jacob

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.openmolecules.net/>

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

#ifndef NAVIGATETOOL_H
#define NAVIGATETOOL_H

#include <avogadro/glwidget.h>
#include <avogadro/tool.h>

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
  class NavigateSettingsWidget;
  class NavigateTool : public Tool
  {
    Q_OBJECT
      AVOGADRO_TOOL("Navigate", tr("Navigate"),
                    tr("Translate, rotate, and zoom around the current view"),
                    tr("Navigate Settings"))

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
     * @return the settings widget for the tool.
     */
    virtual QWidget* settingsWidget();

    /** \name Tool Methods
     * @{
     * @brief Callback methods for ui.actions on the canvas.
     */
    virtual QUndoCommand* mousePressEvent(GLWidget *widget, QMouseEvent *event);
    virtual QUndoCommand* mouseReleaseEvent(GLWidget *widget, QMouseEvent *event);
    virtual QUndoCommand* mouseMoveEvent(GLWidget *widget, QMouseEvent *event);
    virtual QUndoCommand* mouseDoubleClickEvent(GLWidget *widget, QMouseEvent *event);
    virtual QUndoCommand* wheelEvent(GLWidget *widget, QWheelEvent *event);
    virtual QUndoCommand* keyPressEvent(GLWidget *widget, QKeyEvent *event);
    virtual QUndoCommand* keyReleaseEvent(GLWidget *widget, QKeyEvent *event);
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

    /**
     * Write the tool settings so that they can be saved between sessions.
     */
    virtual void writeSettings(QSettings &settings) const;

    /**
     * Read in the settings that have been saved for the tool instance.
     */
    virtual void readSettings(QSettings &settings);

  private slots:
    /**
     * Change whether eye candy/visual cues are displayed.
     */
    void enableEyeCandy(int enable);

  protected:
    QPointer<Atom>      m_clickedAtom;
    Eigen::Vector3d     m_referencePoint; // the reference point for movement
                                          // i.e the center of the clicked atom,
                                          // or of the visible part of the molecule
    bool                m_leftButtonPressed;  // rotation
    bool                m_midButtonPressed;   // scale / zoom
    bool                m_rightButtonPressed; // translation
    bool                m_eyeCandyEnabled;    // Is eye candy enabled?
    bool                m_drawEyeCandy;       // Should eye candy be drawn?
    double m_yAngleEyecandy, m_xAngleEyecandy;

    QPoint              m_lastDraggingPosition;
    bool                m_draggingInitialized;  // Has dragging been initialized?

    Eyecandy * m_eyecandy;
    NavigateSettingsWidget *m_settingsWidget;

    /** recomputes m_referencePoint. Uses the value of m_clickedAtom. */
    void computeReferencePoint(GLWidget *widget);
  };

  /**
   * @class NavigateToolFactory navigatetool.h
   * @brief Factory class to create instances of the NavigateTool class.
   */
  class NavigateToolFactory : public QObject, public PluginFactory
  {
    Q_OBJECT
    Q_INTERFACES(Avogadro::PluginFactory)
    AVOGADRO_TOOL_FACTORY(NavigateTool)
  };

} // end namespace Avogadro

#endif
