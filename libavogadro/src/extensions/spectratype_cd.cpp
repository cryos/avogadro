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

#include "spectratype_cd.h"
#include "spectratype.h"
#include "spectradialog.h"

#include <QTextStream>
#include <QMessageBox>
#include <QDebug>

#include <openbabel/mol.h>
#include <openbabel/generic.h>

using namespace std;

namespace Avogadro {

  CDSpectra::CDSpectra( SpectraDialog *parent ) :
    SpectraType( parent ), m_dialog(parent)
  {
    m_tab_widget = new QWidget;
    ui.setupUi(m_tab_widget);

    m_xList = new QList<double>;
    m_yList = 0; // will point to one of the ylist types below
    m_xList_imp = new QList<double>;
    m_yList_imp = new QList<double>;

    m_yListVelocity = new QList<double>;
    m_yListLength = new QList<double>;

    m_dialog = parent;

    // Setup signals/slots
    connect(this, SIGNAL(plotDataChanged()),
            m_dialog, SLOT(regenerateCalculatedSpectra()));
    connect(ui.cb_labelPeaks, SIGNAL(toggled(bool)),
            this, SIGNAL(plotDataChanged()));
    connect(ui.spin_FWHM, SIGNAL(valueChanged(double)),
            this, SIGNAL(plotDataChanged()));
    connect(ui.combo_rotatoryType, SIGNAL(currentIndexChanged(QString)),
            this, SLOT(rotatoryTypeChanged(QString)));

    readSettings();
  }

  CDSpectra::~CDSpectra() {
    // TODO: Anything to delete?
    delete m_xList;
    delete m_yList;
    delete m_xList_imp;
    delete m_yList_imp;
    delete m_tab_widget;
    writeSettings();
  }

  void CDSpectra::writeSettings() {
    QSettings settings; // Already set up in avogadro/src/main.cpp
    settings.setValue("spectra/CD/gaussianWidth", ui.spin_FWHM->value());
    settings.setValue("spectra/CD/labelPeaks", ui.cb_labelPeaks->isChecked());
  }

  void CDSpectra::readSettings() {
    QSettings settings; // Already set up in avogadro/src/main.cpp
    ui.spin_FWHM->setValue(settings.value("spectra/CD/gaussianWidth",0.0).toDouble());
    ui.cb_labelPeaks->setChecked(settings.value("spectra/CD/labelPeaks",false).toBool());
  }

  bool CDSpectra::checkForData(Molecule * mol) {
    OpenBabel::OBMol obmol = mol->OBMol();
    OpenBabel::OBElectronicTransitionData *etd = static_cast<OpenBabel::OBElectronicTransitionData*>(obmol.GetData("ElectronicTransitionData"));

    if (!etd) return false;
    if ( etd->GetRotatoryStrengthsVelocity().size() == 0 &&
         etd->GetRotatoryStrengthsLength().size() == 0 ) return false;

    // OK, we have valid data, so store them for later
    std::vector<double> wavelengths = etd->GetWavelengths();
    std::vector<double> rotl = etd->GetRotatoryStrengthsLength();
    std::vector<double> rotv = etd->GetRotatoryStrengthsVelocity();

    ui.combo_rotatoryType->clear();
    if (rotl.size() != 0) ui.combo_rotatoryType->addItem("Length");
    if (rotv.size() != 0) ui.combo_rotatoryType->addItem("Velocity");

    // Store in member vars
    m_xList->clear();
    m_yList->clear();
    for (uint i = 0; i < wavelengths.size(); i++)
      m_xList->append(wavelengths.at(i));
    for (uint i = 0; i < rotl.size(); i++)
      m_yListLength->append(rotl.at(i));
    for (uint i = 0; i < rotv.size(); i++)
      m_yListVelocity->append(rotv.at(i));


    rotatoryTypeChanged(ui.combo_rotatoryType->currentText());

    return true;
  }

  void CDSpectra::setupPlot(PlotWidget * plot) {
    plot->scaleLimits();
    plot->axis(PlotWidget::BottomAxis)->setLabel(tr("Wavelength (nm)"));
    plot->axis(PlotWidget::LeftAxis)->setLabel(tr("Intensity (arb. units)"));
  }

  QWidget * CDSpectra::getTabWidget() {return m_tab_widget;}

  void CDSpectra::getCalculatedPlotObject(PlotObject *plotObject) {
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

    double wavelength, intensity;
    double FWHM = ui.spin_FWHM->value();
    bool use_widening = (FWHM == 0) ? false : true;

    if (use_widening) {
      // convert FWHM to sigma squared
      double s2	= pow( (FWHM / (2.0 * sqrt(2.0 * log(2.0)))), 2.0);

      // create points
      QList<double> xPoints = getXPoints(FWHM, 25);
      for (int i = 0; i < xPoints.size(); i++) {
        double x = xPoints.at(i);
        double y = 0.0;
        for (int j = 0; j < m_yList->size(); j++) {
          double t = m_yList->at(j);
          double w = m_xList->at(j);
          y += t * exp( - ( pow( (x - w), 2 ) ) / (2 * s2) ) / 
            (22.97 * x / 1241) // <-- normalization constant (22.97 / X_0)
            / sqrt(2 * M_PI * s2); // <-- gaussian normalization
        }
        plotObject->addPoint(x,y);
      }
    }
    else {
      for (int i = 0; i < m_yList->size(); i++) {
        wavelength = m_xList->at(i);
        intensity = m_yList->at(i) /
          (22.97 * wavelength / 1241) ; // <-- normalization constant (22.97 / X_0)
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

  void CDSpectra::setImportedData(const QList<double> & xList, const QList<double> & yList) {
    m_xList_imp = new QList<double> (xList);
    m_yList_imp = new QList<double> (yList);
  }

  void CDSpectra::getImportedPlotObject(PlotObject *plotObject) {
    plotObject->clearPoints();
    for (int i = 0; i < m_xList_imp->size(); i++)
      plotObject->addPoint(m_xList_imp->at(i), m_yList_imp->at(i));
  }

  QString CDSpectra::getTSV() {
    QString str;
    QTextStream out (&str);
    QString format = "%1\t%2\n";
    out << "Wavelength (nm)\tIntensity (arb)\n";
    for(int i = 0; i< m_xList->size(); i++) {
      out << format.arg(m_xList->at(i), 6, 'g').arg(m_yList->at(i), 6, 'g');
    }
    return str;
  }

  void CDSpectra::rotatoryTypeChanged(const QString & str) {
    if (str == "Velocity")
      m_yList = m_yListVelocity;
    else if (str == "Length")
      m_yList = m_yListLength;
    emit plotDataChanged();
  }

}
