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

#include "spectratype_dos.h"
#include "spectratype.h"
#include "spectradialog.h"

#include <QTextStream>
#include <QMessageBox>
#include <QDebug>

#include <openbabel/mol.h>
#include <openbabel/generic.h>

using namespace std;

namespace Avogadro {

  DOSSpectra::DOSSpectra( SpectraDialog *parent ) :
    SpectraType( parent ), m_dialog(parent), m_intDOS(0)
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

  DOSSpectra::~DOSSpectra() {
    // TODO: Anything to delete?
    delete m_xList;
    delete m_yList;
    delete m_xList_imp;
    delete m_yList_imp;
    delete m_tab_widget;
    writeSettings();
  }

  void DOSSpectra::writeSettings() {
    QSettings settings; // Already set up in avogadro/src/main.cpp
    settings.setValue("spectra/DOS/zeroFermi", ui.cb_fermi->isChecked());
    settings.setValue("spectra/DOS/showIntegrated", ui.cb_toggleIntegrated->isChecked());
    settings.setValue("spectra/DOS/energyUnits", ui.combo_energy->currentIndex());
    settings.setValue("spectra/DOS/densityUnits", ui.combo_density->currentIndex());
  }

  void DOSSpectra::readSettings() {
    QSettings settings; // Already set up in avogadro/src/main.cpp
    ui.cb_fermi->setChecked(settings.value("spectra/DOS/zeroFermi", true).toBool());
    ui.cb_toggleIntegrated->setChecked(settings.value("spectra/DOS/showIntegrated", false).toBool());
    toggleIntegratedDOS(settings.value("spectra/DOS/showIntegrated", false).toBool());
    ui.combo_energy->setCurrentIndex(settings.value("spectra/DOS/energyUnits", ENERGY_EV).toInt());
    ui.combo_density->setCurrentIndex(settings.value("spectra/DOS/densityUnits", DENSITY_PER_CELL).toInt());
  }

  bool DOSSpectra::checkForData(Molecule * mol) {
    OpenBabel::OBMol obmol = mol->OBMol();
    //OpenBabel::OBDOSData *dos = static_cast<OpenBabel::OBDOSData*>(obmol.GetData(OpenBabel::OBGenericDataType::DOSData));
    OpenBabel::OBDOSData *dos = static_cast<OpenBabel::OBDOSData*>(obmol.GetData("DOSData"));
    qDebug() << dos;
    if (!dos) return false;

    // OK, we have valid DOS, so store them for later
    std::vector<double> energies = dos->GetEnergies();
    std::vector<double> densities= dos->GetDensities();
    if (m_intDOS) delete m_intDOS;
    m_intDOS = new std::vector<double> (dos->GetIntegration());

    if (energies.size() == 0 || energies.size() != densities.size())
      return false;

    // Setup signals/slots
    connect(this, SIGNAL(plotDataChanged()),
            m_dialog, SLOT(regenerateCalculatedSpectra()));
    connect(this, SIGNAL(plotDataChanged()),
            m_dialog, SLOT(regenerateImportedSpectra()));
    connect(ui.cb_toggleIntegrated, SIGNAL(toggled(bool)),
            this, SLOT(toggleIntegratedDOS(bool)));
    connect(ui.combo_energy, SIGNAL(currentIndexChanged(int)),
            this, SIGNAL(plotDataChanged()));
    connect(ui.combo_density, SIGNAL(currentIndexChanged(int)),
            this, SIGNAL(plotDataChanged()));
    connect(ui.cb_fermi, SIGNAL(toggled(bool)),
            this, SIGNAL(plotDataChanged()));

    // Store in member vars
    m_numAtoms = mol->numAtoms();
    m_fermi = dos->GetFermiEnergy();
    ui.label_fermi->setText(QString::number(m_fermi));
    m_xList->clear();
    m_yList->clear();
    bool generateInt = false;
    double d_max = densities.at(0);
    if (m_intDOS->size() == 0) generateInt = true;
    for (uint i = 0; i < energies.size(); i++){
      m_xList->append(energies.at(i));
      double d = densities.at(i);
      m_yList->append(d);
      if (d > d_max) d_max = d;
      if (generateInt) {
        if (i == 0)
          m_intDOS->push_back(d);
        else
          m_intDOS->push_back(m_intDOS->at(i-1) + d);
      }
    }

    // Get max integrated value (should be the last entry)
    double i_max = m_intDOS->at(m_intDOS->size() - 1);

    // Scale integrated DOS
    if (i_max != 0 && d_max != 0) {
      for (uint i = 0; i < m_intDOS->size(); i++)
        m_intDOS->at(i) = m_intDOS->at(i) / i_max * d_max;
    }

    setImportedData(*m_xList,
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
    }
  }

  QWidget * DOSSpectra::getTabWidget() {return m_tab_widget;}

  void DOSSpectra::getCalculatedPlotObject(PlotObject *plotObject) {
    plotObject->clearPoints();

    int energy_index = ui.combo_energy->currentIndex();
    int density_index = ui.combo_density->currentIndex();
    bool use_fermi = ui.cb_fermi->isChecked();
    double density, energy;

    for (int i = 0; i < m_yList->size(); i++) {
      switch (energy_index) {
      case ENERGY_EV:
        energy = m_xList->at(i);
        break;
      }
      switch (density_index) {
      case DENSITY_PER_CELL:
        density = m_yList->at(i);
        break;
      case DENSITY_PER_ATOM:
        density = m_yList->at(i) / ((float)m_numAtoms);
        break;
      }
      if (use_fermi) energy -= m_fermi;
      plotObject->addPoint ( energy, density );
    }
  } 

  void DOSSpectra::setImportedData(const QList<double> & xList, const QList<double> & yList) {
    m_xList_imp = new QList<double> (xList);
    m_yList_imp = new QList<double> (yList);
  }

  void DOSSpectra::getImportedPlotObject(PlotObject *plotObject) {
    plotObject->clearPoints();
    int energy_index = ui.combo_energy->currentIndex();
    int density_index = ui.combo_density->currentIndex();
    bool use_fermi = ui.cb_fermi->isChecked();
    double density, energy;

    for (int i = 0; i < m_yList_imp->size(); i++) {
      switch (energy_index) {
      case ENERGY_EV:
        energy = m_xList_imp->at(i);
        break;
      }
      switch (density_index) {
      case DENSITY_PER_CELL:
        density = m_yList_imp->at(i);
        break;
      case DENSITY_PER_ATOM:
        density = m_yList_imp->at(i) / ((float)m_numAtoms);
        break;
      }
      if (use_fermi) energy -= m_fermi;
      plotObject->addPoint ( energy, density );
    }
  }

  QString DOSSpectra::getTSV() {
    QString str;
    QTextStream out (&str);
    QString format = "%1\t%2\n";
    out << "Energy(eV)\tDensity(e/UC)\n";
    for(int i = 0; i< m_xList->size(); i++) {
      out << format.arg(m_xList->at(i), 6, 'g').arg(m_yList->at(i), 6, 'g');
    }
    return str;
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
