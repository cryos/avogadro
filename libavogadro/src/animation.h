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

#ifndef ANIMATION_H
#define ANIMATION_H

#include "global.h"

#include <QObject>

#include <Eigen/Core>

#include <vector>

class QTimeLine;

namespace Avogadro {

  class Molecule;

  /**
   * @class Animation animation.h <avogadro/animation.h>
   * @brief Simple frame-based animation for Molecule primitives
   * @author Geoffrey R. Hutchison
   *
   * The animation class supports 
   */
  class A_EXPORT Animation : public QObject
  {
    Q_OBJECT

    public:
    /**
     * Constructor.
     *
     * @param parent The object parent. Should be the widget viewing the
     * animation or a child of that widget (e.g., an Extension).
     */
      Animation(QObject *parent=0);
    /**
     * Deconstructor.
     */
      virtual ~Animation();
    
      void setFrames(std::vector< std::vector< Eigen::Vector3d> *> frames);
      void setMolecule(Molecule *molecule);

   public Q_SLOTS:
      void setDuration(int i);
      void setLoopCount(int loops);
      void setFrame(int i);

      /**
       * 
       */
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
