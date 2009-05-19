/**********************************************************************
  LinMorph - compute a lin morph and display given a second conformation 
             of the current molecule

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

#ifndef LINMORPHEXTENSION_H
#define LINMORPHEXTENSION_H

#include <avogadro/extension.h>
#include <avogadro/primitive.h>
#include <avogadro/glwidget.h>
#include <QTimeLine>

#include "linmorphdialog.h"

namespace Avogadro {
  
  class LinMorphExtension : public Extension
  {
    Q_OBJECT
    AVOGADRO_EXTENSION("LinMorph", tr("Linear Morph"),
                       tr("Create linear interpolation for animations"))

      public:
      //! Constructor
      LinMorphExtension(QObject *parent=0);
      //! Deconstructor
      virtual ~LinMorphExtension();

      virtual QString menuPath(QAction *action) const;
      virtual QList<QAction *> actions() const;

      virtual QDockWidget * dockWidget();
      virtual QUndoCommand* performAction(QAction *action, GLWidget *widget);

      virtual void setMolecule(Molecule *molecule);
      
  protected:
      Molecule *m_molecule;
      Molecule *m_secondMolecule;
      
      // this will be the glwidget passed from performAction
      GLWidget *m_widget;
      
      QList<QAction *> m_actions;
      LinMorphDialog *m_linMorphDialog;
      QTimeLine *m_timeLine;

      // the total number of frames
      int m_frameCount;
    
   protected Q_SLOTS:
      void saveMovie(QString movieFileName);
      void saveGlSnapshots(QString prefix);
      //void savePovSnapshots(QString prefix);
      void saveTrajectoryFile(QString filename);
      void setDuration(int i);
      void setLoop(int state);
      void setFrame(int i);
      void setFrameCount(int i);
      void stop();
      virtual void loadFile(QString file);

      
  private:
      virtual void computeConformers(Molecule* conformer2Mol);

      bool writeXYZTraj(QString filename);
                  
  };

  class LinMorphExtensionFactory : public QObject, public PluginFactory
  {
    Q_OBJECT
    Q_INTERFACES(Avogadro::PluginFactory)  
    AVOGADRO_EXTENSION_FACTORY(LinMorphExtension)
  };

}
#endif
