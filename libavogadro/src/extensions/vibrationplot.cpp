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

#include <QPen>
#include <QColor>
#include <QColorDialog>
#include <QButtonGroup>
#include <QDebug>
#include <QDoubleValidator>
#include <QFileDialog>
#include <QMessageBox>
#include <QFile>
#include <QDir>
#include <QPixmap>

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

    // Hide advanced options initially
    ui.gb_customize->hide();

    // setting the limits for the plot
    ui.plot->setFontSize( 10);
    ui.plot->setDefaultLimits( 4000.0, 400.0, 0.0, 1.0 );
    ui.plot->setAntialiasing(true);
    ui.plot->axis(PlotWidget::BottomAxis)->setLabel(tr("Wavenumber (cm^(-1))"));
    ui.plot->axis(PlotWidget::LeftAxis)->setLabel(tr("Transmittance"));
    m_calculatedSpectra = new PlotObject (Qt::red, PlotObject::Lines, 2);
    m_importedSpectra = new PlotObject (Qt::white, PlotObject::Lines, 2);
    m_nullSpectra = new PlotObject (Qt::white, PlotObject::Lines, 2); // Used to replace disabled plot objects
    ui.plot->addPlotObject(m_calculatedSpectra);
    ui.plot->addPlotObject(m_importedSpectra);

    connect(ui.push_colorBackground, SIGNAL(clicked()),
            this, SLOT(changeBackgroundColor()));
    connect(ui.push_colorForeground, SIGNAL(clicked()),
            this, SLOT(changeForegroundColor()));
    connect(ui.push_colorCalculated, SIGNAL(clicked()),
            this, SLOT(changeCalculatedSpectraColor()));
    connect(ui.push_colorImported, SIGNAL(clicked()),
            this, SLOT(changeImportedSpectraColor()));
    connect(ui.spin_fontSize, SIGNAL(valueChanged(int)),
            this, SLOT(changeFontSize(int)));
    connect(ui.push_customize, SIGNAL(clicked()),
            this, SLOT(toggleCustomize()));
    connect(ui.push_save, SIGNAL(clicked()),
            this, SLOT(saveImage()));
    connect(ui.cb_import, SIGNAL(toggled(bool)),
            this, SLOT(toggleImported(bool)));
    connect(ui.cb_calculate, SIGNAL(toggled(bool)),
            this, SLOT(toggleCalculated(bool)));
    connect(ui.cb_labelPeaks, SIGNAL(toggled(bool)),
            this, SLOT(regenerateCalculatedSpectra()));
    connect(ui.scaleSlider, SIGNAL(valueChanged(int)),
            this, SLOT(setScale(int)));
    connect(ui.push_import, SIGNAL(clicked()),
            this, SLOT(importSpectra()));
    connect(this, SIGNAL(scaleUpdated()),
            this, SLOT(regenerateCalculatedSpectra()));
    connect(this, SIGNAL(scaleUpdated()),
            this, SLOT(updateScaleEdit()));
  }

  VibrationPlot::~VibrationPlot()
  {
    //TODO: Anything to delete?
  }

  void VibrationPlot::changeBackgroundColor()
  {
    //TODO: Store color choices in config?
    QColor current (ui.plot->backgroundColor());
    QColor color = QColorDialog::getColor(current, this);//, tr("Select Background Color")); <-- Title not supported until Qt 4.5 bump.
    if (color.isValid() && color != current) {
      ui.plot->setBackgroundColor(color);
      updatePlot();
    }
  }

  void VibrationPlot::changeForegroundColor()
  {
    //TODO: Store color choices in config?
    QColor current (ui.plot->foregroundColor());
    QColor color = QColorDialog::getColor(current, this);//, tr("Select Foreground Color")); <-- Title not supported until Qt 4.5 bump.
    if (color.isValid() && color != current) {
      ui.plot->setForegroundColor(color);
      updatePlot();
    }
  }

  void VibrationPlot::changeCalculatedSpectraColor()
  {
    //TODO: Store color choices in config?
    QPen currentPen = m_calculatedSpectra->linePen();
    QColor current (currentPen.color());
    QColor color = QColorDialog::getColor(current, this);//, tr("Select Calculated Spectra Color")); <-- Title not supported until Qt 4.5 bump.
    if (color.isValid() && color != current) {
      currentPen.setColor(color);
      m_calculatedSpectra->setLinePen(currentPen);
      updatePlot();
    }
  }


  void VibrationPlot::changeImportedSpectraColor()
  {
    //TODO: Store color choices in config?
    QPen currentPen (m_importedSpectra->linePen());
    QColor current (currentPen.color());
    QColor color = QColorDialog::getColor(current, this);//, tr("Select Imported Spectra Color")); <-- Title not supported until Qt 4.5 bump.
    if (color.isValid() && color != current) {
      currentPen.setColor(color);
      m_importedSpectra->setLinePen(currentPen);
      updatePlot();
    }
  }

  void VibrationPlot::changeFontSize(int size)
  {
    //TODO: Need to be able to check the font settings of the plot
    ui.plot->setFontSize(size);
    updatePlot();
  }

  void VibrationPlot::setMolecule(Molecule *molecule)
  {
    m_molecule = molecule;
    OBMol obmol = m_molecule->OBMol();

    // Get intensities
    m_vibrations = static_cast<OBVibrationData*>(obmol.GetData(OBGenericDataType::VibrationData));
    if (!m_vibrations) {
      qWarning() << "VibrationPlot::setMolecule: No vibrations to plot!";
      return;
    }

    // OK, we have valid vibrations, so store them
    m_wavenumbers = m_vibrations->GetFrequencies();
    vector<double> intensities = m_vibrations->GetIntensities();

    // FIXME: dlonie: remove this when OB is fixed!! Hack to get around bug in how open babel reads in QChem files
    // While openbabel is broken, remove indicies (n+3), where
    // n=0,1,2...
    if (m_wavenumbers.size() == 0.75 * intensities.size()) {
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
      t = 0.97 * t;		// Keeps the peaks from extending to the limits of the plot
      t = 1 - t; 		// Simulate transmittance
      m_transmittances.push_back(t);
    }

    regenerateCalculatedSpectra();
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
  
  void VibrationPlot::importSpectra()
  {
    QFileInfo defaultFile(m_molecule->fileName());
    QString defaultPath = defaultFile.canonicalPath();
    if (defaultPath.isEmpty())
      defaultPath = QDir::homePath();

    QString defaultFileName = defaultPath + '/' + defaultFile.baseName() + ".tsv";
    QString filename 	= QFileDialog::getOpenFileName(this, tr("Import Spectra"), defaultFileName, tr("Tab Separated Values (*.tsv);;Text Files (*.txt);;All Files (*.*)"));

    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
      qDebug() << "Error reading file " << filename;
      return;
    }
    
    QTextStream in(&file);
    // Process each line
    while (!in.atEnd()) {
      QString line = in.readLine();

      // the following assumes that the file is a tsv of wavenumber \t transmittance
      QStringList data = line.split("\t");
      if (data.at(0).toDouble() && data.at(1).toDouble()) {
        m_imported_wavenumbers.push_back(data.at(0).toDouble());
        m_imported_transmittances.push_back(data.at(1).toDouble());
      }
      else {
        qDebug() << "VibrationPlot::importSpectra Skipping entry as invalid:\n\tWavenumber: " << data.at(0) << "\n\tTransmittance: " << data.at(1);
      }
    }
    ui.push_colorImported->setEnabled(true);
    ui.cb_import->setEnabled(true);
    ui.cb_import->setChecked(true);
    getImportedSpectra(m_importedSpectra);
    updatePlot();
  }

  void VibrationPlot::updateScaleEdit()
  {
    ui.scaleEdit->setText(QString::number(m_scale, 'f', 2));
  }

  void VibrationPlot::saveImage()
  {
    QFileInfo defaultFile(m_molecule->fileName());
    QString defaultPath = defaultFile.canonicalPath();
    if (defaultPath.isEmpty())
      defaultPath = QDir::homePath();

    QString defaultFileName = defaultPath + '/' + defaultFile.baseName() + ".png";
    QString filename 	= QFileDialog::getSaveFileName(this, tr("Save Spectra"), defaultFileName, tr("png (*.png);;jpg (*.jpg);;bmp (*.bmp);;tiff (*.tiff);;All Files (*.*)"));
    QPixmap pix = QPixmap::grabWidget(ui.plot);
    if (!pix.save(filename)) {
      qWarning() << "VibrationPlot::saveImage Error saving plot to " << filename;
    }
  }

  void VibrationPlot::toggleImported(bool state) {
    if (state) {
      ui.plot->replacePlotObject(1,m_importedSpectra);
    }
    else {
      ui.plot->replacePlotObject(1,m_nullSpectra);
    }
    updatePlot();
  }

  void VibrationPlot::toggleCalculated(bool state) {
    if (state) {
      ui.plot->replacePlotObject(0,m_importedSpectra);
    }
    else {
      ui.plot->replacePlotObject(0,m_calculatedSpectra);
    }
    updatePlot();
  }

  void VibrationPlot::toggleCustomize() {
    if (ui.gb_customize->isHidden()) {
      ui.push_customize->setText(tr("Customi&ze <<"));
      ui.gb_customize->show();
    }
    else {
      ui.push_customize->setText(tr("Customi&ze >>"));
      ui.gb_customize->hide();
    }
  }

  void VibrationPlot::regenerateCalculatedSpectra() {
    getCalculatedSpectra(m_calculatedSpectra);
    updatePlot();
  }

  void VibrationPlot::updatePlot()
  {
    ui.plot->update();
  }

  void VibrationPlot::getCalculatedSpectra(PlotObject *vibrationPlotObject)
  {
    vibrationPlotObject->clearPoints();
    vibrationPlotObject->addPoint( 400, 1); // Initial point

    // For now, lets just make singlet peaks. Maybe we can fit a
    // gaussian later?
    for (uint i = 0; i < m_transmittances.size(); i++) {
      double wavenumber = m_wavenumbers.at(i) * m_scale;
      double transmittance = m_transmittances.at(i);
      vibrationPlotObject->addPoint ( wavenumber, 1 );
      if (ui.cb_labelPeaks->isChecked()) {
        vibrationPlotObject->addPoint ( wavenumber, transmittance, QString::number(wavenumber, 'f', 1));
      }
      else {
       	vibrationPlotObject->addPoint ( wavenumber, transmittance );
      }
      vibrationPlotObject->addPoint ( wavenumber, 1 );
    }

    vibrationPlotObject->addPoint( 4000, 1); // Final point
  }

  void VibrationPlot::getImportedSpectra(PlotObject *vibrationPlotObject)
  {
    vibrationPlotObject->clearPoints();
    // For now, lets just make singlet peaks. Maybe we can fit a
    // gaussian later?
    for (uint i = 0; i < m_imported_transmittances.size(); i++) {
      double wavenumber = m_imported_wavenumbers.at(i);
      double transmittance = m_imported_transmittances.at(i);
      vibrationPlotObject->addPoint ( wavenumber, transmittance );
    }
  }
}

#include "vibrationplot.moc"
