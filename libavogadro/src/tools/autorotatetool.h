/**********************************************************************
  AutoRotateTool - Auto Rotation Tool for Avogadro

  Copyright (C) 2007 by Marcus D. Hanwell

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

#ifndef __AUTOROTATETOOL_H
#define __AUTOROTATETOOL_H

#include <avogadro/glwidget.h>
#include <avogadro/tool.h>

#include <QGLWidget>
#include <QObject>
#include <QStringList>
#include <QImage>
#include <QAction>
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

    public:
      //! Constructor
      AutoRotateTool(QObject *parent = 0);
      //! Destructor
      virtual ~AutoRotateTool();

      //! \name Description methods
      //@{
      //! Tool Name
      virtual QString name() const { return(tr("AutoRotate")); }
      //! Tool Description
      virtual QString description() const { return(tr("Auto Rotation Tool")); }
      //@}

      //! \name Tool Methods
      //@{
      //! \brief Callback methods for ui.actions on the canvas.
      /*!
      */
      virtual QUndoCommand* mousePress(GLWidget*, const QMouseEvent*);
      virtual QUndoCommand* mouseRelease(GLWidget*, const QMouseEvent*);
      virtual QUndoCommand* mouseMove(GLWidget*, const QMouseEvent*);
      virtual QUndoCommand* wheel(GLWidget*, const QWheelEvent*) { return 0; }

      virtual int usefulness() const;

      virtual bool paint(GLWidget *widget);

      virtual QWidget* settingsWidget();

    public Q_SLOTS:
      void setXRotation(int i);
      void setYRotation(int i);
      void setZRotation(int i);
      void setTimer();
      void resetRotations();

    Q_SIGNALS:
      void resetRotation(int i);

    protected:
      GLWidget* m_glwidget;
      bool m_leftButtonPressed;  // Rotation about x and y
      bool m_midButtonPressed;   // Rotation about z
      QPoint m_startDraggingPosition;  // Starting position of mouse
      QPoint m_currentDraggingPosition;  // Current dragging position
      int timerId;
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

  class AutoRotateToolFactory : public QObject, public ToolFactory
    {
      Q_OBJECT
      Q_INTERFACES(Avogadro::ToolFactory)

      public:
        Tool* createInstance(QObject *parent = 0) { return new AutoRotateTool(parent); }
    };

} // end namespace Avogadro

#endif
