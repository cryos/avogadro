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

#include "abstract_ir.h"

using namespace std;

namespace Avogadro {
  AbstractIRSpectra::AbstractIRSpectra( SpectraDialog *parent ) :
    SpectraType( parent ), m_scale(0.0), m_fwhm(0.0), m_labelYThreshold(0.0)
    {
    ui.setupUi(m_tab_widget);

    // Setup signals/slots    
    connect(this, SIGNAL(plotDataChanged()),
            m_dialog, SLOT(regenerateCalculatedSpectra()));
    connect(ui.cb_labelPeaks, SIGNAL(toggled(bool)),
            this, SLOT(toggleLabels(bool)));
    connect(ui.spin_threshold, SIGNAL(valueChanged(double)),
            this, SLOT(updateThreshold(double)));
    connect(ui.spin_scale, SIGNAL(valueChanged(double)),
            this, SLOT(updateScaleSlider(double)));
    connect(ui.hs_scale, SIGNAL(sliderPressed()),
            this, SLOT(scaleSliderPressed()));
    connect(ui.hs_scale, SIGNAL(sliderReleased()),
            this, SLOT(scaleSliderReleased()));
    connect(ui.hs_scale, SIGNAL(valueChanged(int)),
            this, SLOT(updateScaleSpin(int)));
    connect(ui.spin_FWHM, SIGNAL(valueChanged(double)),
            this, SLOT(updateFWHMSlider(double)));
    connect(ui.hs_FWHM, SIGNAL(sliderPressed()),
            this, SLOT(fwhmSliderPressed()));
    connect(ui.hs_FWHM, SIGNAL(sliderReleased()),
            this, SLOT(fwhmSliderReleased()));
    connect(ui.hs_FWHM, SIGNAL(valueChanged(int)),
            this, SLOT(updateFWHMSpin(int)));
    connect(ui.combo_yaxis, SIGNAL(currentIndexChanged(QString)),
            this, SLOT(updateYAxis(QString)));
    connect(ui.combo_scalingType, SIGNAL(currentIndexChanged(int)),
            this, SLOT(changeScalingType(int)));
  }

  void AbstractIRSpectra::getCalculatedPlotObject(PlotObject *plotObject) {
    plotObject->clearPoints();

    if (m_fwhm == 0.0) { // get singlets
      plotObject->addPoint( 400, 0);

      for (int i = 0; i < m_yList.size(); i++) {
        double wavenumber = m_xList.at(i);// already scaled!
        double transmittance = m_yList.at(i);
        plotObject->addPoint ( wavenumber, 0 );
        if (ui.cb_labelPeaks->isChecked()) {
          // %L1 uses localized number format (e.g., 1.023,4 in Europe)
          plotObject->addPoint( wavenumber, transmittance, QString("%L1").arg(wavenumber, 0, 'f', 1) );
        }
        else {
          plotObject->addPoint( wavenumber, transmittance );
        }
        plotObject->addPoint( wavenumber, 0 );
      }
      plotObject->addPoint( 3500, 0);
    } // End singlets

    else { // Get gaussians
      // convert FWHM to sigma squared
      gaussianWiden(plotObject, m_fwhm);

      // Normalization is probably screwed up, so renormalize the data
      double min, max;
      min = max = plotObject->points().first()->y();
      for(int i = 0; i< plotObject->points().size(); i++) {
        double cur = plotObject->points().at(i)->y();
        if (cur < min) min = cur;
        if (cur > max) max = cur;
      }
      for(int i = 0; i< plotObject->points().size(); i++) {
        double cur = plotObject->points().at(i)->y();
        plotObject->points().at(i)->setY( (cur - min) * 100 / (max - min));
      }
    } // End gaussians
  }

  void AbstractIRSpectra::rescaleFrequencies()
  {
    for (int i=0; i<m_xList_orig.size(); i++) {
      m_xList[i] = m_xList_orig.at(i) * scale(m_xList.at(i));
    }
    emit plotDataChanged();
  }

  void AbstractIRSpectra::updateScaleSpin(int intScale)
  {
    double scale = intScale*0.01;
    if (scale == m_scale) return;
    m_scale = scale;
    ui.spin_scale->setValue(scale);
    rescaleFrequencies();
  }

  void AbstractIRSpectra::updateScaleSlider(double scale)
  {
    int intScale = static_cast<int>(scale*100);
    disconnect(ui.hs_scale, SIGNAL(valueChanged(int)),
      this, SLOT(updateScaleSpin(int)));
    ui.hs_scale->setValue(intScale);
    connect(ui.hs_scale, SIGNAL(valueChanged(int)),
      this, SLOT(updateScaleSpin(int)));
    m_scale = scale;
    rescaleFrequencies();
  }

  void AbstractIRSpectra::scaleSliderPressed()
  {
      disconnect(ui.spin_scale, SIGNAL(valueChanged(double)),
            this, SLOT(updateScaleSlider(double)));
  }

  void AbstractIRSpectra::scaleSliderReleased()
  {
      connect(ui.spin_scale, SIGNAL(valueChanged(double)),
            this, SLOT(updateScaleSlider(double)));
  }
  
  void AbstractIRSpectra::updateFWHMSpin(int fwhm)
  {
    if (fwhm == m_fwhm) return;
    m_fwhm = fwhm;
    ui.spin_FWHM->setValue(m_fwhm);
    emit plotDataChanged();
  }
  
  void AbstractIRSpectra::updateFWHMSlider(double fwhm)
  {    
    disconnect(ui.hs_FWHM, SIGNAL(valueChanged(int)),
      this, SLOT(updateFWHMSpin(int)));
    ui.hs_FWHM->setValue(fwhm);
    connect(ui.hs_FWHM, SIGNAL(valueChanged(int)),
      this, SLOT(updateFWHMSpin(int)));
    m_fwhm = fwhm;
    emit plotDataChanged();
  }
  
  void AbstractIRSpectra::fwhmSliderPressed()
  {
      disconnect(ui.spin_FWHM, SIGNAL(valueChanged(double)),
            this, SLOT(updateFWHMSlider(double)));
  }
  
  void AbstractIRSpectra::fwhmSliderReleased()
  {
      connect(ui.spin_FWHM, SIGNAL(valueChanged(double)),
            this, SLOT(updateFWHMSlider(double)));
  }

  void AbstractIRSpectra::updateYAxis(QString text) {
    if (m_yaxis == text) {
      return;
    }
    m_dialog->getUi()->plot->axis(PlotWidget::LeftAxis)->setLabel(text);
    m_yaxis = text;
    emit plotDataChanged();
  }

  void AbstractIRSpectra::changeScalingType(int type)
  {
    m_scalingType = static_cast<ScalingType>(type);
    rescaleFrequencies();
  }

  void AbstractIRSpectra::toggleLabels(bool enabled)
  {
    ui.spin_threshold->setEnabled(enabled);   
    emit plotDataChanged();
  }

  void AbstractIRSpectra::updateThreshold(double t)
  {
    m_labelYThreshold = t;
    emit plotDataChanged();
  }

  double AbstractIRSpectra::scale(double w)
  {
    switch(m_scalingType) {
      case LINEAR:
        return m_scale;
        break;
      case RELATIVE:
        return 1-w*(1-m_scale)/1000;
        break;
    //TODO: add other scaling algorithms
      default:
        return m_scale;
    }
  }
}
