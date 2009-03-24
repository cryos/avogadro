/**********************************************************************
  VibrationPlot - Visualize vibrational modes graphically

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

#include "vibrationplot.h"

#include <QPushButton>
#include <QButtonGroup>
#include <QDebug>
#include <QDoubleValidator>

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
    ui.setupUi(this);
    
    //TODO link the scale here to vibrationdialog
    m_scale = 1.0;
    ui.scaleEdit->setText(QString::number(m_scale, 'f', 2));
    ui.scaleEdit->setValidator( new QDoubleValidator (0.5, 1.5, 2, ui.scaleEdit) );
    ui.scaleSlider->setSliderPosition( static_cast<int>((m_scale - 0.5) * 100) );

    // setting the limits for the plot
    ui.plot->setFontSize( 10);
    ui.plot->setLimits( 4000.0, 400.0, 0.0, 1.0 );
    ui.plot->setMinimumSize( 800, 500 );
    ui.plot->setAntialiasing(true);
    ui.plot->axis(PlotWidget::BottomAxis)->setLabel("Wavenumber (cm^(-1))");
    ui.plot->axis(PlotWidget::LeftAxis)->setLabel("Transmittance");

    connect(ui.scaleSlider, SIGNAL(valueChanged(int)),
            this, SLOT(setScale(int)));
    connect(this, SIGNAL(scaleUpdated()),
            this, SLOT(drawVibrationSpectra()));
    connect(this, SIGNAL(scaleUpdated()),
            this, SLOT(updateScaleEdit()));
    connect(ui.cb_labelPeaks, SIGNAL(toggled(bool)),
            this, SLOT(drawVibrationSpectra()));
  }

  VibrationPlot::~VibrationPlot()
  {
    //TODO Anything to delete?
  }

  void VibrationPlot::setMolecule(Molecule *molecule)
  {
    m_molecule = molecule;
    drawVibrationSpectra();
  }

  void VibrationPlot::setScale(int scale)
  {
    double newScale = scale / 100.0 + 0.5;
    if (newScale == m_scale) {
      return;
    }
    m_scale = newScale;
    emit scaleUpdated();
  }

  void VibrationPlot::setScale(double scale)
  {
    if (scale == m_scale) {
      return;
    }
    m_scale = scale;
    emit scaleUpdated();
  }
  
  void VibrationPlot::updateScaleEdit(){
    ui.scaleEdit->setText(QString::number(m_scale, 'f', 2));
  }
  
  void VibrationPlot::drawVibrationSpectra()
  {
    OBMol obmol = m_molecule->OBMol();
    m_vibrations = static_cast<OBVibrationData*>(obmol.GetData(OBGenericDataType::VibrationData));
    if (!m_vibrations) {
      qWarning() << "VibrationPlot::setMolecule: No vibrations to plot!";
      return;
    }

    // OK, we have valid vibrations, so plot them
    vector<double> frequencies = m_vibrations->GetFrequencies();
    vector<double> intensities = m_vibrations->GetIntensities();

    // FIXME: dlonie: remove this when OB is fixed!! Hack to get around bug in how open babel reads in QChem files
    // While openbabel is broken, remove indicies (n+3), where
    // n=0,1,2...
    if (frequencies.size() == 0.75 * intensities.size()) {
      uint count = 0;
      for (uint i = 0; i < intensities.size(); i++) {
        if ((i+count)%3 == 0){
          intensities.erase(intensities.begin()+i);
          count++;
          i--;
        }
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
    ui.plot->removeAllPlotObjects();
    m_vibrationPlotObject = new PlotObject( Qt::red, PlotObject::Lines, 2);
    m_vibrationPlotObject->clearPoints();
    m_vibrationPlotObject->addPoint( 400, 1); // Initial point

//     qDebug() << "size transmittances" << transmittances.size();
//     qDebug() << "size intensities   " << intensities.size();
//     qDebug() << "size frequencies   " << frequencies.size();
//     for (uint i = 0; i < transmittances.size(); i++) {
//       qDebug() << i << " " << transmittances.at(i);
//     }
//     for (uint i = 0; i < intensities.size(); i++) {
//       qDebug() << i << " " << intensities.at(i);
//     }
//     for (uint i = 0; i < transmittances.size(); i++) {
//       qDebug() << i << " " << frequencies.at(i);
//    }

    // For now, lets just make singlet peaks. Maybe we can fit a
    // gaussian later?
    for (uint i = 0; i < transmittances.size(); i++) {
      double wavenumber = frequencies.at(i) * m_scale;
      double transmittance = transmittances.at(i);
      m_vibrationPlotObject->addPoint ( wavenumber, 1 );
      if (ui.cb_labelPeaks->isChecked()) {
        m_vibrationPlotObject->addPoint ( wavenumber, transmittance, QString::number(wavenumber, 'f', 1));
      }
      else {
       	m_vibrationPlotObject->addPoint ( wavenumber, transmittance );
      }
      m_vibrationPlotObject->addPoint ( wavenumber, 1 );
    }

    m_vibrationPlotObject->addPoint( 4000, 1); // Final point
    ui.plot->addPlotObject(m_vibrationPlotObject);
    ui.plot->update();
  }
}

#include "vibrationplot.moc"
