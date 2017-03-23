/**********************************************************************
  Animation - Basic animation

  Copyright (C) 2008 by Tim Vandermeersch
  Copyright (C) 2009 by Geoffrey Hutchison

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.cc/>

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

#include "config.h"

#include "animation.h"

#include <avogadro/molecule.h>
#include <avogadro/atom.h>
#include <avogadro/bond.h>
#include <openbabel/mol.h>
#include <Eigen/Core>

#include <QTimer>

using namespace OpenBabel;
using Eigen::Vector3d;

namespace Avogadro {

  class AnimationPrivate
  {
    public:
      AnimationPrivate() : fps(25), framesSet(false), dynamicBonds(false),
                           loop(false), paused(false)
      {
      }

      int fps;
      bool framesSet;
      bool dynamicBonds;
      bool loop;
      bool paused;
  };

  Animation::Animation(QObject *parent) : QObject(parent), d(new AnimationPrivate),
                                          m_molecule(0), m_timer(new QTimer)
  {
    connect(m_timer, SIGNAL(timeout()), this, SLOT(timerFired()));
  }

  Animation::~Animation()
  {
    delete m_timer;
    delete d;
  }

  void Animation::setMolecule(Molecule *molecule)
  {
    m_molecule = molecule;
    if (molecule == NULL)
      return; // we can't save the current conformers

    if (d->framesSet) {
      m_originalConformers.clear();
      for (unsigned int i = 0; i < molecule->numConformers(); ++i) {
        m_originalConformers.push_back(molecule->conformer(i));
      }
    }
  }

  int Animation::numFrames() const
  {
    if (d->framesSet)
      return m_frames.size();
    if (m_molecule)
      return m_molecule->numConformers();
    return 0;  
  }
 
  int Animation::fps() const
  {
    return d->fps;
  }
 
  void Animation::setFps(int fps)
  {
    if (fps < 1)
      fps = 1;

    d->fps = fps;

    if (m_timer->isActive())
      startTimer();
  }

  bool Animation::loop() const
  {
    return d->loop;
  }

  void Animation::setLoop(const bool loop)
  {
    d->loop = loop;
  }

  void Animation::setFrame(int i)
  {
    if (i <= 0 || !m_molecule || i > (int)m_molecule->numConformers())
      return; // nothing to do

    m_molecule->lock()->lockForWrite();
    m_molecule->setConformer(i-1); // Frame counting starts from 1

    if (d->dynamicBonds) {
      // construct minimal OBMol
      OpenBabel::OBMol obmol;
      obmol.BeginModify();
      foreach(Atom *atom, m_molecule->atoms()) {
        OpenBabel::OBAtom *a = obmol.NewAtom();
        OpenBabel::OBAtom obatom = atom->OBAtom();
        *a = obatom;
      }
      obmol.EndModify();

      // connect the dots
      obmol.ConnectTheDots();

      QList<Bond*> bonds(m_molecule->bonds());
      foreach(Bond *bond, bonds)
        m_molecule->removeBond(bond->id());

      FOR_BONDS_OF_MOL (obbond, obmol) {
        Bond *bond = m_molecule->addBond();
        bond->setBegin(m_molecule->atom(obbond->GetBeginAtomIdx()-1));
        bond->setEnd(m_molecule->atom(obbond->GetEndAtomIdx()-1));
        bond->setOrder(obbond->GetBondOrder());
      }
    }
    m_molecule->lock()->unlock();
    m_molecule->update();
    emit frameChanged(i);
  }

  void Animation::sliderChanged(int i)
  {
    setFrame(i);
  }

  void Animation::sliderPressed()
  {
    if (m_timer->isActive()) {
      d->paused = true;
      m_timer->stop();
    }
  }

  void Animation::sliderReleased()
  {
    if (d->paused) {
      d->paused = false;
      startTimer();
    }
  }
 
  bool Animation::dynamicBonds() const
  {
    return d->dynamicBonds;
  }
 
  void Animation::setDynamicBonds(bool enable)
  {
    d->dynamicBonds = enable;
  }

  void Animation::setFrames(std::vector< std::vector< Eigen::Vector3d> *> frames)
  {
    if (frames.size() == 0)
      return; // nothing to do

    if (!m_originalConformers.empty())
      m_originalConformers.clear();
    if (m_molecule) {
      for (unsigned int i = 0; i < m_molecule->numConformers(); ++i) {
        m_originalConformers.push_back(m_molecule->conformer(i));
      }
    }
 
    d->framesSet = true;
    m_frames = frames;
  }

  void Animation::stop()
  {
    if(!m_molecule)
      return;

    m_timer->stop();

    // restore original conformers
    if (d->framesSet) {
      m_molecule->lock()->lockForWrite();
      m_molecule->setAllConformers(m_originalConformers);
      m_molecule->lock()->unlock();
    }
    setFrame(1);
  }

  void Animation::start()
  {
    if(!m_molecule)
      return;

    startTimer();

    // set molecule conformers
    if (d->framesSet) {
      m_molecule->lock()->lockForWrite();
      // don't delete the existing conformers -- we save them as m_originalConformers
      m_molecule->setAllConformers(m_frames, false);
      m_molecule->lock()->unlock();
    }

    if (m_molecule->currentConformer() + 1 == m_molecule->numConformers())
      setFrame(1);
  }
  
  void Animation::pause()
  {
    m_timer->stop();
  }

  void Animation::timerFired()
  {
    const unsigned int currentConformer = m_molecule->currentConformer();
    if (currentConformer + 1 == m_molecule->numConformers()) {
      if (d->loop) {
        setFrame(1);
      } else {
        m_timer->stop();
      }
    } else {
      setFrame(currentConformer + 1 + 1);
    }
  }

  void Animation::startTimer()
  {
    const int interval = 1000 / d->fps;
    m_timer->start(interval);
  }

} // end namespace Avogadro
