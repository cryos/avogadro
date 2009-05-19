/**********************************************************************
  Animation - Basic animation interface

	Copyright (c) 2009 Tim Vandermeersch
	Copyright (c) 2009 Geoff Hutchison
	Copyright (c) 2009 Marcus D. Hanwell

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
  class AnimationPrivate;
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
    
      /**
       * Set the molecule to animate.
       */
      void setMolecule(Molecule *molecule);
      /**
       * Set the frames for the animation. By default, the conformers in the 
       * molecule are used as animation frames. However, for trajectory files
       * that don't contain any topology, it is needed to read in the the 
       * molecule topology before the trajectory. The trajectory frames can 
       * be used to call setFrames() later.
       */
      void setFrames(std::vector< std::vector< Eigen::Vector3d> *> frames);

      /**
       * @return The number of frames per second.
       */
      int fps() const;
      /**
       * @return The loopCount (0 = repeat forever).
       */
      int loopCount() const;
      /**
       * @return The total number of frames in the animation.
       */
      int numFrames() const;
      /**
       * @return True if dynamic bond detection is enabled.
       */
      bool dynamicBonds() const;

    Q_SIGNALS:
      /**
       * This signal is emitted when the current frame is changed (i.e. setFrame() called)
       */
      void frameChanged(int);

    public Q_SLOTS:
      /**
       * Set the number of frames per second.
       */
      void setFps(int fps);
      /**
       * Set the loop count. (0 = repeat forever)
       */
      void setLoopCount(int loops);
      /**
       * Set the current frame. 
       */
      void setFrame(int i);

      /**
       * Enable/disable dynamic bond detection. For QM reactions for example.
       */
      void setDynamicBonds(bool enable);

      /**
       * Start the animation (at current frame).
       */
      void start();
      /**
       * Pause the animation.
       */
      void pause();
      /**
       * Stop the animation (and return to first frame).
       */
      void stop();

    private:
      AnimationPrivate * const d;
      
      Molecule *m_molecule;
      QTimeLine *m_timeLine;
      std::vector< std::vector< Eigen::Vector3d> *> m_originalConformers;
      std::vector< std::vector< Eigen::Vector3d> *> m_frames;
  };

} // end namespace Avogadro

#endif
