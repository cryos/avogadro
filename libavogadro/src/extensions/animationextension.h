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

#ifndef ANIMATIONEXTENSION_H
#define ANIMATIONEXTENSION_H


#include <openbabel/mol.h>

#include <avogadro/glwidget.h>
#include <avogadro/extension.h>
#include <avogadro/animation.h>

#include <QObject>
#include <QList>
#include <QString>
#include <QUndoCommand>
#include <QTimeLine>

#include "animationdialog.h"

namespace Avogadro {

 class AnimationExtension : public Extension
  {
    Q_OBJECT
    AVOGADRO_EXTENSION("Animation", tr("Animation"), 
                       tr("Animate trajectories, reactions, and vibratins."))

    public:
      //! Constructor
      AnimationExtension(QObject *parent=0);
      //! Deconstructor
      virtual ~AnimationExtension();

      //! Perform Action
      virtual QList<QAction *> actions() const;
      virtual QUndoCommand* performAction(QAction *action, GLWidget *widget);
      virtual QString menuPath(QAction *action) const;

      virtual void setMolecule(Molecule *molecule);
      //@}

    private:
      QList<QAction *> m_actions;
      Molecule *m_molecule;
      AnimationDialog *m_animationDialog;
      Animation *m_animation;
      
      //only needed for rendering a video
      GLWidget* m_widget;

    
   private Q_SLOTS:
      void loadFile(QString file);
      void setLoop(int state);
      void setDynamicBonds(int state);
      void saveVideo(QString videoFileName);

  private:
      //!support to read a trajectory from xyz as described here:
      //!http://www.ks.uiuc.edu/Research/vmd/plugins/molfile/xyzplugin.html
      void readTrajFromXyz(QString filename);

      //!support to write a trajectory to xyz as described here:
      //!http://www.ks.uiuc.edu/Research/vmd/plugins/molfile/xyzplugin.html
      bool writeXyzTraj(QString filename);
  };

  class AnimationExtensionFactory : public QObject, public PluginFactory
  {
      Q_OBJECT
      Q_INTERFACES(Avogadro::PluginFactory)
      AVOGADRO_EXTENSION_FACTORY(AnimationExtension)
  };

} // end namespace Avogadro

#endif
