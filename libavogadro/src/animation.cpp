/**********************************************************************
  Animation - Basic animation

  Copyright (C) 2008 by Tim Vandermeersch
  Copyright (C) 2009 by Geoffrey Hutchison

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

#include "animation.h"

#include <avogadro/molecule.h>

#include <Eigen/Core>

#include <QTimeLine>

using namespace OpenBabel;
using Eigen::Vector3d;

namespace Avogadro {

  Animation::Animation(QObject *parent) : QObject(parent),
                                          m_molecule(0), m_timeLine(new QTimeLine)
  {
  }

  Animation::~Animation()
  {
    if (m_timeLine) {
      delete m_timeLine;
      m_timeLine = 0;
    }
  }

  void Animation::setMolecule(Molecule *molecule)
  {
    m_molecule = molecule;
    m_originalConformers.clear();

    if (molecule == NULL)
      return; // we can't save the current conformers

    for (unsigned int i = 0; i < molecule->numConformers(); ++i) {
      m_originalConformers.push_back(molecule->conformer(i));
    }
  }

  void Animation::setDuration(int i)
  {
    int interval = 1000 / i;
    m_timeLine->setUpdateInterval(interval);
    int duration = interval * m_frames.size();
    m_timeLine->setDuration(duration);
  }

  void Animation::setLoopCount(int loops)
  {
    m_timeLine->setLoopCount(loops);
  }

  void Animation::setFrame(int i)
  {
    m_molecule->setConformer(i - 1);
    m_molecule->update();
  }

  void Animation::setFrames(std::vector< std::vector< Eigen::Vector3d> *> frames)
  {
    if (frames.size() == 0)
      return; // nothing to do

    m_frames = frames;
    m_timeLine->setFrameRange(1, frames.size());
  }

  void Animation::stop()
  {
    m_timeLine->stop();
    m_timeLine->setCurrentTime(0);
    disconnect(m_timeLine, SIGNAL(frameChanged(int)),
            this, SLOT(setFrame(int)));

    // restore original conformers
    m_molecule->setAllConformers(m_originalConformers);
    setFrame(1);
  }

  void Animation::start()
  {
    // set molecule conformers
    m_molecule->setAllConformers(m_frames);
    setFrame(1);

    connect(m_timeLine, SIGNAL(frameChanged(int)),
            this, SLOT(setFrame(int)));
    m_timeLine->setCurrentTime(0);
    m_timeLine->start();
  }

} // end namespace Avogadro

#include "animation.moc"
