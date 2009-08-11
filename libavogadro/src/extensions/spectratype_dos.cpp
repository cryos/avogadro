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

   DOSSpectra::~DOSSpectra() {
     // TODO: Anything to delete?
     writeSettings();
   }

  void DOSSpectra::writeSettings() {
    QSettings settings; // Already set up in avogadro/src/main.cpp
  }

  void DOSSpectra::readSettings() {
    QSettings settings; // Already set up in avogadro/src/main.cpp
  }

  bool DOSSpectra::checkForData(Molecule * mol) {
    OpenBabel::OBMol obmol = mol->OBMol();
    //OpenBabel::OBDOSData *dos = static_cast<OpenBabel::OBDOSData*>(obmol.GetData(OpenBabel::OBGenericDataType::DOSData));
    OpenBabel::OBDOSData *dos = static_cast<OpenBabel::OBDOSData*>(obmol.GetData("DOSData"));
    qDebug() << dos;
    if (!dos) return false;

    // Setup signals/slots
    connect(this, SIGNAL(plotDataChanged()),
            m_dialog, SLOT(regenerateCalculatedSpectra()));

    // OK, we have valid DOS, so store them for later
    double fermi = dos->GetFermiEnergy();
    std::vector<double> energies = dos->GetEnergies();
    std::vector<double> densities= dos->GetDensities();

    // Shift energies so that fermi energy is at 0.0
    for (unsigned int i = 0; i < energies.size(); i++) {
      energies[i] -= fermi;
    }

    // Store in member vars
    m_xList->clear();
    m_yList->clear();
    for (uint i = 0; i < energies.size(); i++){
      m_xList->append(energies.at(i));
      m_yList->append(densities.at(i));
    }

    return true;
  }

  void DOSSpectra::setupPlot(PlotWidget * plot) {
    plot->scaleLimits();
    plot->axis(PlotWidget::BottomAxis)->setLabel(tr("Energy (eV)"));
    plot->axis(PlotWidget::LeftAxis)->setLabel(tr("Density of States (states/cell)"));
  }

  QWidget * DOSSpectra::getTabWidget() {return m_tab_widget;}

  void DOSSpectra::getCalculatedPlotObject(PlotObject *plotObject) {
    plotObject->clearPoints();

    for (int i = 0; i < m_yList->size(); i++) {
      double energy = m_xList->at(i);
      double density = m_yList->at(i);
      plotObject->addPoint ( energy, density );
    }
  } 

  void DOSSpectra::setImportedData(const QList<double> & xList, const QList<double> & yList) {
    m_xList_imp = new QList<double> (xList);
    m_yList_imp = new QList<double> (yList);
  }

  void DOSSpectra::getImportedPlotObject(PlotObject *plotObject) {
    plotObject->clearPoints();
    for (int i = 0; i < m_xList_imp->size(); i++)
      plotObject->addPoint(m_xList_imp->at(i), m_yList_imp->at(i));
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

}
