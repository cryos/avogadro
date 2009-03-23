/**********************************************************************
  VibrationPlot - Visualize vibrational modes graphically

  Copyright (C) 2009 by David Lonie

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.openmolecules.net/>

  Some code is based on Open Babel
  For more information, see <http://openbabel.sourceforge.net/>

  The plotting interface is provided by a version of KPlotWidget that 
  is included with avogadro. The KPlotWidget, etc, files are copied 
  the kde tree.
  For more information, see <http://www.kde.org/>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation version 2 of the License.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
 ***********************************************************************/

#include "vibrationplot.h"

#include <QPushButton>
#include <QButtonGroup>
#include <QDebug>

#include <avogadro/molecule.h>
#include <avogadro/plotwidget.h>
#include <openbabel/mol.h>
#include <openbabel/generic.h>

using namespace OpenBabel;
using namespace std;

namespace Avogadro {

  VibrationPlot::VibrationPlot( QWidget *parent, Qt::WindowFlags f ) : 
    QDialog( parent, f )
  {
    qDebug("VibrationPlot: Constructor called");
    ui.setupUi(this);
    qDebug("Are we getting this far?");
    // setting the limits for the plot
    ui.plot->setLimits( 4000.0, 400.0, 0.0, 1.0 );
    ui.plot->axis(PlotWidget::BottomAxis)->setLabel("Wavenumber (cm^(-1))");
    ui.plot->axis(PlotWidget::LeftAxis)->setLabel("Transmittance");
    //TODO: Set system colors

    connect(ui.scaleSlider, SIGNAL(valueChanged(int)),
            this, SLOT(setScale(int)));
  }

  VibrationPlot::~VibrationPlot()
  {
    //TODO Anything to delete?
  }

  void VibrationPlot::setMolecule(Molecule *molecule)
  {
    qDebug("VibrationPlot: setMolecule called");
    m_molecule = molecule;

    OBMol obmol = molecule->OBMol();
    m_vibrations = static_cast<OBVibrationData*>(obmol.GetData(OBGenericDataType::VibrationData));
    if (!m_vibrations) {
      qWarning() << "No vibrations to plot!";
      return;
    }

    // OK, we have valid vibrations, so plot them
    vector<double> frequencies = m_vibrations->GetFrequencies();
    vector<double> intensities = m_vibrations->GetIntensities();

#warning: dlonie: remove this!!
    // While openbabel is broken, remove indicies (n+3), where
    // n=0,1,2...
    uint count = 0;
    for (uint i = 0; i < intensities.size(); i++) {
      if ((i+count)%3 == 0){
	intensities.erase(intensities.begin()+i);
	count++;
	i--;
      }
    }
    
    // Normalize intensities into transmittances
    double maxIntensity=0;
    vector<double> transmittances;
    for (unsigned int i = 0; i < intensities.size(); i++) {
      if (intensities.at(i) >= maxIntensity) {
	maxIntensity = intensities.at(i);
      }
    }

    if (maxIntensity == 0) {
      qWarning() << "VibrationPlot::setMolecule: No intensities > 0 in dataset.";
      return;
    }

    for (unsigned int i = 0; i < intensities.size(); i++) {
      double t = intensities.at(i);
      t = t / maxIntensity; 	// Normalize
      t = 1 - t; 		// Simulate transmittance
      transmittances.push_back(t);
    }

    // Construct plot data
    //TODO How to use system colors?    
    vibrationPlotObject = new PlotObject( Qt::red, PlotObject::Lines, 2);
    vibrationPlotObject->addPoint( 400, 1); // Initial point

    // For now, lets just make singlet peaks. Maybe we can fit a
    // gaussian later?
    qDebug() << "size transmittances" << transmittances.size();
    qDebug() << "size intensities   " << intensities.size();
    qDebug() << "size frequencies   " << frequencies.size();
    for (uint i = 0; i < transmittances.size(); i++) {
      qDebug() << i << " " << transmittances.at(i);
    }
    for (uint i = 0; i < intensities.size(); i++) {
      qDebug() << i << " " << intensities.at(i);
    }
    for (uint i = 0; i < transmittances.size(); i++) {
      qDebug() << i << " " << frequencies.at(i);
    }
    for (uint i = 0; i < transmittances.size(); i++) {
      double wavenumber = frequencies.at(i);
      double transmittance = transmittances.at(i);
      vibrationPlotObject->addPoint ( wavenumber, 1 );
      vibrationPlotObject->addPoint ( wavenumber, transmittance );
      vibrationPlotObject->addPoint ( wavenumber, 1 );
    }

    vibrationPlotObject->addPoint( 4000, 1); // Final point
    ui.plot->addPlotObject(vibrationPlotObject);
    ui.plot->update();

  }

  void VibrationPlot::accept()
  {
    qDebug("VibrationPlot: accept called");
    hide();
  }

  void VibrationPlot::reject()
  {
    qDebug("VibrationPlot: accept called");
    hide();
  }

  void VibrationPlot::setScale(int scale)
  {
    qDebug("VibrationPlot: setScale(int) called");
    emit scaleUpdated(scale / 2.0);
  }

}

#include "vibrationplot.moc"
