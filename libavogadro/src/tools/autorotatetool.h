/**********************************************************************
  AutoRotateTool - Auto Rotation Tool for Avogadro

  Copyright (C) 2007,2008 by Marcus D. Hanwell

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

#ifndef AUTOROTATETOOL_H
#define AUTOROTATETOOL_H

#include <avogadro/glwidget.h>
#include <avogadro/tool.h>

#include <QGLWidget>
#include <QObject>

#include <QWidget>

class QPushButton;
class QSlider;

namespace Avogadro {

  /**
   * @class AutoRotateTool
   * @brief Automatic molecule rotation
   * @author Marcus D. Hanwell
   *
   * This class handles the automatic rotation of the view
   * so that the molecule can be shown in more detail.
   */

  class AutoRotateTool : public Tool
  {
    Q_OBJECT
      AVOGADRO_TOOL("AutoRotate", tr("AutoRotate"),
                    tr("Automatic rotation of molecules"),
                    tr("AutoRotate Settings"))

    public:
      //! Constructor
      AutoRotateTool(QObject *parent = 0);
      //! Destructor
      virtual ~AutoRotateTool();

      //! \name Tool Methods
      //@{
      //! \brief Callback methods for ui.actions on the canvas.
      /*!
      */
      virtual QUndoCommand* mousePressEvent(GLWidget*, QMouseEvent*);
      virtual QUndoCommand* mouseReleaseEvent(GLWidget*, QMouseEvent*);
      virtual QUndoCommand* mouseMoveEvent(GLWidget*, QMouseEvent*);
      virtual QUndoCommand* wheelEvent(GLWidget*, QWheelEvent*) { return 0; }

      virtual int usefulness() const;

      virtual bool paint(GLWidget *widget);

      virtual QWidget* settingsWidget();

    public Q_SLOTS:
      void setXRotation(int i);
      void setYRotation(int i);
      void setZRotation(int i);
      void toggleTimer();
      void resetRotations();
      void enableTimer();
      void disableTimer();

    Q_SIGNALS:
      void resetRotation(int i);

    protected:
      GLWidget* m_glwidget;
      bool m_leftButtonPressed;  // Rotation about x and y
      bool m_midButtonPressed;   // Rotation about z
      QPoint m_startDraggingPosition;  // Starting position of mouse
      QPoint m_currentDraggingPosition;  // Current dragging position
      int m_timerId;
      int m_xRotation;
      int m_yRotation;
      int m_zRotation;
      int m_maxRotation;

      void rotate() const;
      void timerEvent(QTimerEvent* event);

      QWidget* m_settingsWidget;
      QPushButton* m_buttonStartStop;
      QSlider* m_sliderX;
      QSlider* m_sliderY;
      QSlider* m_sliderZ;

    private Q_SLOTS:
      void settingsWidgetDestroyed();
  };

  class AutoRotateToolFactory : public QObject, public PluginFactory
  {
    Q_OBJECT
    Q_INTERFACES(Avogadro::PluginFactory)
    AVOGADRO_TOOL_FACTORY(AutoRotateTool)
  };

} // end namespace Avogadro

#endif
