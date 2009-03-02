/**********************************************************************
  Animation - Basic animation interface

  Copyright (C) 2008 by Tim Vandermeersch
  Some Portions Copyright (C) 2009 by Geoffrey Hutchison

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

#ifndef __ANIMATION_H
#define __ANIMATION_H

#include <QObject>

#include <vector>
#include <avogadro/atom.h>

class QTimeLine;

namespace Avogadro {

 class Animation : public QObject
  {
    Q_OBJECT

    public:
      //! Constructor
      Animation(QObject *parent=0);
      //! Deconstructor
      virtual ~Animation();
    
      void setFrames(std::vector< std::vector< Eigen::Vector3d> *> frames);
      void setMolecule(Molecule *molecule);

   public Q_SLOTS:
      void setDuration(int i);
      void setLoop(int state);
      void setFrame(int i);

      void start();
      void stop();

    private:
      Molecule *m_molecule;
      QTimeLine *m_timeLine;
      std::vector< std::vector< Eigen::Vector3d> *> m_originalConformers;
      std::vector< std::vector< Eigen::Vector3d> *> m_frames;
  };

} // end namespace Avogadro

#endif
