/**********************************************************************
  TrajVideoMaker - used to generate a video of a trajectory

  Copyright (C) 2008 by Naomi Fox

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

#include "trajvideomaker.h"
#include "povpainter.h"
#include <avogadro/molecule.h>
#include <avogadro/animation.h>

#include <QMessageBox>
#include <QInputDialog>
#include <QProgressDialog>
#include <QFile>

#include <fstream>
#include <vector>

namespace Avogadro {

  TrajVideoMaker::TrajVideoMaker(){}

  TrajVideoMaker::~TrajVideoMaker(){}

  void TrajVideoMaker::makeVideo(GLWidget *widget, Animation *animation,
                                 const QString& workingDirectory,
                                 const QString& videoFileName)
  {
    QString workDirectory = workingDirectory;
    if (!workDirectory.endsWith('/'))
      workDirectory += '/';

    // check widget is okay
    if (!widget) {
      QMessageBox::warning( NULL, QObject::tr( "Avogadro" ),
                            QObject::tr( "GL widget was not correctly initialized in order to make a video" ));
      return;
    }

    Molecule* molecule = widget->molecule();
    // check molecule is okay
    if (!molecule) {
      QMessageBox::warning( NULL, QObject::tr( "Avogadro" ),
                            QObject::tr( "GL widget has no molecule" ));
      return;
    }

    double aspectRatio = getAspectRatio(widget);
    //start the progress dialog
    QProgressDialog progDialog(QObject::tr("Building video "),
                               QObject::tr("Cancel"), 0,
                               molecule->numConformers()*2);
    progDialog.setMinimumDuration(1);
    progDialog.setValue(0);

    //list of pngfiles for mencoder
    std::vector<QString> pngFiles;

    for (int i = 0; i < animation->numFrames(); ++i) {
      QString povFileName = workDirectory + QString::number(i) + ".pov";
      animation->setFrame(i);

      // write the pov file
      // must be in own scope so object is destroyed and file is closed after
      // (a design flaw in POVPainterDevice?)
      POVPainterDevice pd( povFileName, aspectRatio, widget );

      progDialog.setValue(2*i);

      //make the png
      runPovRay(workDirectory, povFileName);
      progDialog.setValue(2*i+1);

      QString pngFileName = workDirectory + QString::number(i) + ".png";
      pngFiles.push_back(pngFileName);

      if (progDialog.wasCanceled()) {
        //stop making
        return;
      }
    }

    //now run mencoder
    runMencoder(workDirectory, videoFileName, pngFiles.begin(), pngFiles.end());

    progDialog.setValue(progDialog.maximum());

    //tell user if successful
    std::ifstream fin(QFile::encodeName(videoFileName));
    if(!fin.fail()) {
    fin.close();
    QString successMessage = "Video file " + videoFileName + " written.";
    QMessageBox::information( NULL, QObject::tr( "Avogadro" ),
            successMessage);
    }
    else {
      QString failedMessage = QObject::tr("Video file not written.");
      QMessageBox::warning( NULL, QObject::tr( "Avogadro" ), failedMessage);
    }
  }

  void TrajVideoMaker::runPovRay(QString directory, QString povFileName)
  {
    //executable for povray.  -D suppresses the popup image.
    const QString povrayexe = "povray -D ";

    QString povRayCommand = "cd " +  directory +
      " && " + povrayexe + ' ' + povFileName;
    //QMessageBox::warning( NULL, QObject::tr( "Avogadro" ), povRayCommand);
    int ret = system(povRayCommand.toStdString().c_str());
    if (ret)
      QMessageBox::warning( NULL, QObject::tr( "Avogadro" ), QObject::tr("Could not run povray."));
  }

  template <class QStringIterator>
  void TrajVideoMaker::runMencoder(QString pngFileDirectory, QString videoFileName,
                                   QStringIterator startPngFiles, QStringIterator endPngFiles)
  {
    //executable for mencoder
    const QString mencoderexe = "mencoder -ovc lavc -lavcopts vcodec=mpeg4 -of avi -o ";

    QString pngString;
    for (QStringIterator pngIterator = startPngFiles;
      pngIterator != endPngFiles; ++pngIterator) {
      pngString += *pngIterator + ',';
    }

    //strip off last comma
    pngString = pngString.left(pngString.length()-1);

    QString mencoderCommand = "cd " + pngFileDirectory + " && " + mencoderexe + ' ' + videoFileName + " mf://" + pngString ;
    //QMessageBox::warning( NULL, QObject::tr( "Avogadro" ), mencoderCommand);
    int ret = system(mencoderCommand.toStdString().c_str());
    if (ret)
      QMessageBox::warning( NULL, QObject::tr( "Avogadro" ), QObject::tr("Could not run mencoder."));
  }

  double TrajVideoMaker::getAspectRatio(GLWidget* widget)
  {
    bool ok;
    int w = widget->width();
    int h = widget->height();
    double defaultAspectRatio = static_cast<double>(w)/h;
    double aspectRatio =
      QInputDialog::getDouble(0,
			      QObject::tr("Set Aspect Ratio"),
			      QObject::tr("The current Avogadro scene is %1x%2 pixels large, "
					  "and therefore has aspect ratio %3.\n"
					  "You may keep this value, for example if you intend to use POV-Ray\n"
					  "to produce an image of %4x1000 pixels, or you may enter any other positive value,\n"
					  "for example 1 if you intend to use POV-Ray to produce a square image, like 1000x1000 pixels.")
			      .arg(w).arg(h).arg(defaultAspectRatio)
			      .arg(static_cast<int>(1000*defaultAspectRatio)),
			      defaultAspectRatio,
			      0.1,
			      10,
			      6,
			      &ok);
    if (!ok) {
      aspectRatio = defaultAspectRatio;
    }
    return aspectRatio;
  }


}

