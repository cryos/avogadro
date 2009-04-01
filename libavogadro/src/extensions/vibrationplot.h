/**********************************************************************
  VibrationPlot - Visualize vibrational modes graphically

  Copyright (C) 2009 by David Lonie

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.openmolecules.net/>

  This library is free software; you can redistribute it and/or modify
  it under the terms of the GNU Library General Public License as
  published by the Free Software Foundation; either version 2.1 of the
  License, or (at your option) any later version.

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

#include <openbabel/mol.h>
#include <openbabel/generic.h>

#include "ui_vibrationplot.h"

namespace Avogadro {

  class VibrationPlot : public QDialog
  {
    Q_OBJECT

  public:
    //! Constructor
    explicit VibrationPlot( QWidget *parent = 0, Qt::WindowFlags f = 0 );
    //! Destructor
    ~VibrationPlot();

    void setMolecule(Molecule *molecule);

  public slots:
    void setScale(double scale);

  private slots:
    void changeBackgroundColor();
    void changeForegroundColor();
    void changeCalculatedSpectraColor();
    void changeImportedSpectraColor();
    void changeFontSize(int size);
    void importSpectra();
    void updatePlot();
    void toggleImported(bool state);
    void toggleCalculated(bool state);
    void regenerateCalculatedSpectra();
    void regenerateImportedSpectra();
    void saveImage();
    void toggleCustomize();
    void updateYAxis(QString);

  signals:
    void scaleUpdated();

  private:
    Ui::VibrationPlot ui;

    void getCalculatedSpectra(PlotObject *vibrationPlotObject);
    void getImportedSpectra(PlotObject *vibrationPlotObject);

    double m_scale;
    QString m_yaxis;
    PlotObject *m_calculatedSpectra;
    PlotObject *m_importedSpectra;
    PlotObject *m_nullSpectra;
    std::vector<double> m_wavenumbers;
    std::vector<double> m_transmittances;
    std::vector<double> m_imported_wavenumbers;
    std::vector<double> m_imported_transmittances;
    Molecule *m_molecule;
    OpenBabel::OBVibrationData *m_vibrations;
  };
}

#endif
