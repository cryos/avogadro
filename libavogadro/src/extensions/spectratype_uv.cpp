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

#include "spectratype_uv.h"
#include "spectratype.h"
#include "spectradialog.h"

#include <QTextStream>
#include <QMessageBox>
#include <QDebug>

#include <openbabel/mol.h>
#include <openbabel/generic.h>

using namespace std;

namespace Avogadro {

  UVSpectra::UVSpectra( SpectraDialog *parent ) :
    SpectraType( parent ), m_dialog(parent)
  {
    m_tab_widget = new QWidget;
    ui.setupUi(m_tab_widget);

    m_xList = new QList<double>;
    m_yList = new QList<double>;
    m_xList_imp = new QList<double>;
    m_yList_imp = new QList<double>;

    m_dialog = parent;

    readSettings();
  }

  UVSpectra::~UVSpectra() {
    // TODO: Anything to delete?
    writeSettings();
  }

  void UVSpectra::writeSettings() {
    QSettings settings; // Already set up in avogadro/src/main.cpp
    settings.setValue("spectra/UV/gaussianWidth", ui.spin_FWHM->value());
    settings.setValue("spectra/UV/labelPeaks", ui.cb_labelPeaks->isChecked());
  }

  void UVSpectra::readSettings() {
    QSettings settings; // Already set up in avogadro/src/main.cpp
    ui.spin_FWHM->setValue(settings.value("spectra/UV/gaussianWidth",0.0).toDouble());
    ui.cb_labelPeaks->setChecked(settings.value("spectra/UV/labelPeaks",false).toBool());
  }

  bool UVSpectra::checkForData(Molecule * mol) {
    OpenBabel::OBMol obmol = mol->OBMol();
    OpenBabel::OBExcitedStatesData *esd = static_cast<OpenBabel::OBExcitedStatesData*>(obmol.GetData("ExcitedStatesData"));

    if (!esd) return false;
    if (esd->GetEDipole().size() == 0) return false;

    // Setup signals/slots
    connect(this, SIGNAL(plotDataChanged()),
            m_dialog, SLOT(regenerateCalculatedSpectra()));
    connect(ui.cb_labelPeaks, SIGNAL(toggled(bool)),
            this, SIGNAL(plotDataChanged()));
    connect(ui.spin_FWHM, SIGNAL(valueChanged(double)),
            this, SIGNAL(plotDataChanged()));

    // OK, we have valid data, so store them for later
    std::vector<double> wavelengths = esd->GetWavelengths();
    std::vector<double> edipole= esd->GetEDipole();

    // Store in member vars
    m_xList->clear();
    m_yList->clear();
    for (uint i = 0; i < wavelengths.size(); i++){
      m_xList->append(wavelengths.at(i));
      m_yList->append(edipole.at(i));
    }

    return true;
  }

  void UVSpectra::setupPlot(PlotWidget * plot) {
    plot->scaleLimits();
    plot->axis(PlotWidget::BottomAxis)->setLabel(tr("Wavelength (nm)"));
    plot->axis(PlotWidget::LeftAxis)->setLabel(tr("Intensity (arb. units)"));
  }

  QWidget * UVSpectra::getTabWidget() {return m_tab_widget;}

  void UVSpectra::getCalculatedPlotObject(PlotObject *plotObject) {
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

    if (m_xList->size() < 1 && m_yList->size() < 1) return;

    double wavelength, intensity, maxint;
    double FWHM = ui.spin_FWHM->value();
    bool use_widening = (FWHM == 0) ? false : true;

    maxint = m_yList->at(0);

    for (int i = 0; i < m_yList->size(); i++)
      if (m_yList->at(i) > maxint)
        maxint = m_yList->at(i);

    if (use_widening) {
      // convert FWHM to sigma squared
      double s2	= pow( (FWHM / (2.0 * sqrt(2.0 * log(2.0)))), 2.0);

      // determine range
      // - find maximum and minimum
      double min, max;
      min = m_xList->first();
      max = m_xList->last();

      for (int i = 0; i < m_xList->size(); i++) {
        double cur = m_xList->at(i);
        if (cur > max) max = cur;
        if (cur < min) min = cur;
      }
      min -= 2*FWHM;
      max += 2*FWHM;
      // - get resolution (TODO)
      double res = (FWHM/10.0 < 10.0) ? FWHM/10.0 : 10.0;
      if (res < 0.05) res = 0.05;
      // create points
      for (double x = min; x < max; x += res) {
        double y = 0.0;
        for (int i = 0; i < m_yList->size(); i++) {
          double t = m_yList->at(i);
          double w = m_xList->at(i);
          // 0.348 term is a normalization constant
          y += t * exp( - ( pow( (x - w), 2 ) ) / (2 * s2) ) / 0.348;
        }
        plotObject->addPoint(x,y);
      }
    }
    else {
      for (int i = 0; i < m_yList->size(); i++) {
        wavelength = m_xList->at(i);
        intensity = m_yList->at(i) / maxint;
        plotObject->addPoint ( wavelength, 0 );
        if (ui.cb_labelPeaks->isChecked()) {
          // %L1 uses localized number format (e.g., 1.023,4 in Europe)
          plotObject->addPoint( wavelength, intensity, QString("%L1").arg(wavelength, 0, 'f', 1) );
        } else {
        plotObject->addPoint ( wavelength, intensity );
        }
        plotObject->addPoint ( wavelength, 0 );
      }
    }
  } 

  void UVSpectra::setImportedData(const QList<double> & xList, const QList<double> & yList) {
    m_xList_imp = new QList<double> (xList);
    m_yList_imp = new QList<double> (yList);
  }

  void UVSpectra::getImportedPlotObject(PlotObject *plotObject) {
    plotObject->clearPoints();
    for (int i = 0; i < m_xList_imp->size(); i++)
      plotObject->addPoint(m_xList_imp->at(i), m_yList_imp->at(i));
  }

  QString UVSpectra::getTSV() {
    QString str;
    QTextStream out (&str);
    QString format = "%1\t%2\n";
    out << "Wavelength (nm)\tIntensity (arb)\n";
    for(int i = 0; i< m_xList->size(); i++) {
      out << format.arg(m_xList->at(i), 6, 'g').arg(m_yList->at(i), 6, 'g');
    }
    return str;
  }

}
