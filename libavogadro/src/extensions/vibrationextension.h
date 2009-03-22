/**********************************************************************
  VibrationExtension - Visualize vibrational modes from QM calculations

  Copyright (C) 2009 by Geoffrey R. Hutchison

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

#ifndef VIBRATIONEXTENSION_H
#define VIBRATIONEXTENSION_H

#include "vibrationdialog.h"
#include "vibrationplot.h"

#include <avogadro/glwidget.h>
#include <avogadro/extension.h>
#include <avogadro/animation.h>

#include <QObject>
#include <QList>
#include <QString>
#include <QUndoCommand>

namespace OpenBabel {
  class OBVibrationData;
}

namespace Avogadro {

 class VibrationExtension : public Extension
  {
    Q_OBJECT
    AVOGADRO_EXTENSION("Vibration", tr("Vibration"),
                       tr("Visualize vibrational modes from quantum chemistry calculations"))

    public:
      //! Constructor
      VibrationExtension(QObject *parent=0);
      //! Deconstructor
      virtual ~VibrationExtension();

      //! Perform Action
      virtual QList<QAction *> actions() const;
      virtual QUndoCommand* performAction(QAction *action, GLWidget *widget);
      virtual QString menuPath(QAction *action) const;

      virtual void setMolecule(Molecule *molecule);

      void clearAnimationFrames();

    public slots:
      void updateMode(int mode);

      void setScale(double scale);
      void setDisplayForceVectors(bool enabled);
      void toggleAnimation();

    private:
      void updateForcesAndFrames(); // helper when settings change

      QList<QAction *> m_actions;

      OpenBabel::OBVibrationData *m_vibrations;
      int m_mode;
      VibrationDialog *m_dialog;
      Molecule *m_molecule;
      GLWidget *m_widget;
      Animation *m_animation;

      double m_scale;
      unsigned int m_framesPerStep;
      bool m_displayVectors;
      bool m_animating;

      std::vector< std::vector< Eigen::Vector3d> *> m_animationFrames;
  };

  class VibrationExtensionFactory : public QObject, public PluginFactory
  {
      Q_OBJECT
      Q_INTERFACES(Avogadro::PluginFactory)
      AVOGADRO_EXTENSION_FACTORY(VibrationExtension)
  };


} // end namespace Avogadro

#endif
