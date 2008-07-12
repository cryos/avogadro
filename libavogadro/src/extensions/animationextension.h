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

#ifndef __ANIMATIONEXTENSION_H
#define __ANIMATIONEXTENSION_H


#include <openbabel/mol.h>

#include <avogadro/glwidget.h>
#include <avogadro/extension.h>

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

    public:
      //! Constructor
      AnimationExtension(QObject *parent=0);
      //! Deconstructor
      virtual ~AnimationExtension();

      //! \name Description methods
      //@{
      //! Plugin Name (ie Draw)
      virtual QString name() const { return QObject::tr("Animation"); }
      //! Plugin Description (ie. Draws atoms and bonds)
      virtual QString description() const { return QObject::tr("Animation Plugin"); };
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
      QTimeLine *m_timeLine;

      int m_frameCount;
    
    private Q_SLOTS:
      void loadFile(QString file);
      void setDuration(int i);
      void setLoop(int state);
      void setFrame(int i);
      void stop();
  };

  class AnimationExtensionFactory : public QObject, public PluginFactory
  {
      Q_OBJECT
      Q_INTERFACES(Avogadro::PluginFactory)

    public:
      Plugin *createInstance(QObject *parent = 0) { return new AnimationExtension(parent); }
      int type() const { return Plugin::ExtensionType; };
      QString name() const { return tr("Animate Extension"); };
      QString description() const { return tr("Extension for animating trajectories."); };
  };

} // end namespace Avogadro

#endif
