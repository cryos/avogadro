/**********************************************************************
  VibrationDialog - Visualize and animate vibrational modes

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

#ifndef VIBRATIONDIALOG_H
#define VIBRATIONDIALOG_H

#include <QDialog>

#include <avogadro/primitive.h>
#include <avogadro/glwidget.h>

#include "ui_vibrationdialog.h"

namespace OpenBabel {
  class OBVibrationData;
}

namespace Avogadro {

  class VibrationDialog : public QDialog
  {
      Q_OBJECT

    public:
      //! Constructor
      explicit VibrationDialog( QWidget *parent = 0, Qt::WindowFlags f = 0 );
      //! Deconstructor
      ~VibrationDialog();

      void setMolecule(Molecule *molecule);
      void setWidget(GLWidget *widget) {m_widget = widget; }

    public slots:
      void accept();
      void reject();
      void cellClicked(int, int);

    signals:
      void selectedMode(int);

    private:
      Ui::VibrationDialog ui;
      
      GLWidget *m_widget;
      Molecule *m_molecule;
      OpenBabel::OBVibrationData *m_vibrations;
  };
}

#endif
