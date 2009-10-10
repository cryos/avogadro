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
#include <avogadro/atom.h>
#include <avogadro/bond.h>
#include <openbabel/mol.h>
#include <Eigen/Core>

#include <QTimeLine>

using namespace OpenBabel;
using Eigen::Vector3d;

namespace Avogadro {

  class AnimationPrivate
  {
    public:
      AnimationPrivate() : fps(25), framesSet(false), dynamicBonds(false) {}

      int fps;
      bool framesSet;
      bool dynamicBonds;
  };

  Animation::Animation(QObject *parent) : QObject(parent), d(new AnimationPrivate),
                                          m_molecule(0), m_timeLine(new QTimeLine)
  {
    // in chemical animations, each frame should take the same time
    m_timeLine->setCurveShape(QTimeLine::LinearCurve);
  }

  Animation::~Animation()
  {
    if (m_timeLine) {
      delete m_timeLine;
      m_timeLine = 0;
    }

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
    } else {
      m_timeLine->setFrameRange(0, m_molecule->numConformers() - 1);
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
    d->fps = fps;
  }

  int Animation::loopCount() const
  {
    return m_timeLine->loopCount();
  }

  void Animation::setLoopCount(int loops)
  {
    m_timeLine->setLoopCount(loops);
  }

  void Animation::setFrame(int i)
  {
    m_molecule->lock()->lockForWrite();
    m_molecule->setConformer(i);
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
    m_timeLine->setFrameRange(0, frames.size() - 1);
  }

  void Animation::stop()
  {
    m_timeLine->stop();
    m_timeLine->setCurrentTime(0);
    disconnect(m_timeLine, SIGNAL(frameChanged(int)),
            this, SLOT(setFrame(int)));

    // restore original conformers
    if (d->framesSet) {
      m_molecule->lock()->lockForWrite();
      m_molecule->setAllConformers(m_originalConformers);
      m_molecule->lock()->unlock();
    }
    setFrame(0);
  }

  void Animation::start()
  {
    // set molecule conformers
    if (d->framesSet) {

      if (!m_originalConformers.empty())
        m_originalConformers.clear();

      for (unsigned int i = 0; i < m_molecule->numConformers(); ++i) {
        m_originalConformers.push_back(m_molecule->conformer(i));
      }

      m_molecule->lock()->lockForWrite();
      m_molecule->setAllConformers(m_frames);
      m_molecule->lock()->unlock();
    }

    if (d->fps < 1.0)
      d->fps = 1.0;
    int interval = 1000 / d->fps;
    m_timeLine->setUpdateInterval(interval);
    int duration = interval * numFrames();
    m_timeLine->setDuration(duration);
    setFrame(0);

    connect(m_timeLine, SIGNAL(frameChanged(int)),
            this, SLOT(setFrame(int)));
    m_timeLine->setCurrentTime(0);
    m_timeLine->start();
  }
  
  void Animation::pause()
  {
    m_timeLine->stop();
  }

} // end namespace Avogadro

#include "animation.moc"
