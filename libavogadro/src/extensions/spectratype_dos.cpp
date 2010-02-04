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

#ifdef OPENBABEL_IS_NEWER_THAN_2_2_99

#include "spectratype_dos.h"
#include "spectratype.h"
#include "spectradialog.h"

#include <QtGui/QMessageBox>
#include <QtCore/QDebug>

#include <openbabel/mol.h>
#include <openbabel/generic.h>

using namespace std;

namespace Avogadro {

  DOSSpectra::DOSSpectra( SpectraDialog *parent ) :
    SpectraType( parent ), m_intDOS(0)
  {
    ui.setupUi(m_tab_widget);

    // Setup signals/slots
    connect(this, SIGNAL(plotDataChanged()),
            m_dialog, SLOT(regenerateCalculatedSpectra()));
    connect(this, SIGNAL(plotDataChanged()),
            m_dialog, SLOT(regenerateImportedSpectra()));
    connect(ui.cb_toggleIntegrated, SIGNAL(toggled(bool)),
            this, SLOT(toggleIntegratedDOS(bool)));
    connect(ui.cb_scaleIntegrated, SIGNAL(toggled(bool)),
            m_dialog, SLOT(regenerateImportedSpectra()));
    connect(ui.combo_energy, SIGNAL(currentIndexChanged(int)),
            this, SIGNAL(plotDataChanged()));
    connect(ui.combo_density, SIGNAL(currentIndexChanged(int)),
            this, SIGNAL(plotDataChanged()));
    connect(ui.cb_fermi, SIGNAL(toggled(bool)),
            this, SIGNAL(plotDataChanged()));
    connect(ui.spin_valence, SIGNAL(valueChanged(int)),
            this, SIGNAL(plotDataChanged()));

    readSettings();
  }

  DOSSpectra::~DOSSpectra() {
    writeSettings();
    // TODO: Anything to delete?
  }

  void DOSSpectra::writeSettings() {
    QSettings settings; // Already set up in avogadro/src/main.cpp
    settings.setValue("spectra/DOS/zeroFermi", ui.cb_fermi->isChecked());
    settings.setValue("spectra/DOS/showIntegrated", ui.cb_toggleIntegrated->isChecked());
    settings.setValue("spectra/DOS/scaleIntegrated", ui.cb_scaleIntegrated->isChecked());
    settings.setValue("spectra/DOS/energyUnits", ui.combo_energy->currentIndex());
    settings.setValue("spectra/DOS/densityUnits", ui.combo_density->currentIndex());
//qDebug() <<  ui.spin_valence->value();
    settings.setValue("spectra/DOS/valence", ui.spin_valence->value());
    
  }

  void DOSSpectra::readSettings() {
    QSettings settings; // Already set up in avogadro/src/main.cpp
    ui.cb_fermi->setChecked(settings.value("spectra/DOS/zeroFermi", true).toBool());
    ui.cb_toggleIntegrated->setChecked(settings.value("spectra/DOS/showIntegrated", true).toBool());
    ui.cb_scaleIntegrated->setChecked(settings.value("spectra/DOS/scaleIntegrated", false).toBool());
    ui.combo_energy->setCurrentIndex(settings.value("spectra/DOS/energyUnits", ENERGY_EV).toInt());
    ui.combo_density->setCurrentIndex(settings.value("spectra/DOS/densityUnits", DENSITY_PER_CELL).toInt());
    ui.spin_valence->setValue(settings.value("spectra/DOS/valence", 1).toInt());
  }

  bool DOSSpectra::checkForData(Molecule * mol) {
    OpenBabel::OBMol obmol = mol->OBMol();
    //OpenBabel::OBDOSData *dos = static_cast<OpenBabel::OBDOSData*>(obmol.GetData(OpenBabel::OBGenericDataType::DOSData));
    OpenBabel::OBDOSData *dos = static_cast<OpenBabel::OBDOSData*>(obmol.GetData("DOSData"));
    if (!dos) return false;

    // OK, we have valid DOS, so store them for later
    std::vector<double> energies = dos->GetEnergies();
    std::vector<double> densities= dos->GetDensities();
    if (m_intDOS) delete m_intDOS;
    m_intDOS = new std::vector<double> (dos->GetIntegration());

    if (energies.size() == 0 || energies.size() != densities.size())
      return false;

    // Store in member vars
    m_numAtoms = mol->numAtoms();
    m_fermi = dos->GetFermiEnergy();
    ui.label_fermi->setText(QString::number(m_fermi));
    m_xList.clear();
    m_yList.clear();
    bool generateInt = false;
    if (m_intDOS->size() == 0) generateInt = true;
    for (uint i = 0; i < energies.size(); i++){
      m_xList.append(energies.at(i));
      double d = densities.at(i);
      m_yList.append(d);
      if (generateInt) {
        if (i == 0)
          m_intDOS->push_back(d);
        else
          m_intDOS->push_back(m_intDOS->at(i-1) + d);
      }
    }

    setImportedData(m_xList,
                    QList<double>::fromVector(QVector<double>::fromStdVector(*m_intDOS)));

    return true;
  }

  void DOSSpectra::setupPlot(PlotWidget * plot) {
    plot->scaleLimits();
    switch (ui.combo_energy->currentIndex()) {
    case ENERGY_EV:
      plot->axis(PlotWidget::BottomAxis)->setLabel(tr("Energy (eV)"));
      break;
    default:
      break;
    }
    switch (ui.combo_density->currentIndex()) {
    case DENSITY_PER_CELL:
      plot->axis(PlotWidget::LeftAxis)->setLabel(tr("Density of States (states/cell)"));
      break;
    case DENSITY_PER_ATOM:
      plot->axis(PlotWidget::LeftAxis)->setLabel(tr("Density of States (states/atom)"));
      break;
    case DENSITY_PER_VALENCE:
      plot->axis(PlotWidget::LeftAxis)->setLabel(tr("Density of States (states/valence electron)"));
      break;
    }
  }

 // QWidget * DOSSpectra::getTabWidget() {return m_tab_widget;}

  void DOSSpectra::getCalculatedPlotObject(PlotObject *plotObject) {
    plotObject->clearPoints();

    int energy_index = ui.combo_energy->currentIndex();
    int density_index = ui.combo_density->currentIndex();
    bool use_fermi = ui.cb_fermi->isChecked();
    double density, energy;

    // Update GUI if needed
    double valence = 1;
    if (density_index == DENSITY_PER_VALENCE) {
      ui.spin_valence->setVisible(true);
      valence = ui.spin_valence->value();
    }
    else
      ui.spin_valence->setVisible(false);

    for (int i = 0; i < m_yList.size(); i++) {
      switch (energy_index) {
      case ENERGY_EV:
        energy = m_xList.at(i);
        break;
      }
      switch (density_index) {
      case DENSITY_PER_CELL:
        density = m_yList.at(i);
        break;
      case DENSITY_PER_ATOM:
        density = m_yList.at(i) / ((double)m_numAtoms);
        break;
      case DENSITY_PER_VALENCE:
        density = m_yList.at(i) / valence;
        break;
      }
      if (use_fermi) energy -= m_fermi;
      plotObject->addPoint ( energy, density );
    }
  } 

  /*void DOSSpectra::setImportedData(const QList<double> & xList, const QList<double> & yList) {
    m_xList_imp = new QList<double> (xList);
    m_yList_imp = new QList<double> (yList);
  }*/

  void DOSSpectra::getImportedPlotObject(PlotObject *plotObject) {
    plotObject->clearPoints();
    int energy_index = ui.combo_energy->currentIndex();
    int density_index = ui.combo_density->currentIndex();
    bool use_fermi = ui.cb_fermi->isChecked();
    double density, energy;

    // Scale to density max if requested
    double scale = 0; // leave at 0 if no scaling is to be done
    if (ui.cb_scaleIntegrated->isChecked()) {
      // Get scaling factors
      double d_max = m_yList.at(0);
      double i_max = m_intDOS->at(m_intDOS->size() - 1);
      for (int i = 0; i < m_yList.size(); i++){
        double d = m_yList.at(i);
        if (d > d_max) d_max = d;
      }

      if (i_max != 0 && d_max != 0)
        scale = d_max / i_max;
    }

    // Update GUI if needed
    double valence = 1;
    if (density_index == DENSITY_PER_VALENCE) {
      ui.spin_valence->setVisible(true);
      valence = ui.spin_valence->value();
    }
    else
      ui.spin_valence->setVisible(false);

    for (int i = 0; i < m_yList_imp.size(); i++) {
      switch (energy_index) {
      case ENERGY_EV:
        energy = m_xList_imp.at(i);
        break;
      }
      switch (density_index) {
      case DENSITY_PER_CELL:
        density = m_yList_imp.at(i);
        break;
      case DENSITY_PER_ATOM:
        density = m_yList_imp.at(i) / ((double)m_numAtoms);
        break;
      case DENSITY_PER_VALENCE:
        density = m_yList_imp.at(i) / valence;
        break;
      }
      if (use_fermi) energy -= m_fermi;
      if (scale != 0.0) density *= scale;
      plotObject->addPoint ( energy, density );
    }
  }

  QString DOSSpectra::getTSV() {
    /*QString str;
    QTextStream out (&str);
    QString format = "%1\t%2\n";
    out << "Energy(eV)\tDensity(e/UC)\n";
    for(int i = 0; i< m_xList.size(); i++) {
      out << format.arg(m_xList.at(i), 6, 'g').arg(m_yList.at(i), 6, 'g');
    }
    return str;*/
    return SpectraType::getTSV("Energy(eV)", "Density(e/UC)");
  }

  void DOSSpectra::toggleIntegratedDOS(bool b) {
    if (!b) {
      m_dialog->getUi()->cb_import->setChecked(false);
      return;
    }
    m_dialog->regenerateImportedSpectra();
    m_dialog->getUi()->cb_import->setChecked(true);
  }
}

#endif
