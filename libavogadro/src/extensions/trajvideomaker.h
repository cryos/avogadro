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

#ifndef TRAJVIDEOMAKER_H
#define TRAJVIDEOMAKER_H

#include <avogadro/glwidget.h>

namespace Avogadro {

class Animation;

  class TrajVideoMaker
  {
  public:
    //! Constructor
        TrajVideoMaker();
    //! Destructor
    virtual ~TrajVideoMaker();

    static void makeVideo(GLWidget *widget, Animation *animation,
                          const QString& workDirectory,
                          const QString& videoFileName);

  private:
    static double getAspectRatio(GLWidget* widget);
    static void runPovRay(QString directory, QString povFileName);

    template <class QStringIterator>
    static void runMencoder(QString pngFileDirectory, QString videoFileName,
               QStringIterator startPngFiles, QStringIterator endPngFiles);

  };
}
#endif
