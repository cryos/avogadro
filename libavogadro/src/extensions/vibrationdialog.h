/**********************************************************************
  VibrationWidget - Visualize and animate vibrational modes

  Copyright (C) 2009 by Geoffrey Hutchison
  Some portions Copyright (C) 2010 by Konstantin Tokarev

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

#ifndef VIBRATIONDIALOG_H
#define VIBRATIONDIALOG_H

#include <QtGui/QWidget>

#include <avogadro/primitive.h>
#include <avogadro/glwidget.h>

#include "ui_vibrationwidget.h"

namespace OpenBabel {
  class OBVibrationData;
}

namespace Avogadro {

  class VibrationWidget : public QWidget
  {
      Q_OBJECT

    public:
      //! Constructor
      explicit VibrationWidget( QWidget *parent = 0, Qt::WindowFlags f = 0 );
      //! Deconstructor
      ~VibrationWidget();

      void setMolecule(Molecule *molecule);
      void setWidget(GLWidget *widget) {m_widget = widget; }

    public slots:
      void accept();
      void reject();
      void cellClicked(int, int);
      void currentCellChanged(int, int, int, int);

      void setScale(int scale);
      void setScale(double scale);
      void setDisplayForceVectors(bool checked);
      void setAnimationSpeed(bool checked);
      void animateButtonClicked(bool);
      void exportVibrationData(bool);

    signals:
      void selectedMode(int);
      void scaleUpdated(double scale);
      void forceVectorUpdated(bool enabled);
      void animationSpeedUpdated(bool enabled);
      void toggleAnimation();

    private:
      Ui::VibrationWidget ui;

      GLWidget *m_widget;
      Molecule *m_molecule;
      OpenBabel::OBVibrationData *m_vibrations;
      std::vector<int> *m_indexMap;
  };
}

#endif
