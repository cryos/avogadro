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

#include "autorotatetool.h"
#include <avogadro/primitive.h>
#include <avogadro/glwidget.h>
#include <avogadro/camera.h>

#include <QtPlugin>
#include <QObject>

#include <QSlider>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>

using namespace std;
using namespace OpenBabel;
using namespace Avogadro;
using namespace Eigen;

AutoRotateTool::AutoRotateTool(QObject *parent) : Tool(parent), m_glwidget(0), timerId(0),
  m_xRotation(0), m_yRotation(0), m_zRotation(0), m_settingsWidget(0)
{
  QAction *action = activateAction();
  action->setIcon(QIcon(QString::fromUtf8(":/navigate/navigate.png")));
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
  // Get back transformed axes that we can rotate around
  Vector3d xAxis = m_glwidget->camera()->backtransformedXAxis();
  Vector3d yAxis = m_glwidget->camera()->backtransformedYAxis();
  Vector3d zAxis = m_glwidget->camera()->backtransformedZAxis();
  // Perform the rotations
  m_glwidget->camera()->translate( m_glwidget->center() );
  m_glwidget->camera()->rotate( m_xRotation * ROTATION_SPEED, yAxis );
  m_glwidget->camera()->rotate( m_yRotation * ROTATION_SPEED, xAxis );
  m_glwidget->camera()->rotate( m_zRotation * ROTATION_SPEED, zAxis );
  m_glwidget->camera()->translate( -m_glwidget->center() );
}

bool AutoRotateTool::paint(GLWidget *widget)
{
  // Get the widget for our rotation function
  m_glwidget = widget;
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

void AutoRotateTool::setTimer()
{
  // Toggle the timer on and off
  if (timerId)
  {
    killTimer(timerId);
    timerId = 0;
  }
  else
    timerId = startTimer(40);
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
    QLabel* labelX = new QLabel("x rotation:");
    labelX->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    labelX->setMaximumHeight(15);
    QSlider* sliderXRotation = new QSlider(m_settingsWidget);
    sliderXRotation->setOrientation(Qt::Horizontal);
    sliderXRotation->setTickPosition(QSlider::TicksBothSides);
    sliderXRotation->setToolTip("x rotation");
    sliderXRotation->setTickInterval(10);
    sliderXRotation->setPageStep(5);
    sliderXRotation->setRange(-20, 20);
    sliderXRotation->setValue(0);

    // Label and slider to set y axis rotation
    QLabel* labelY = new QLabel("y rotation:");
    labelY->setMaximumHeight(15);
    QSlider* sliderYRotation = new QSlider(m_settingsWidget);
    sliderYRotation->setOrientation(Qt::Horizontal);
    sliderYRotation->setTickPosition(QSlider::TicksBothSides);
    sliderYRotation->setToolTip("y rotation");
    sliderYRotation->setTickInterval(10);
    sliderYRotation->setPageStep(5);
    sliderYRotation->setRange(-20, 20);
    sliderYRotation->setValue(0);

    // Label and slider to set z axis rotation
    QLabel* labelZ = new QLabel("z rotation:");
    labelZ->setMaximumHeight(15);
    QSlider* sliderZRotation = new QSlider(m_settingsWidget);
    sliderZRotation->setOrientation(Qt::Horizontal);
    sliderZRotation->setTickPosition(QSlider::TicksBothSides);
    sliderZRotation->setToolTip("z rotation");
    sliderZRotation->setTickInterval(10);
    sliderZRotation->setPageStep(5);
    sliderZRotation->setRange(-20, 20);
    sliderZRotation->setValue(0);

    // Push buttons to start/stop and to reset
    QPushButton* buttonStartStop = new QPushButton("Start/Stop", m_settingsWidget);
    QPushButton* buttonReset = new QPushButton("Reset", m_settingsWidget);
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(buttonStartStop);
    buttonLayout->addWidget(buttonReset);

    QVBoxLayout* layout = new QVBoxLayout();
    layout->addWidget(labelX);
    layout->addWidget(sliderXRotation);
    layout->addWidget(labelY);
    layout->addWidget(sliderYRotation);
    layout->addWidget(labelZ);
    layout->addWidget(sliderZRotation);
    layout->addLayout(buttonLayout);
    layout->addStretch(1);
    m_settingsWidget->setLayout(layout);

    // Connect the sliders with the slots
    connect(sliderXRotation, SIGNAL(valueChanged(int)),
        this, SLOT(setXRotation(int)));

    connect(sliderYRotation, SIGNAL(valueChanged(int)),
        this, SLOT(setYRotation(int)));

    connect(sliderZRotation, SIGNAL(valueChanged(int)),
        this, SLOT(setZRotation(int)));

    // Connect the start/stop button
    connect(buttonStartStop, SIGNAL(clicked()),
        this, SLOT(setTimer()));

    // Connect the reset button to the reset slot
    connect(buttonReset, SIGNAL(clicked()),
        this, SLOT(resetRotations()));
    // Connect the reset signal to the sliders
    connect(this, SIGNAL(resetRotation(int)),
        sliderXRotation, SLOT(setValue(int)));
    connect(this, SIGNAL(resetRotation(int)),
        sliderYRotation, SLOT(setValue(int)));
    connect(this, SIGNAL(resetRotation(int)),
        sliderZRotation, SLOT(setValue(int)));

    connect(m_settingsWidget, SIGNAL(destroyed()),
        this, SLOT(settingsWidgetDestroyed()));
  }

  return m_settingsWidget;
}

void AutoRotateTool::settingsWidgetDestroyed()
{
  m_settingsWidget = 0;
}

#include "autorotatetool.moc"

Q_EXPORT_PLUGIN2(autorotatetool, AutoRotateToolFactory)
