/**********************************************************************
  TrajVideoMaker - used to generate a video of a trajectory

  Copyright (C) 2008 by Naomi Fox

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

#include "trajvideomaker.h"
#include <avogadro/povpainter.h>
#include <QMessageBox>
#include <QInputDialog>

#include <fstream>



namespace Avogadro {
  
  TrajVideoMaker::TrajVideoMaker(){}

  TrajVideoMaker::~TrajVideoMaker(){}

  void TrajVideoMaker::makeVideo(GLWidget *widget, QString workDirectory, 
			QString videoFileName) 
  {

    //executable for povray.  -D suppresses the popup image.
    const QString povrayexe = "povray -D ";

    //executable for mencoder
    const QString mencoderexe = "mencoder -ovc lavc -lavcopts vcodec=mpeg4 -of avi -o ";

    if (!workDirectory.endsWith("/"))
      workDirectory += "/";
    
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
    
    //string to keep track of pngfiles for mencoder
    QString pngFiles;
    for (int i=0; i < molecule->NumConformers(); i++) {
      QString povFileName = workDirectory + QString::number(i) + ".pov";
      QString pngFileName = workDirectory + QString::number(i) + ".png";
      molecule->SetConformer(i);
      
      //write the pov file
      {
	//must be in scope so object is destroyed and file is closed after
	POVPainterDevice pd( povFileName, aspectRatio, widget );
      }

      //make the png
      QString povRayCommand = "cd " +  workDirectory + 
	" && " + povrayexe + " " + povFileName; 
      //QMessageBox::warning( NULL, QObject::tr( "Avogadro" ), povRayCommand);
      system(povRayCommand.toStdString().c_str());
      pngFiles += pngFileName + ","; 
      
    }


     //strip off extra comma
    pngFiles = pngFiles.left(pngFiles.length()-1);

    //now run mencoder
    QString mencoderCommand = "cd " + workDirectory + " && " + mencoderexe + " " + videoFileName + " mf://" + pngFiles ;
    //QMessageBox::warning( NULL, QObject::tr( "Avogadro" ), mencoderCommand);
    system(mencoderCommand.toStdString().c_str());


    //tell user if successful
    std::ifstream fin(videoFileName.toStdString().c_str());
    if(!fin.fail()) {
	fin.close();
	QString successMessage = "Video file " + videoFileName + " written.";
	QMessageBox::information( NULL, QObject::tr( "Avogadro" ), 
				  successMessage); 
  }
    else {
	QString failedMessage = QObject::tr("Video file not written.");
	QMessageBox::information( NULL, QObject::tr( "Avogadro" ), failedMessage); 
      }
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

