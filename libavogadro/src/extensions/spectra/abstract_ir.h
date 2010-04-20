/**********************************************************************
  SpectraDialog - Visualize spectral data from QM calculations

  Copyright (C) 2009 by David Lonie
  Copyright (C) 2010 by Konstantin Tokarev

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

#ifndef SPECTRATYPE_ABSTRACT_IR_H
#define SPECTRATYPE_ABSTRACT_IR_H

#include "spectradialog.h"
#include "spectratype.h"
#include "ui_tab_ir_raman.h"

namespace Avogadro {
    
  enum ScalingType { LINEAR, RELATIVE };

  // Abstract data type - no instance of it can be created
  class AbstractIRSpectra : public SpectraType
  {
    Q_OBJECT

  public:
    AbstractIRSpectra( SpectraDialog *parent = 0 );
    virtual void setupPlot(PlotWidget * plot) = 0;
    void getCalculatedPlotObject(PlotObject *plotObject);
    
  protected slots:
    void toggleLabels(bool);
    void updateThreshold(double);
    void updateScaleSpin(int);
    void updateScaleSlider(double);
    void scaleSliderPressed();
    void scaleSliderReleased();
    void updateFWHMSpin(int);
    void updateFWHMSlider(double);
    void fwhmSliderPressed();
    void fwhmSliderReleased();    
    void changeScalingType(int);
    void updateYAxis(QString);
    void rescaleFrequencies();

  protected:
    double scale(double w);
    
    Ui::Tab_IR_Raman ui;
    double m_scale;
    double m_fwhm;
    double m_labelYThreshold;
    QString m_yaxis;
    QList<double> m_xList_orig;
    ScalingType m_scalingType;
  };
}

#endif
