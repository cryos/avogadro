/**********************************************************************
  SpectraDialog - Visualize spectral data from QM calculations

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

#include "spectratype_raman.h"
#include "spectratype.h"
#include "spectradialog.h"

#include <QtCore/QTextStream>
#include <QtGui/QMessageBox>
#include <QtCore/QDebug>

#include <openbabel/mol.h>
#include <openbabel/generic.h>

using namespace std;

namespace Avogadro {

  RamanSpectra::RamanSpectra( SpectraDialog *parent ) :
    SpectraType( parent ), m_dialog(parent)
  {
    m_tab_widget = new QWidget;
    ui.setupUi(m_tab_widget);

    m_xList = new QList<double>;
    m_yList = new QList<double>;
    m_xList_imp = new QList<double>;
    m_yList_imp = new QList<double>;

    m_dialog = parent;

    // Setup signals/slots
    connect(this, SIGNAL(plotDataChanged()),
            m_dialog, SLOT(regenerateCalculatedSpectra()));
    connect(ui.cb_labelPeaks, SIGNAL(toggled(bool)),
            m_dialog, SLOT(regenerateCalculatedSpectra()));
    connect(ui.spin_scale, SIGNAL(valueChanged(double)),
            this, SLOT(setScale(double)));
    connect(ui.spin_FWHM, SIGNAL(valueChanged(double)),
            m_dialog, SLOT(regenerateCalculatedSpectra()));
    connect(ui.combo_yaxis, SIGNAL(currentIndexChanged(QString)),
            this, SLOT(updateYAxis(QString)));

    readSettings();
  }

   RamanSpectra::~RamanSpectra() {
     // TODO: Anything to delete?
     writeSettings();
     delete m_xList;
     delete m_yList;
     delete m_xList_imp;
     delete m_yList_imp;
     delete m_tab_widget;
   }

  void RamanSpectra::writeSettings() {
    QSettings settings; // Already set up in avogadro/src/main.cpp

    settings.setValue("spectra/Raman/scale", m_scale);
    settings.setValue("spectra/Raman/gaussianWidth", ui.spin_FWHM->value());
    settings.setValue("spectra/Raman/labelPeaks", ui.cb_labelPeaks->isChecked());
    settings.setValue("spectra/Raman/yAxisUnits", ui.combo_yaxis->currentText());
  }

  void RamanSpectra::readSettings() {
    QSettings settings; // Already set up in avogadro/src/main.cpp
    ui.spin_scale->setValue(settings.value("spectra/Raman/scale", 1.0).toDouble());
    ui.spin_FWHM->setValue(settings.value("spectra/Raman/gaussianWidth",0.0).toDouble());
    ui.cb_labelPeaks->setChecked(settings.value("spectra/IR/labelPeaks",false).toBool());
    updateYAxis(settings.value("spectra/Raman/yAxisUnits","Absorbance (%)").toString());
    emit plotDataChanged();
  }

  bool RamanSpectra::checkForData(Molecule * mol) {
    OpenBabel::OBMol obmol = mol->OBMol();
    OpenBabel::OBVibrationData *vibrations = static_cast<OpenBabel::OBVibrationData*>(obmol.GetData(OpenBabel::OBGenericDataType::VibrationData));
    if (!vibrations) return false;

    // OK, we have valid vibrations, so store them for later
    vector<double> wavenumbers = vibrations->GetFrequencies();
    vector<double> intensities = vibrations->GetRamanActivities();

    // Case where there are no intensities, set all intensities to an arbitrary value, i.e. 1.0
    if (wavenumbers.size() > 0 && intensities.size() == 0) {
      // Warn user
      QMessageBox::information(m_dialog, tr("No intensities"), tr("The vibration data in the molecule you have loaded does not have any intensity data. Intensities have been set to an arbitrary value for visualization."));
      for (uint i = 0; i < wavenumbers.size(); i++) {
        intensities.push_back(1.0);
      }
    }

    // Normalize intensities into transmittances
    double maxIntensity=0;
    for (unsigned int i = 0; i < intensities.size(); i++) {
      if (intensities.at(i) >= maxIntensity) {
        maxIntensity = intensities.at(i);
      }
    }

    vector<double> transmittances;

    for (unsigned int i = 0; i < intensities.size(); i++) {
      double t = intensities.at(i);
      t = t / maxIntensity; 	// Normalize
      t = 0.97 * t;		// Keeps the peaks from extending to the limits of the plot
      t = 1.0 - t; 		// Simulate transmittance
      t *= 100.0;		// Convert to percent
      transmittances.push_back(t);
    }

    // Store in member vars
    m_xList->clear();
    m_yList->clear();
    for (uint i = 0; i < wavenumbers.size(); i++){
      m_xList->append(wavenumbers.at(i));
      m_yList->append(transmittances.at(i));
    }

    return true;
  }

  void RamanSpectra::setupPlot(PlotWidget * plot) {
    plot->setDefaultLimits( 4000.0, 400.0, 0.0, 100.0 );
    plot->axis(PlotWidget::BottomAxis)->setLabel(tr("Wavenumber (cm<sup>-1</sup>)"));
    plot->axis(PlotWidget::LeftAxis)->setLabel(m_yaxis);
  }

  QWidget * RamanSpectra::getTabWidget() {return m_tab_widget;}

  void RamanSpectra::getCalculatedPlotObject(PlotObject *plotObject) {
    plotObject->clearPoints();

    if (ui.spin_FWHM->value() != 0.0 && ui.cb_labelPeaks->isEnabled()) {
      ui.cb_labelPeaks->setEnabled(false);
      ui.cb_labelPeaks->setChecked(false);
    }
    if (ui.spin_FWHM->value() == 0.0 && !ui.cb_labelPeaks->isEnabled()) {
      ui.cb_labelPeaks->setEnabled(true);
    }
    if (!ui.cb_labelPeaks->isEnabled()) {
      ui.cb_labelPeaks->setChecked(false);
    }

    if (ui.spin_FWHM->value() == 0.0) { // get singlets
      plotObject->addPoint( 400, 100);

      for (int i = 0; i < m_yList->size(); i++) {
        double wavenumber = m_xList->at(i) * m_scale;
        double transmittance = m_yList->at(i);
        plotObject->addPoint ( wavenumber, 100 );
        if (ui.cb_labelPeaks->isChecked()) {
          // %L1 uses localized number format (e.g., 1.023,4 in Europe)
          plotObject->addPoint( wavenumber, transmittance, QString("%L1").arg(wavenumber, 0, 'f', 1) );
        }
        else {
          plotObject->addPoint( wavenumber, transmittance );
        }
        plotObject->addPoint( wavenumber, 100 );
      }
      plotObject->addPoint( 4000, 100);
    } // End singlets

    else { // Get gaussians
      // convert FWHM to sigma squared
      double FWHM = ui.spin_FWHM->value();
      double s2	= pow( (FWHM / (2.0 * sqrt(2.0 * log(2.0)))), 2.0);

      // create points
      QList<double> xPoints = getXPoints(FWHM, 10);
      for (int i = 0; i < xPoints.size(); i++) {
        double x = xPoints.at(i);
        double y = 100;
        for (int j = 0; j < m_yList->size(); j++) {
          double t = m_yList->at(j);
          double w = m_xList->at(j) * m_scale;
          y += (t-100) * exp( - ( pow( (x - w), 2 ) ) / (2 * s2) );
        }
        plotObject->addPoint(x,y);
      }

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
        // cur - min 		: Shift lowest point of plot to be at zero
        // 100 / (max - min)	: Conversion factor for current spread -> percent
        // * 0.97 + 3		: makes plot stay away from 0 transmittance
        //			: (easier to see multiple peaks on strong signals)
        plotObject->points().at(i)->setY( (cur - min) * 100 / (max - min) * 0.97 + 3);
      }
    } // End gaussians

    // Convert to absorbance?
    if (ui.combo_yaxis->currentText() == "Absorbance (%)") {
      for(int i = 0; i< plotObject->points().size(); i++) {
        double absorbance = 100 - plotObject->points().at(i)->y();
        plotObject->points().at(i)->setY(absorbance);
      }
    }
    return;
  } // End Raman spectra

  void RamanSpectra::setImportedData(const QList<double> & xList, const QList<double> & yList) {
    m_xList_imp = new QList<double> (xList);
    m_yList_imp = new QList<double> (yList);

    // Convert y values to percents from fraction, if necessary...
    bool convert = true;
    for (int i = 0; i < m_yList_imp->size(); i++) {
      if (m_yList_imp->at(i) > 1.5) { // If transmittances exist greater than this, they're already in percent.
        convert = false;
        break;
      }
    }
    if (convert) {
      for (int i = 0; i < m_yList->size(); i++) {
        double tmp = m_yList->at(i);
        tmp *= 100;
        m_yList->replace(i, tmp);
      }
    }
  }

  void RamanSpectra::getImportedPlotObject(PlotObject *plotObject) {
    plotObject->clearPoints();
    for (int i = 0; i < m_xList_imp->size(); i++)
      plotObject->addPoint(m_xList_imp->at(i), m_yList_imp->at(i));
  }

  QString RamanSpectra::getTSV() {
    QString str;
    QTextStream out (&str);
    QString format = "%1\t%2\n";
    out << "Frequencies\tIntensities\n";
    for(int i = 0; i< m_xList->size(); i++) {
      out << format.arg(m_xList->at(i), 0, 'g').arg(m_yList->at(i), 0, 'g');
    }
    return str;
  }

  void RamanSpectra::setScale(double scale) {
    qDebug() << scale << " " << m_scale;
    if (scale == m_scale) return;
    m_scale = scale;
    emit plotDataChanged();
  }

  void RamanSpectra::updateYAxis(QString text) {
    if (m_yaxis == text) {
      return;
    }
    m_dialog->getUi()->plot->axis(PlotWidget::LeftAxis)->setLabel(text);
    m_yaxis = text;
    emit plotDataChanged();
  }
}
