/**********************************************************************
  Animation - Basic animation

  Copyright (C) 2008 by Tim Vandermeersch

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

#include "animationextension.h"
#include "trajvideomaker.h"
#include <avogadro/molecule.h>
#include <avogadro/color.h>
#include <avogadro/animation.h>
#include <avogadro/glwidget.h>

#include <openbabel/mol.h>
#include <openbabel/obconversion.h>

#include <Eigen/Core>

#include <QMessageBox>
#include <QDir>

#include <fstream>

using namespace OpenBabel;
using Eigen::Vector3d;

namespace Avogadro {

  AnimationExtension::AnimationExtension(QObject *parent) : Extension(parent),
    m_molecule(0), m_animationDialog(0), m_animation(0), m_widget(0)
  {
    QAction *action = new QAction(this);
    action->setText(tr("Animation..."));
    m_actions.append(action);

    action = new QAction( this );
    action->setSeparator(true);
    m_actions.append(action);
  }

  AnimationExtension::~AnimationExtension()
  {
    if (m_animation) {
      delete m_animation;
      m_animation = 0;
    }

    if (m_animationDialog) {
      m_animationDialog->deleteLater();
    }
  }

  QList<QAction *> AnimationExtension::actions() const
  {
    return m_actions;
  }

  QString AnimationExtension::menuPath(QAction *) const
  {
    return tr("E&xtensions");
  }

  void AnimationExtension::setMolecule(Molecule *molecule)
  {
    m_molecule = molecule;
  }

  QUndoCommand* AnimationExtension::performAction(QAction *, GLWidget* widget)
  {
    m_widget = widget;

    if (!m_animation) {
      m_animation = new Animation;
    }
    m_animation->setMolecule(widget->molecule());

    if (!m_animationDialog)
    {
      m_animationDialog = new AnimationDialog(static_cast<QWidget*>(parent()));

      connect(m_animationDialog, SIGNAL(fileName(QString)), this, SLOT(loadFile(QString)));
      connect(m_animationDialog, SIGNAL(sliderChanged(int)), m_animation, SLOT(setFrame(int)));
      connect(m_animationDialog, SIGNAL(fpsChanged(int)), m_animation, SLOT(setFps(int)));
      connect(m_animationDialog, SIGNAL(loopChanged(int)), this, SLOT(setLoop(int)));
      connect(m_animationDialog, SIGNAL(dynamicBondsChanged(int)), this, SLOT(setDynamicBonds(int)));

      connect(m_animationDialog, SIGNAL(play()), m_animation, SLOT(start()));
      connect(m_animationDialog, SIGNAL(pause()), m_animation, SLOT(pause()));
      connect(m_animationDialog, SIGNAL(stop()), m_animation, SLOT(stop()));
      connect(m_animationDialog, SIGNAL(videoFileInfo(QString)), this, SLOT(saveVideo(QString)));

      connect(m_animation, SIGNAL(frameChanged(int)), m_animationDialog, SLOT(setFrame(int)));
    }

    m_animationDialog->setFrameCount(m_animation->numFrames());
    m_animationDialog->setFrame(1);

    m_animationDialog->show();

    return 0;
  }

  void AnimationExtension::loadFile(QString file)
  {
    //qDebug() << "AnimationExtension::loadFile()" << endl;

    if (file.isEmpty())
      return;

    if (file.endsWith(QLatin1String(".xyz"))) {
      readTrajFromXyz(file);
    }

    else { //non xyz

      OBConversion conv;
      OBFormat *inFormat = conv.FormatFromExt(( file.toAscii() ).data() );

      if ( !inFormat || !conv.SetInFormat( inFormat ) ) {
        QMessageBox::warning( NULL, tr( "Avogadro" ),
            tr( "Cannot read file format of file %1." )
            .arg( file ) );
        return;
      }

      OpenBabel::OBMol obmol;
      if (!conv.ReadFile(&obmol, file.toStdString())) {
        QMessageBox::warning( NULL, tr( "Avogadro" ),
                              tr( "Read trajectory file %1 failed." )
                              .arg( file ) );
        return;
      }
      else
        m_molecule->setOBMol(&obmol);
    }

    m_animationDialog->setFrameCount(m_animation->numFrames());
    m_animationDialog->setFrame(1);
    m_animation->setFps(m_animationDialog->fps());
  }

  void AnimationExtension::setLoop(int state)
  {
    if (state == Qt::Checked) {
      m_animation->setLoopCount(0);
    } else {
      m_animation->setLoopCount(1);
    }
  }

  void AnimationExtension::setDynamicBonds(int state)
  {
    if (state == Qt::Checked) {
      m_animation->setDynamicBonds(true);
    } else {
      m_animation->setDynamicBonds(false);
    }
  }

  void AnimationExtension::saveVideo(QString videoFileName)
  {
    if (videoFileName.isEmpty()) {
      QMessageBox::warning( NULL, tr( "Avogadro" ),
          tr( "Must specify a valid .avi file name" ));
      return;
    }

    if (!videoFileName.endsWith(QLatin1String(".avi"))) {
      QMessageBox::warning( NULL, tr( "Avogadro" ),
          tr( "Must specify a valid .avi file name" ));
      return;
    }

    if (!m_widget) {
      QMessageBox::warning( NULL, tr( "Avogadro" ),
          tr( "GL widget was not correctly initialized in order to save video" ));
      return;
    }

    //first, split out the directory and filenames
    QString dir, fileName, prefix;

    int slashPos = videoFileName.lastIndexOf('/');

    if (slashPos < 0) {
      QMessageBox::warning( NULL, tr( "Avogadro" ),
                tr( "Invalid video filename.  Must include full directory path" ));
      return;
    }

    dir = videoFileName.left(slashPos) + '/';
    fileName = videoFileName.right(videoFileName.length() - (slashPos+1));
    if (fileName.isEmpty()) {
      QMessageBox::warning( NULL, tr( "Avogadro" ),
                tr( "Invalid video filename.  Must include full directory path and name, ending with .avi" ));
      return;
    }

    //if (fileName.endsWith(".avi")) {
    prefix = fileName.left(fileName.length() - 4);


    //Make the directory where the snapshots will be saved
    QString snapshotsDir = dir + prefix + '/';
    QDir qdir;
    if (!qdir.exists(snapshotsDir))
      qdir.mkpath(snapshotsDir);

    TrajVideoMaker::makeVideo(m_widget, snapshotsDir, videoFileName);

  }

  void AnimationExtension::readTrajFromXyz(QString xyzfile)
  {
    OBConversion conv;
    if (!conv.SetInFormat("XYZ")) {
      QMessageBox::warning( NULL, tr( "Avogadro" ),
                            tr( "could not set format to XYZ" ));
    }

    m_molecule->clearConformers();
#ifdef Q_CC_MSVC
    std::ifstream file(xyzfile.toStdWString().c_str());
#else
    std::ifstream file(QFile::encodeName(xyzfile));
#endif

    OpenBabel::OBMol tmpMol;
    int i=0;
    while (conv.Read(&tmpMol, &file)) {
      double* tmpCoords = tmpMol.GetCoordinates();
      if (!tmpCoords) {
        QMessageBox::warning( NULL, tr( "Avogadro" ),
                              tr( "Problem reading traj file %1").arg(xyzfile));
        return;
      }

      if (tmpMol.NumAtoms() != m_molecule->numAtoms()) {
        QMessageBox::warning( NULL, tr( "Avogadro" ),
          tr( "Trajectory file %1 disagrees on the number of atoms in the present molecule").arg(xyzfile));
        return;
      }

      std::vector<Eigen::Vector3d> *coords = m_molecule->addConformer(i);
      //copy coords read in to a new array that will not be deleted
      for (uint j = 0; j < tmpMol.NumAtoms(); ++j) {
        (*coords)[j] = Vector3d(tmpCoords[3*j], tmpCoords[3*j+1], tmpCoords[3*j+2]);
      }
      ++i;
    }

    file.close();
  }

  bool AnimationExtension::writeXyzTraj(QString filename) {
    OBConversion conv;
    conv.SetInAndOutFormats("XYZ","XYZ");

#ifdef Q_CC_MSVC
    std::ofstream file(fileName.toStdWString().c_str());
#else
    std::ofstream file(QFile::encodeName(filename));
#endif

    for (unsigned int i = 1; i <= m_molecule->numConformers(); ++i) {
      m_animation->setFrame(i);

      OpenBabel::OBMol obmol(m_molecule->OBMol());
      conv.Write(&obmol, &file);
      file << std::endl;
    }

    file.close();

    return true;
  }

} // end namespace Avogadro

Q_EXPORT_PLUGIN2(animationextension, Avogadro::AnimationExtensionFactory)

