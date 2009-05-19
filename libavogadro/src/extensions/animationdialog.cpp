/**********************************************************************
  AnimationDialog - Dialog for animation extension

  Copyright (C) 2008 by Tim Vandermeersch

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

#include "animationdialog.h"

#include <QPushButton>
#include <QButtonGroup>
#include <QDebug>

#include <QFileDialog>
#include <QFile>
#include <QMessageBox>
#include <QProgressDialog>


#include <openbabel/plugin.h>

using namespace OpenBabel;

namespace Avogadro {

  AnimationDialog::AnimationDialog( QWidget *parent, Qt::WindowFlags f ) : QDialog( parent, f )
  {
    //  qDebug() << "AnimationDialog::AnimationDialog()" << endl;
    ui.setupUi(this);
    connect(ui.loadButton, SIGNAL(clicked()), this, SLOT(loadFile()));
    connect(ui.frameSlider, SIGNAL(valueChanged(int)), this, SIGNAL(sliderChanged(int)));
    connect(ui.fpsSpin, SIGNAL(valueChanged(int)), this, SIGNAL(fpsChanged(int)));
    connect(ui.loopBox, SIGNAL(stateChanged(int)), this, SIGNAL(loopChanged(int)));
    connect(ui.dynBondsBox, SIGNAL(stateChanged(int)), this, SIGNAL(dynamicBondsChanged(int)));
    
    connect(ui.playButton, SIGNAL(clicked()), this, SIGNAL(play()));
    connect(ui.pauseButton, SIGNAL(clicked()), this, SIGNAL(pause()));
    connect(ui.stopButton, SIGNAL(clicked()), this, SIGNAL(stop()));
    connect(ui.saveVideoButton, SIGNAL(clicked()), this, SLOT(saveVideo()));
      }

  AnimationDialog::~AnimationDialog()
  {
    //  qDebug() << "AnimationDialog::~AnimationDialog()" << endl;
  }

  void AnimationDialog::loadFile()
  {
    // Load a file
    QString file = QFileDialog::getOpenFileName(this,
      tr("Open trajectory file"), ui.fileEdit->text(),
      tr("Trajectory files (*.xtc *.xyz)"));
    ui.fileEdit->setText(file);
    
    emit fileName(file);
  }

  void AnimationDialog::setFrame(int i)
  {
    QString str = tr("%1/%2").arg( i ).arg( m_frameCount );

    ui.frameEdit->setText(str);
    ui.frameSlider->setValue(i);
  }

  void AnimationDialog::setFrameCount(int i)
  {
    m_frameCount = i;
    ui.frameSlider->setMaximum(i);
  }

  int AnimationDialog::fps()
  {
    return ui.fpsSpin->value();
  }


  void AnimationDialog::saveVideo()
  {
    QString sVideoFileName = QFileDialog::getSaveFileName(this, 
							  tr("Save Video File"),
							  ui.videoFileLine->text(),
							  tr("video files (*.avi)"));

    
    if (!sVideoFileName.isEmpty() )  { 
      if (!sVideoFileName.endsWith(QLatin1String(".avi"))){
        QMessageBox::warning( NULL, tr( "Avogadro" ),
                              tr( "Adding .avi extension" ));
        sVideoFileName = sVideoFileName + ".avi";
      }
      ui.videoFileLine->setText(sVideoFileName);
    }
    
    emit videoFileInfo(sVideoFileName);

  }

}

