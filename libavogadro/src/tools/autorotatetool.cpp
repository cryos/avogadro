/**********************************************************************
  AutoRotateTool - Auto Rotation Tool for Avogadro

  Copyright (C) 2007,2008 by Marcus D. Hanwell

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

#include "autorotatetool.h"
#include <avogadro/primitive.h>
#include <avogadro/glwidget.h>
#include <avogadro/camera.h>

#include <QtPlugin>
#include <QObject>
#include <QAction>

#include <QSlider>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>

using namespace std;
using namespace Eigen;

namespace Avogadro {

  AutoRotateTool::AutoRotateTool(QObject *parent) : Tool(parent), m_glwidget(0), m_leftButtonPressed(false),
  m_midButtonPressed(false), m_timerId(0), m_xRotation(0), m_yRotation(0), m_zRotation(0), m_maxRotation(40),
  m_settingsWidget(0), m_buttonStartStop(0), m_sliderX(0), m_sliderY(0), m_sliderZ(0)

  {
    QAction *action = activateAction();
    action->setIcon(QIcon(QString::fromUtf8(":/autorotate/autorotate.png")));
    action->setToolTip(tr("Auto Rotation Tool"));
  }

  AutoRotateTool::~AutoRotateTool()
  {
  }

  int AutoRotateTool::usefulness() const
  {
    return 20000;
  }

  void AutoRotateTool::rotate() const
  {
    if (!m_glwidget->molecule()) {
      return; // done!
    }

    // Get back transformed axes that we can rotate around
    Vector3d xAxis = m_glwidget->camera()->backTransformedXAxis();
    Vector3d yAxis = m_glwidget->camera()->backTransformedYAxis();
    Vector3d zAxis = m_glwidget->camera()->backTransformedZAxis();
    // Perform the rotations
    m_glwidget->camera()->translate( m_glwidget->center() );
    m_glwidget->camera()->rotate( m_xRotation * ROTATION_SPEED, yAxis );
    m_glwidget->camera()->rotate( m_yRotation * ROTATION_SPEED, xAxis );
    m_glwidget->camera()->rotate( m_zRotation * ROTATION_SPEED, zAxis );
    m_glwidget->camera()->translate( -m_glwidget->center() );
  }

  QUndoCommand* AutoRotateTool::mousePressEvent(GLWidget *widget, QMouseEvent *event)
  {
    // Record the starting postion and which mouse button was pressed
    m_glwidget = widget;
    m_startDraggingPosition = event->pos();
    m_currentDraggingPosition = m_startDraggingPosition;
    m_leftButtonPressed = ( event->buttons() & Qt::LeftButton );
    m_midButtonPressed = ( event->buttons() & Qt::MidButton );

    if (m_leftButtonPressed || m_midButtonPressed)
      event->accept();

    m_glwidget->update();

    return 0;
  }

  QUndoCommand* AutoRotateTool::mouseReleaseEvent(GLWidget *widget, QMouseEvent *event)
  {
    m_glwidget = widget;
    // Calculate some multipliers for the delta
    double xMultiplier = m_maxRotation / static_cast<double>(m_glwidget->width());
    double yMultiplier = m_maxRotation / static_cast<double>(m_glwidget->height());
    QPoint deltaDragging = event->pos() - m_startDraggingPosition;

    if(m_leftButtonPressed) {
      event->accept();
      // Rotation about the x and y axes
      m_xRotation = static_cast<int>(deltaDragging.x() * xMultiplier);
      m_sliderX->setValue(m_xRotation);
      m_yRotation = static_cast<int>(deltaDragging.y() * yMultiplier);
      m_sliderY->setValue(m_yRotation);
      m_zRotation = 0;
      m_sliderZ->setValue(m_zRotation);

      enableTimer();
    }
    else if (m_midButtonPressed) {
      event->accept();
      // Rotation about the z axis
      m_xRotation = 0;
      m_sliderX->setValue(m_xRotation);
      m_yRotation = 0;
      m_sliderY->setValue(m_yRotation);
      m_zRotation = static_cast<int>(deltaDragging.x() * xMultiplier);
      m_sliderZ->setValue(m_zRotation);

      enableTimer();
    }

    m_leftButtonPressed = false;
    m_midButtonPressed = false;

    m_glwidget->update();

    return 0;
  }

  QUndoCommand* AutoRotateTool::mouseMoveEvent(GLWidget *widget, QMouseEvent *event)
  {
    m_glwidget = widget;

    // Keep track of the current position to draw the movement line
    m_currentDraggingPosition = event->pos();

    if (m_leftButtonPressed || m_midButtonPressed)
      event->accept();

    m_glwidget->update();

    return 0;
  }


  bool AutoRotateTool::paint(GLWidget *widget)
  {
    m_glwidget = widget;
    if(m_leftButtonPressed || m_midButtonPressed)
    {
      // Draw a line from the start position to the current mouse position
      if (m_leftButtonPressed) {
        glColor4f(1., 0., 0., 1.);
      } else if (m_midButtonPressed) {
        glColor4f(0., 1., 0., 1.);
      }
      Vector3d start = m_glwidget->camera()->unProject(m_startDraggingPosition);
      Vector3d end = m_glwidget->camera()->unProject(m_currentDraggingPosition);
      glDisable(GL_LIGHTING);
      glBegin(GL_LINES);
      glVertex3d(start.x(), start.y(), start.z());
      glVertex3d(end.x(), end.y(), end.z());
      glEnd();
      glEnable(GL_LIGHTING);
    }
    return true;
  }

  void AutoRotateTool::timerEvent(QTimerEvent*)
  {
    // If there is a glwidget and at least one axis is set for rotation
    // then perform rotation and trigger a window update
    if (m_glwidget && (m_xRotation || m_yRotation || m_zRotation))
    {
      rotate();
      m_glwidget->update();
    }
  }

  void AutoRotateTool::setXRotation(int i)
  {
    m_xRotation = i;
  }

  void AutoRotateTool::setYRotation(int i)
  {
    m_yRotation = i;
  }

  void AutoRotateTool::setZRotation(int i)
  {
    m_zRotation = i;
  }

  void AutoRotateTool::toggleTimer()
  {
    // Toggle the timer on and off
    if (m_timerId) {
      disableTimer();
    } else {
      enableTimer();
    }
  }

  void AutoRotateTool::enableTimer()
  {
    if(!m_timerId)
    {
      m_timerId = startTimer(40);
      m_buttonStartStop->setText(tr("Stop"));
    }
  }

  void AutoRotateTool::disableTimer()
  {
    if(m_timerId)
    {
      killTimer(m_timerId);
      m_timerId = 0;
      m_buttonStartStop->setText(tr("Start"));
    }
  }

  void AutoRotateTool::resetRotations()
  {
    // Emit the reset signal with a value of 0
    emit resetRotation(0);
  }

  QWidget* AutoRotateTool::settingsWidget() {
    if(!m_settingsWidget) {
      m_settingsWidget = new QWidget;

      // Label and slider to set x axis rotation
      QLabel* labelX = new QLabel(tr("x rotation:"));
      labelX->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
      labelX->setMaximumHeight(15);
      m_sliderX = new QSlider(m_settingsWidget);
      m_sliderX->setOrientation(Qt::Horizontal);
      m_sliderX->setTickPosition(QSlider::TicksAbove);
      m_sliderX->setToolTip(tr("x rotation"));
      m_sliderX->setTickInterval(10);
      m_sliderX->setPageStep(5);
      m_sliderX->setRange(-m_maxRotation, m_maxRotation);
      m_sliderX->setValue(0);
      QHBoxLayout* xLayout = new QHBoxLayout;
      xLayout->addWidget(labelX);
      xLayout->addWidget(m_sliderX);

      // Label and slider to set y axis rotation
      QLabel* labelY = new QLabel(tr("y rotation:"));
      labelY->setMaximumHeight(15);
      m_sliderY = new QSlider(m_settingsWidget);
      m_sliderY->setOrientation(Qt::Horizontal);
      m_sliderY->setTickPosition(QSlider::TicksAbove);
      m_sliderY->setToolTip(tr("y rotation"));
      m_sliderY->setTickInterval(10);
      m_sliderY->setPageStep(5);
      m_sliderY->setRange(-m_maxRotation, m_maxRotation);
      m_sliderY->setValue(0);
      QHBoxLayout* yLayout = new QHBoxLayout;
      yLayout->addWidget(labelY);
      yLayout->addWidget(m_sliderY);

      // Label and slider to set z axis rotation
      QLabel* labelZ = new QLabel(tr("z rotation:"));
      labelZ->setMaximumHeight(15);
      m_sliderZ = new QSlider(m_settingsWidget);
      m_sliderZ->setOrientation(Qt::Horizontal);
      m_sliderZ->setTickPosition(QSlider::TicksAbove);
      m_sliderZ->setToolTip(tr("z rotation"));
      m_sliderZ->setTickInterval(10);
      m_sliderZ->setPageStep(5);
      m_sliderZ->setRange(-m_maxRotation, m_maxRotation);
      m_sliderZ->setValue(0);
      QHBoxLayout* zLayout = new QHBoxLayout;
      zLayout->addWidget(labelZ);
      zLayout->addWidget(m_sliderZ);

      // Push buttons to start/stop and to reset
      m_buttonStartStop = new QPushButton(tr("Start"), m_settingsWidget);
      QPushButton* buttonReset = new QPushButton(tr("Reset"), m_settingsWidget);
      QHBoxLayout* buttonLayout = new QHBoxLayout();
      buttonLayout->addWidget(m_buttonStartStop);
      buttonLayout->addWidget(buttonReset);

      QVBoxLayout* layout = new QVBoxLayout();
      layout->addLayout(xLayout);
      layout->addLayout(yLayout);
      layout->addLayout(zLayout);
      layout->addLayout(buttonLayout);
      layout->addStretch(1);
      m_settingsWidget->setLayout(layout);

      // Connect the sliders with the slots
      connect(m_sliderX, SIGNAL(valueChanged(int)),
          this, SLOT(setXRotation(int)));

      connect(m_sliderY, SIGNAL(valueChanged(int)),
          this, SLOT(setYRotation(int)));

      connect(m_sliderZ, SIGNAL(valueChanged(int)),
          this, SLOT(setZRotation(int)));

      // Connect the start/stop button
      connect(m_buttonStartStop, SIGNAL(clicked()),
          this, SLOT(toggleTimer()));

      // Connect the reset button to the reset slot
      connect(buttonReset, SIGNAL(clicked()),
          this, SLOT(resetRotations()));
      // Connect the reset signal to the sliders
      connect(this, SIGNAL(resetRotation(int)),
          m_sliderX, SLOT(setValue(int)));
      connect(this, SIGNAL(resetRotation(int)),
          m_sliderY, SLOT(setValue(int)));
      connect(this, SIGNAL(resetRotation(int)),
          m_sliderZ, SLOT(setValue(int)));

      connect(m_settingsWidget, SIGNAL(destroyed()),
          this, SLOT(settingsWidgetDestroyed()));
    }

    return m_settingsWidget;
  }

  void AutoRotateTool::settingsWidgetDestroyed()
  {
    m_settingsWidget = 0;
    m_buttonStartStop = 0;
  }
}

#include "autorotatetool.moc"

Q_EXPORT_PLUGIN2(autorotatetool, Avogadro::AutoRotateToolFactory)
