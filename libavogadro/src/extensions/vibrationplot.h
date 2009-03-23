/**********************************************************************
  VibrationPlot - Visualize vibrational modes graphically

  Copyright (C) 2009 by David Lonie

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.openmolecules.net/>

  Some code is based on Open Babel
  For more information, see <http://openbabel.sourceforge.net/>

  The plotting interface is provided by a version of KPlotWidget that 
  is included with avogadro. The PlotWidget, etc, files are copied 
  the kde tree with slight modifications.
  For more information, see <http://www.kde.org/>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation version 2 of the License.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
 ***********************************************************************/

#ifndef VIBRATIONPLOT_H
#define VIBRATIONPLOT_H

#include <QDialog>

#include <avogadro/primitive.h>
#include <avogadro/glwidget.h>
#include <avogadro/plotwidget.h>

#include "ui_vibrationplot.h"

namespace OpenBabel {
  class OBVibrationData;
}

namespace Avogadro {

  class VibrationPlot : public QDialog
  {
      Q_OBJECT

    public:
      //! Constructor
      explicit VibrationPlot( QWidget *parent = 0, Qt::WindowFlags f = 0 );
      //! Deconstructor
      ~VibrationPlot();

      void setMolecule(Molecule *molecule);

    public slots:
      void setScale(int scale);
      void setScale(double scale);
      void exportVibrationPlot(bool);

    private slots:
      void drawVibrationSpectra();
      void updateScaleEdit();

    signals:
      void scaleUpdated();

    private:
      Ui::VibrationPlot ui;
      
      PlotObject *m_vibrationPlotObject;
      double m_scale;
      Molecule *m_molecule;
      OpenBabel::OBVibrationData *m_vibrations;
  };
}

#endif
