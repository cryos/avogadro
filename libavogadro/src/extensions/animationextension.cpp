/**********************************************************************
  Animation - Basic animation

  Copyright (C) 2008 by Tim Vandermeersch

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

#include "animationextension.h"
#include <avogadro/primitive.h>
#include <avogadro/color.h>
#include <avogadro/glwidget.h>

#include <openbabel/obconversion.h>

#include <QtGui>
#include <QMessageBox>
//#include <QDebug>

using namespace OpenBabel;

namespace Avogadro {

  AnimationExtension::AnimationExtension(QObject *parent) : Extension(parent), m_molecule(0),
      m_animationDialog(0), m_timeLine(0), m_frameCount(0)
  {
    QAction *action = new QAction(this);
    action->setText(tr("Animation"));
    m_actions.append(action);
  }

  AnimationExtension::~AnimationExtension()
  {
    if (m_animationDialog)
    {
      delete m_animationDialog;
      m_animationDialog = 0;
    }
 
    if (m_timeLine)
    {
      delete m_timeLine;
      m_timeLine = 0;
    }
  }

  QList<QAction *> AnimationExtension::actions() const
  {
    return m_actions;
  }

  QString AnimationExtension::menuPath(QAction *) const
  {
    return tr("&Extensions");
  }

  void AnimationExtension::setMolecule(Molecule *molecule)
  {
    m_molecule = molecule;
  }

  QUndoCommand* AnimationExtension::performAction( QAction *, GLWidget *widget )
  {
    if (!m_animationDialog)
    {
      m_timeLine = new QTimeLine;
      m_animationDialog = new AnimationDialog;

      connect(m_animationDialog, SIGNAL(fileName(QString)), this, SLOT(loadFile(QString)));
      connect(m_animationDialog, SIGNAL(sliderChanged(int)), this, SLOT(setFrame(int)));
      connect(m_animationDialog, SIGNAL(fpsChanged(int)), this, SLOT(setDuration(int)));
      connect(m_animationDialog, SIGNAL(loopChanged(int)), this, SLOT(setLoop(int)));

      connect(m_timeLine, SIGNAL(frameChanged(int)), this, SLOT(setFrame(int)));
      connect(m_animationDialog, SIGNAL(play()), m_timeLine, SLOT(start()));
      connect(m_animationDialog, SIGNAL(pause()), m_timeLine, SLOT(stop()));
      connect(m_animationDialog, SIGNAL(stop()), this, SLOT(stop()));
    } 

    m_animationDialog->show();

    return 0;
  }

  void AnimationExtension::loadFile(QString file)
  {
    //qDebug() << "AnimationExtension::loadFile()" << endl;

    if (file.isEmpty())
      return;

    OBConversion conv;
    OBFormat *inFormat = conv.FormatFromExt(( file.toAscii() ).data() );
    if ( !inFormat || !conv.SetInFormat( inFormat ) ) {
      QMessageBox::warning( NULL, tr( "Avogadro" ),
          tr( "Cannot read file format of file %1." )
          .arg( file ) );
      return;
    }

    if (!conv.ReadFile(m_molecule, file.toStdString())) {
      QMessageBox::warning( NULL, tr( "Avogadro" ),
          tr( "Read trajectory file %1 failed." )
          .arg( file ) );
      return;
    }

    m_frameCount = m_molecule->NumConformers();
    m_animationDialog->setFrameCount(m_frameCount);
    m_animationDialog->setFrame(1);
    m_timeLine->setFrameRange(1, m_frameCount);
    setDuration(m_animationDialog->fps());
  }

  void AnimationExtension::setDuration(int i)
  {
    int interval = 1000 / i;
    m_timeLine->setUpdateInterval(interval);
    int duration = interval * m_frameCount;
    m_timeLine->setDuration(duration);
  }

  void AnimationExtension::setLoop(int state)
  {
    if (state == Qt::Checked) {
      m_timeLine->setLoopCount(0);
    } else {
      m_timeLine->setLoopCount(1);
    }
  }

  void AnimationExtension::setFrame(int i)
  {
    // if (m_timeLine->state() != QTimeLine::Running)
    //  m_timeLine->setCurrentTime(m_timeLine->updateInterval() * i);

    m_animationDialog->setFrame(i);
    m_molecule->SetConformer(i - 1);
    m_molecule->update();
  }

  void AnimationExtension::stop()
  {
    m_timeLine->stop();
    m_timeLine->setCurrentTime(0);
    setFrame(1);
  }


} // end namespace Avogadro

#include "animationextension.moc"
Q_EXPORT_PLUGIN2(animationextension, Avogadro::AnimationExtensionFactory)
