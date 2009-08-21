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

#include "spectratype_nmr.h"
#include "spectratype.h"
#include "spectradialog.h"

#include <QTextStream>
#include <QMessageBox>
#include <QDebug>

#include <openbabel/mol.h>
#include <openbabel/generic.h>
#include <openbabel/obiter.h>

using namespace std;
using namespace OpenBabel;

namespace Avogadro {

  NMRSpectra::NMRSpectra( SpectraDialog *parent ) :
    SpectraType( parent ), m_dialog(parent), m_NMRdata(0)
  {
    m_tab_widget = new QWidget;
    ui.setupUi(m_tab_widget);

    m_xList = new QList<double>;
    m_yList = new QList<double>;
    m_xList_imp = new QList<double>;
    m_yList_imp = new QList<double>;
    m_NMRdata = new QHash<QString, QList<double>* >;
    m_dialog = parent;

    readSettings();
  }

  NMRSpectra::~NMRSpectra() {
    // TODO: Anything to delete?
    writeSettings();
    delete m_xList;
    delete m_yList;
    delete m_xList_imp;
    delete m_yList_imp;
    delete m_tab_widget;
  }

  void NMRSpectra::writeSettings() {
    QSettings settings; // Already set up in avogadro/src/main.cpp
    settings.setValue("spectra/NMR/reference", m_ref);
    settings.setValue("spectra/NMR/gaussianWidth", ui.spin_FWHM->value());
    settings.setValue("spectra/NMR/labelPeaks", ui.cb_labelPeaks->isChecked());
  }

  void NMRSpectra::readSettings() {
    QSettings settings; // Already set up in avogadro/src/main.cpp
    setReference(settings.value("spectra/NMR/reference", 0.0).toDouble());
    ui.spin_FWHM->setValue(settings.value("spectra/NMR/gaussianWidth",0.0).toDouble());
    ui.cb_labelPeaks->setChecked(settings.value("spectra/NMR/labelPeaks",false).toBool());
  }

  QWidget * NMRSpectra::getTabWidget() {return m_tab_widget;}

  void NMRSpectra::getCalculatedPlotObject(PlotObject *plotObject) {
    plotObject->clearPoints();
    if (m_xList->isEmpty()) {
      qWarning() << "NMRSpectra::getCalculatedPlotObject: Empty xList? Refusing to plot.";
      return;
    }
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
      for (int i = 0; i < m_xList->size(); i++) {
        double shift = m_xList->at(i) - m_ref;
        //      double intensity = m_NMRintensities.at(i);
        plotObject->addPoint ( shift, 0);
        if (ui.cb_labelPeaks->isChecked()) {
          // %L1 uses localized number format (e.g., 10,23 in Europe)
          plotObject->addPoint( shift, 1.0 /* intensity */, QString("%L1").arg(shift, 0, 'f', 2));
        }
        else {
          plotObject->addPoint( shift, 1.0 /* intensity */ );
        }
        plotObject->addPoint( shift, 0 );
      }
    } // End singlets

    else { // Get gaussians
      // convert FWHM to sigma squared
      double FWHM = ui.spin_FWHM->value();
      double s2	= pow( (FWHM / (2.0 * sqrt(2.0 * log(2.0)))), 2.0);

      // create points
      QList<double> xPoints = getXPoints(FWHM, 10);
      for (int i = 0; i < xPoints.size(); i++) {
        double x = xPoints.at(i);
        double y = 0;
        for (int j = 0; j < m_xList->size(); j++) {
          double t = 1.0; //m_NMRintensities.at(i);
          double w = m_xList->at(j) - m_ref;
          y += t * exp( - ( pow( (x - w), 2 ) ) / (2 * s2) );
        }
        plotObject->addPoint(x,y);
      }

      // Normalization is probably screwed up, so renormalize the data
      max = plotObject->points().first()->y();
      min = max;
      for(int i = 0; i< plotObject->points().size(); i++) {
        double cur = plotObject->points().at(i)->y();
        if (cur < min) min = cur;
        if (cur > max) max = cur;
      }
      for(int i = 0; i< plotObject->points().size(); i++) {
        double cur = plotObject->points().at(i)->y();
        // cur - min 		: Shift lowest point of plot to be at zero
        // 1.0 / (max - min)	: Conversion factor for current spread -> fraction of 1
        // * 0.97			: makes plot stay away from 0 transmittance
        //			: (easier to see multiple peaks on strong signals)
        plotObject->points().at(i)->setY( (cur - min) * 1.0 / (max - min) * 0.97);
      }
    } // End gaussians
    updatePlotAxes();
  }

  void NMRSpectra::setImportedData(const QList<double> & xList, const QList<double> & yList) {
    m_xList_imp = new QList<double> (xList);
    m_yList_imp = new QList<double> (yList);

    // Normalize intensities
    double max = m_yList_imp->first();
    for (int i = 0; i < m_yList_imp->size(); i++) {
      if (m_yList_imp->at(i) > max) max = m_yList_imp->at(i);
    }
    for (int i = 0; i < m_yList_imp->size(); i++) {
      double tmp = m_yList_imp->at(i);
      tmp /= max;
      m_yList_imp->replace(i,tmp);
    }
  }

  void NMRSpectra::getImportedPlotObject(PlotObject *plotObject) {
    plotObject->clearPoints();
    for (int i = 0; i < m_xList_imp->size(); i++)
      plotObject->addPoint(m_xList_imp->at(i), m_yList_imp->at(i));
  }

  QString NMRSpectra::getTSV() {
    QString str;
    QTextStream out (&str);
    QString format = "%1\t%2\n";
    out << "Isotropic Shift\tIntensities\n";
    for(int i = 0; i< m_xList->size(); i++) {
      out << format.arg(m_xList->at(i), 0, 'g').arg(m_yList->at(i), 0, 'g');
    }
    return str;
  }

  bool NMRSpectra::checkForData(Molecule * mol) {
    OpenBabel::OBMol obmol = mol->OBMol();
    qDeleteAll(*m_NMRdata);
    m_NMRdata->clear();
    // Test for "NMR Isotropic Shift" in first atom
    bool hasNMR = false;
    if (obmol.NumAtoms() > 0)
      if (obmol.GetFirstAtom()->HasData("NMR Isotropic Shift"))
        hasNMR = true;

    if (!hasNMR) return false;
    // Setup signals/slots
    connect(this, SIGNAL(plotDataChanged()),
            m_dialog, SLOT(regenerateCalculatedSpectra()));
    connect(ui.combo_type, SIGNAL(currentIndexChanged(QString)),
            this, SLOT(setAtom(QString)));
    connect(ui.spin_ref, SIGNAL(valueChanged(double)),
            this, SLOT(setReference(double)));
    connect(ui.push_resetAxes, SIGNAL(clicked()),
            this, SLOT(updatePlotAxes()));
    connect(ui.spin_FWHM, SIGNAL(valueChanged(double)),
            m_dialog, SLOT(regenerateCalculatedSpectra()));
    connect(ui.cb_labelPeaks, SIGNAL(toggled(bool)),
            m_dialog, SLOT(regenerateCalculatedSpectra()));

    // Extract data from obmol
    FOR_ATOMS_OF_MOL(atom,obmol) {
      QString symbol 		= QString(OpenBabel::etab.GetSymbol(atom->GetAtomicNum()));
      double shift 		= QString(atom->GetData("NMR Isotropic Shift")->GetValue().c_str()).toFloat();
      QList<double> *list = new QList<double>;
      if (m_NMRdata->contains(symbol)) {
        list	= m_NMRdata->value(symbol);
      }
      else {
        // Dump symbol into NMR Type list
        ui.combo_type->addItem(symbol);
      }
      list->append(shift);
      m_NMRdata->insert(symbol, list);
    }
    return true;
  }

  void NMRSpectra::setAtom(QString symbol)
  {
    if (symbol.isEmpty()) symbol = ui.combo_type->currentText();
    if (!m_NMRdata->contains(symbol)) return;
    m_xList = m_NMRdata->value(symbol);
    updatePlotAxes();
    m_dialog->regenerateCalculatedSpectra();
  }

  void NMRSpectra::updatePlotAxes()
  {
    QList<double> tmp (*m_xList);
    qSort(tmp);
    double FWHM = ui.spin_FWHM->value();
    if (tmp.size() == 1) {
      double center 	= tmp.first() - m_ref;
      double ext	= 5 + FWHM;
      m_dialog->getUi()->plot->setDefaultLimits( center + ext, center - ext, 0.0, 1.0 );
    }
    else {
      double min = tmp.last() - m_ref;
      double max = tmp.first() - m_ref;
      double ext;
      if (fabs(min-max) < 0.1) { // If the spread of the peaks is less than 0.1, the nuclei are likely equivalent, so zoom out a bit.
        ext = 5;
      }
      else {
        ext = ( min - max ) * 0.1 + FWHM;
      }
      m_dialog->getUi()->plot->setDefaultLimits( min + ext, max - ext, 0.0, 1.0 );
    }
  }

  void NMRSpectra::setReference(double ref)
  {
    if (ref == m_ref) {
      return;
    }
    m_ref = ref;
    ui.spin_ref->setValue(ref);
    emit plotDataChanged();
  }

  void NMRSpectra::setupPlot(PlotWidget * plot) {
    plot->setDefaultLimits( 10.0, 0.0, 0.0, 1.0 );
    plot->axis(PlotWidget::BottomAxis)->setLabel(tr("Shift (ppm)"));
    plot->axis(PlotWidget::LeftAxis)->setLabel("");
  }
}
