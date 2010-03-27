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

#include "spectratype_abstract_ir.h"

using namespace std;

namespace Avogadro {
  AbstractIRSpectra::AbstractIRSpectra( SpectraDialog *parent ) :
    SpectraType( parent )
    {
    ui.setupUi(m_tab_widget);

    // Setup signals/slots    
    connect(this, SIGNAL(plotDataChanged()),
            m_dialog, SLOT(regenerateCalculatedSpectra()));
    connect(ui.cb_labelPeaks, SIGNAL(toggled(bool)),
            m_dialog, SLOT(regenerateCalculatedSpectra()));
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

  void AbstractIRSpectra::changeScalingType(int type) {
    m_scalingType = static_cast<ScalingType>(type);
    rescaleFrequencies();
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
