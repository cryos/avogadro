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

#include "spectradialog.h"

#include <QPen>
#include <QColor>
#include <QColorDialog>
#include <QButtonGroup>
#include <QDebug>
#include <QDoubleValidator>
#include <QFileDialog>
#include <QFontDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QFile>
#include <QDir>
#include <QPixmap>
#include <QSettings>
#include <QListWidgetItem>

#include <avogadro/molecule.h>
#include <avogadro/plotwidget.h>
#include <openbabel/mol.h>
#include <openbabel/obiter.h>
#include <openbabel/generic.h>

using namespace OpenBabel;
using namespace std;

namespace Avogadro {

  SpectraDialog::SpectraDialog( QWidget *parent, Qt::WindowFlags f ) :
      QDialog( parent, f )
  {
    ui.setupUi(this);

    // Initialize vars
    m_IR_yaxis = ui.combo_IR_yaxis->currentText();
    m_schemes = new QList<QHash<QString, QVariant> >;

    // Hide advanced options initially
    ui.tab_widget->hide();

    // setting the limits for the plot
    ui.plot->setAntialiasing(true);
    ui.plot->setDefaultLimits( 4000.0, 400.0, 0.0, 100.0 );
    ui.plot->axis(PlotWidget::BottomAxis)->setLabel(tr("X Axis"));
    ui.plot->axis(PlotWidget::LeftAxis)->setLabel(tr("Y Axis"));
    m_calculatedSpectra = new PlotObject (Qt::red, PlotObject::Lines, 2);
    m_importedSpectra = new PlotObject (Qt::white, PlotObject::Lines, 2);
    m_nullSpectra = new PlotObject (Qt::white, PlotObject::Lines, 2); // Used to replace disabled plot objects
    ui.plot->addPlotObject(m_calculatedSpectra);
    ui.plot->addPlotObject(m_importedSpectra);

    // Scheme connections
    connect(ui.list_schemes, SIGNAL(currentRowChanged(int)),
            this, SLOT(updateScheme(int)));
    connect(ui.push_newScheme, SIGNAL(clicked()),
            this, SLOT(addScheme()));
    connect(ui.push_renameScheme, SIGNAL(clicked()),
            this, SLOT(renameScheme()));
    connect(ui.push_removeScheme, SIGNAL(clicked()),
            this, SLOT(removeScheme()));
    connect(ui.push_colorBackground, SIGNAL(clicked()),
            this, SLOT(changeBackgroundColor()));
    connect(ui.push_colorForeground, SIGNAL(clicked()),
            this, SLOT(changeForegroundColor()));
    connect(ui.push_colorCalculated, SIGNAL(clicked()),
            this, SLOT(changeCalculatedSpectraColor()));
    connect(ui.push_colorImported, SIGNAL(clicked()),
            this, SLOT(changeImportedSpectraColor()));
    connect(ui.push_font, SIGNAL(clicked()),
            this, SLOT(changeFont()));

    // Image export connections
    connect(ui.push_imageSave, SIGNAL(clicked()),
            this, SLOT(saveImage()));
    connect(ui.push_imageFilename, SIGNAL(clicked()),
            this, SLOT(saveImageFileDialog()));

    // Plot connections
    connect(ui.cb_import, SIGNAL(toggled(bool)),
            this, SLOT(toggleImported(bool)));
    connect(ui.cb_calculate, SIGNAL(toggled(bool)),
            this, SLOT(toggleCalculated(bool)));
    connect(ui.push_import, SIGNAL(clicked()),
            this, SLOT(importSpectra()));
    connect(ui.push_export, SIGNAL(clicked()),
            this, SLOT(exportSpectra()));

    // Misc. connections
    connect(ui.combo_spectra, SIGNAL(currentIndexChanged(QString)),
            this, SLOT(updateCurrentSpectra(QString)));
    connect(ui.push_customize, SIGNAL(clicked()),
            this, SLOT(toggleCustomize()));
    connect(ui.push_loadSpectra, SIGNAL(clicked()),
            this, SLOT(loadSpectra()));

    readSettings();
  }

  SpectraDialog::~SpectraDialog()
  {
    writeSettings();
  }

  void SpectraDialog::setMolecule(Molecule *molecule)
  {
    if (m_molecule == molecule || !molecule) {
      return;
    }
    m_molecule = molecule;
    OBMol obmol = m_molecule->OBMol();

    // set the filename in the image export widget
    QFileInfo defaultFile(m_molecule->fileName());
    QString defaultPath = defaultFile.canonicalPath();
    if (defaultPath.isEmpty()) {
      defaultPath = QDir::homePath();
    }
    ui.edit_imageFilename->setText(defaultPath + '/' + defaultFile.baseName() + ".png");

    // Empty the tab widget and spectra combo box when the molecule changes,
    // only adding in the entries appropriate to the molecule as needed
    ui.combo_spectra->clear();
    ui.tab_widget->clear();
    ui.tab_widget->addTab(ui.tab_appearance, tr("&Appearance"));
    ui.tab_widget->addTab(ui.tab_imageExport, tr("E&xport Image"));

    // Check for IR data
    OpenBabel::OBVibrationData *vibrations = static_cast<OBVibrationData*>(obmol.GetData(OBGenericDataType::VibrationData));
    if (vibrations) {
      // Setup GUI
      ui.combo_spectra->addItem(tr("Infrared", "Infrared spectra option"));
      ui.tab_widget->addTab(ui.tab_infrared, tr("&Infrared Spectra Settings"));

      // Setup signals/slots
      connect(this, SIGNAL(scaleUpdated_IR()),
              this, SLOT(regenerateCalculatedSpectra()));
      connect(ui.cb_IR_labelPeaks, SIGNAL(toggled(bool)),
              this, SLOT(regenerateCalculatedSpectra()));
      connect(ui.spin_IR_scale, SIGNAL(valueChanged(double)),
              this, SLOT(setScale_IR(double)));
      connect(ui.spin_IR_FWHM, SIGNAL(valueChanged(double)),
              this, SLOT(regenerateCalculatedSpectra()));
      connect(ui.combo_IR_yaxis, SIGNAL(currentIndexChanged(QString)),
              this, SLOT(updateYAxis_IR(QString)));

      // OK, we have valid vibrations, so store them for later
      m_IRwavenumbers = vibrations->GetFrequencies();
      vector<double> intensities = vibrations->GetIntensities();

      // Case where there are no intensities, set all intensities to an arbitrary value, i.e. 1.0
      if (m_IRwavenumbers.size() > 0 && intensities.size() == 0) {
        // Warn user
        QMessageBox::information(this, tr("No intensities"), tr("The vibration data in the molecule you have loaded does not have any intensity data. Intensities have been set to an arbitrary value for visualization."));
        for (uint i = 0; i < m_IRwavenumbers.size(); i++) {
          intensities.push_back(1.0);
        }
      }

      ////////////////////////////////////////////////////////////
      // FIXME: dlonie: remove this when OB is fixed!! Hack to get
      // around bug in how open babel reads in QChem files.
      // While openbabel is broken, remove indicies (n+3), where
      // n=0,1,2...
      if (m_IRwavenumbers.size() == 0.75 * intensities.size()) {
        uint count = 0;
        for (uint i = 0; i < intensities.size(); i++) {
          if ((i+count)%3 == 0){
            intensities.erase(intensities.begin()+i);
            count++;
            i--;
          }
        }
      }
      ///////////////////////////////////////////////////////////

      // Normalize intensities into transmittances
      double maxIntensity=0;
      for (unsigned int i = 0; i < intensities.size(); i++) {
        if (intensities.at(i) >= maxIntensity) {
          maxIntensity = intensities.at(i);
        }
      }

      // Clear out any old transmittance data
      m_IRtransmittances.clear();

      for (unsigned int i = 0; i < intensities.size(); i++) {
        double t = intensities.at(i);
        t = t / maxIntensity; 	// Normalize
        t = 0.97 * t;		// Keeps the peaks from extending to the limits of the plot
        t = 1.0 - t; 		// Simulate transmittance
        t *= 100.0;		// Convert to percent
        m_IRtransmittances.push_back(t);
      }
    }

    // Check for NMR data
    // First remove any old data
    m_NMRdata = new QHash<QString, QList<double> >;
    // Test for "NMR Isotropic Shift" in first atom
    bool hasNMR = false;
    if (obmol.NumAtoms() > 0)
      if (obmol.GetFirstAtom()->HasData("NMR Isotropic Shift"))
        hasNMR = true;

    if (hasNMR) {
      qDebug() << "NMR data found:";
      // Setup GUI
      ui.combo_spectra->addItem(tr("NMR", "NMR spectra option"));
      ui.tab_widget->addTab(ui.tab_NMR, tr("&NMR Spectra Settings"));

      // Setup signals/slots
      connect(this, SIGNAL(refUpdated_NMR()),
              this, SLOT(regenerateCalculatedSpectra()));
      connect(ui.combo_NMR_type, SIGNAL(currentIndexChanged(QString)),
              this, SLOT(setNMRAtom(QString)));
      connect(ui.spin_NMR_ref, SIGNAL(valueChanged(double)),
              this, SLOT(setReference_NMR(double)));
      connect(ui.push_NMR_resetAxes, SIGNAL(clicked()),
              this, SLOT(updatePlotAxes_NMR()));
      connect(ui.spin_NMR_FWHM, SIGNAL(valueChanged(double)),
              this, SLOT(regenerateCalculatedSpectra()));
      connect(ui.cb_NMR_labelPeaks, SIGNAL(toggled(bool)),
              this, SLOT(regenerateCalculatedSpectra()));

      // Extract data from obmol
      FOR_ATOMS_OF_MOL(atom,obmol) {
        QString symbol 		= QString(OpenBabel::etab.GetSymbol(atom->GetAtomicNum()));
        double shift 		= QString(atom->GetData("NMR Isotropic Shift")->GetValue().c_str()).toFloat();
        QList<double> list;
        if (m_NMRdata->contains(symbol)) {
          list	= m_NMRdata->value(symbol);
        }
        else {
          // Dump symbol into NMR Type list
          ui.combo_NMR_type->addItem(symbol);
        }
        list.append(shift);
        m_NMRdata->insert(symbol, list);
      }
      qDebug() << *m_NMRdata;
      setNMRAtom(ui.combo_NMR_type->currentText());
    } else { qDebug() << "No NMR data found..."; }

    // Change this when other spectra are added!!
    if (!vibrations && !hasNMR) { // Actions if there are no spectra loaded
      qWarning() << "SpectraDialog::setMolecule: No spectra available!";
      ui.combo_spectra->addItem(tr("No data"));
      ui.push_colorCalculated->setEnabled(false);
      ui.cb_calculate->setEnabled(false);
      ui.cb_calculate->setChecked(false);
      return;
    } else { // Actions for all spectra
      ui.push_colorCalculated->setEnabled(true);
      ui.cb_calculate->setEnabled(true);
      ui.cb_calculate->setChecked(true);
    }
    // Set the appearances tab to be opened by default
    ui.tab_widget->setCurrentIndex(0);
    m_spectra = ui.combo_spectra->currentText();
    regenerateCalculatedSpectra();
  }

  void SpectraDialog::writeSettings() const {
    QSettings settings; // Already set up in avogadro/src/main.cpp

    settings.setValue("spectra/IR/scale", m_IR_scale);
    settings.setValue("spectra/IR/gaussianWidth", ui.spin_IR_FWHM->value());
    settings.setValue("spectra/IR/labelPeaks", ui.cb_IR_labelPeaks->isChecked());

    settings.setValue("spectra/NMR/reference", m_NMR_ref);
    settings.setValue("spectra/NMR/gaussianWidth", ui.spin_NMR_FWHM->value());
    settings.setValue("spectra/NMR/labelPeaks", ui.cb_NMR_labelPeaks->isChecked());

    settings.setValue("spectra/image/width", ui.spin_imageWidth->value());
    settings.setValue("spectra/image/height", ui.spin_imageHeight->value());
    settings.setValue("spectra/image/units", ui.combo_imageUnits->currentIndex());
    settings.setValue("spectra/image/DPI", ui.spin_imageDPI->value());
    settings.setValue("spectra/image/optimizeFontSize", ui.cb_imageFontAdjust->isChecked());

    settings.setValue("spectra/currentScheme", m_scheme);
    settings.beginWriteArray("spectra/schemes");
    for (int i = 0; i < m_schemes->size(); ++i) {
      settings.setArrayIndex(i);
      ////////////////////////////////////////////////////////////////
      // FIXME: When we bump to Qt 4.5, change the following
      //      settings.setValue("scheme", m_schemes->at(i));
      settings.beginGroup("hash");
      QHashIterator<QString, QVariant> iter(m_schemes->at(i));
      while (iter.hasNext()) {
          iter.next();
          settings.setValue(iter.key(), iter.value());
        }
      settings.endGroup();
      ////////////////////////////////////////////////////////////////
    }
    settings.endArray();
  }

  void SpectraDialog::readSettings() {
    QSettings settings; // Already set up in avogadro/src/main.cpp
    setScale_IR(settings.value("spectra/IR/scale", 1.0).toDouble());
    ui.spin_IR_FWHM->setValue(settings.value("spectra/IR/gaussianWidth",0.0).toDouble());
    ui.cb_IR_labelPeaks->setChecked(settings.value("spectra/IR/labelPeaks",false).toBool());

    setReference_NMR(settings.value("spectra/NMR/reference", 0.0).toDouble());
    ui.spin_NMR_FWHM->setValue(settings.value("spectra/NMR/gaussianWidth",0.0).toDouble());
    ui.cb_NMR_labelPeaks->setChecked(settings.value("spectra/NMR/labelPeaks",false).toBool());

    ui.spin_imageWidth->setValue(settings.value("spectra/image/width", 21).toInt());
    ui.spin_imageHeight->setValue(settings.value("spectra/image/height", 10).toInt());
    ui.combo_imageUnits->setCurrentIndex(settings.value("spectra/image/units", 0).toInt());
    ui.spin_imageDPI->setValue(settings.value("spectra/image/DPI", 150).toInt());
    ui.cb_imageFontAdjust->setChecked(settings.value("spectra/image/optimizeFontSize", true).toBool());

    int scheme = settings.value("spectra/currentScheme", 0).toInt();
    int size = settings.beginReadArray("spectra/schemes");
    m_schemes = new QList<QHash<QString, QVariant> >;
    for (int i = 0; i < size; ++i) {
      settings.setArrayIndex(i);
      ////////////////////////////////////////////////////////////////
      // FIXME: QVariant::toHash() isn't around until Qt 4.5
      //      m_schemes->append(settings.value("scheme").toHash());
      settings.beginGroup("hash");
      QHash<QString, QVariant> hash;
      QStringList keys = settings.allKeys();
      foreach (const QString &key, settings.allKeys())
        hash[key] = settings.value(key);
      m_schemes->append(hash);
      settings.endGroup();
      ////////////////////////////////////////////////////////////////
      new QListWidgetItem(m_schemes->at(i)["name"].toString(), ui.list_schemes);
    }
    settings.endArray();

    // create scheme list if it doesn't already exist
    if (m_schemes->isEmpty()) {
      // dark
      QHash<QString, QVariant> dark;
      dark["name"] = tr("Dark");
      dark["backgroundColor"] = Qt::black;
      dark["foregroundColor"] = Qt::white;
      dark["calculatedColor"] = Qt::red;
      dark["importedColor"] = Qt::gray;
      dark["font"] = QFont();
      new QListWidgetItem(dark["name"].toString(), ui.list_schemes);
      m_schemes->append(dark);

      // light
      QHash<QString, QVariant> light;
      light["name"] = tr("Light");
      light["backgroundColor"] = Qt::white;
      light["foregroundColor"] = Qt::black;
      light["calculatedColor"] = Qt::red;
      light["importedColor"] = Qt::gray;
      light["font"] = QFont();
      new QListWidgetItem(light["name"].toString(), ui.list_schemes);
      m_schemes->append(light);

      // publication
      QHash<QString, QVariant> publication;
      publication["name"] = tr("Publication");
      publication["backgroundColor"] = Qt::white;
      publication["foregroundColor"] = Qt::black;
      publication["calculatedColor"] = Qt::black;
      publication["importedColor"] = Qt::gray;
      publication["font"] = QFont("Century Schoolbook L", 13);
      new QListWidgetItem(publication["name"].toString(), ui.list_schemes);
      m_schemes->append(publication);

      // handdrawn
      QHash<QString, QVariant> handdrawn;
      handdrawn["name"] = tr("Handdrawn");
      handdrawn["backgroundColor"] = Qt::white;
      handdrawn["foregroundColor"] = Qt::gray;
      handdrawn["calculatedColor"] = Qt::darkGray;
      handdrawn["importedColor"] = Qt::lightGray;
      handdrawn["font"] = QFont("Domestic Manners", 16);
      new QListWidgetItem(handdrawn["name"].toString(), ui.list_schemes);
      m_schemes->append(handdrawn);
    }
    updateScheme(scheme);
  }

  ///////////////////
  // Color schemes //
  ///////////////////

  void SpectraDialog::updateScheme(int scheme) {
    ui.list_schemes->setCurrentRow(scheme);
    if (m_scheme != scheme) {
      m_scheme = scheme;
      schemeChanged();
    }
  }

  void SpectraDialog::addScheme() {
    QHash<QString, QVariant> newScheme = m_schemes->at(m_scheme);
    newScheme["name"] = tr("New Scheme");
    new QListWidgetItem(newScheme["name"].toString(), ui.list_schemes);
    m_schemes->append(newScheme);
    schemeChanged();
  }

  void SpectraDialog::removeScheme() {
    if (m_schemes->size() <= 1) return; // Don't delete the last scheme!
    int ret = QMessageBox::question(this, tr("Confirm Scheme Removal"), tr("Really remove current scheme?"));
    if (ret == QMessageBox::Ok) {
      m_schemes->removeAt(m_scheme);
      delete (ui.list_schemes->takeItem(m_scheme));
    }
  }

  void SpectraDialog::renameScheme() {
    int idx = m_scheme;
    bool ok;
    QString text = QInputDialog::getText(this, tr("Change Scheme Name"),
                                         tr("Enter new name for current scheme:"), QLineEdit::Normal,
                                         m_schemes->at(m_scheme)["name"].toString(), &ok);
    if (ok) {
      (*m_schemes)[idx]["name"] = text;
      delete (ui.list_schemes->takeItem(idx));
      ui.list_schemes->insertItem(idx, m_schemes->at(idx)["name"].toString());
      updateScheme(idx);
    }
  }

  void SpectraDialog::changeBackgroundColor()
  {
    QColor current (m_schemes->at(m_scheme)["backgroundColor"].value<QColor>());
    QColor color = QColorDialog::getColor(current, this);//, tr("Select Background Color")); <-- Title not supported until Qt 4.5 bump.
    if (color.isValid() && color != current) {
      (*m_schemes)[m_scheme]["backgroundColor"] = color;
      schemeChanged();
    }
  }

  void SpectraDialog::changeForegroundColor()
  {
    QColor current (m_schemes->at(m_scheme)["foregroundColor"].value<QColor>());
    QColor color = QColorDialog::getColor(current, this);//, tr("Select Foreground Color")); <-- Title not supported until Qt 4.5 bump.
    if (color.isValid() && color != current) {
      (*m_schemes)[m_scheme]["foregroundColor"] = color;
      schemeChanged();
    }
  }

  void SpectraDialog::changeCalculatedSpectraColor()
  {
    QColor current (m_schemes->at(m_scheme)["calculatedColor"].value<QColor>());
    QColor color = QColorDialog::getColor(current, this);//, tr("Select Calculated Spectra Color")); <-- Title not supported until Qt 4.5 bump.
    if (color.isValid() && color != current) {
      (*m_schemes)[m_scheme]["calculatedColor"] = color;
      schemeChanged();
    }
  }

  void SpectraDialog::changeImportedSpectraColor()
  {
    QColor current (m_schemes->at(m_scheme)["importedColor"].value<QColor>());
    QColor color = QColorDialog::getColor(current, this);//, tr("Select Imported Spectra Color")); <-- Title not supported until Qt 4.5 bump.
    if (color.isValid() && color != current) {
      (*m_schemes)[m_scheme]["importedColor"] = color;
      schemeChanged();
    }
  }

  void SpectraDialog::changeFont()
  {
    bool ok;
    QFont current (m_schemes->at(m_scheme)["font"].value<QFont>());
    QFont font = QFontDialog::getFont(&ok, current, this);
    if (ok) {
      (*m_schemes)[m_scheme]["font"] = font;
      schemeChanged();
    }
  }

  void SpectraDialog::schemeChanged() {
    ui.plot->setBackgroundColor(m_schemes->at(m_scheme)["backgroundColor"].value<QColor>());
    ui.plot->setForegroundColor(m_schemes->at(m_scheme)["foregroundColor"].value<QColor>());
    ui.plot->setFont(m_schemes->at(m_scheme)["font"].value<QFont>());

    QPen currentPen (m_importedSpectra->linePen());
    currentPen.setColor(m_schemes->at(m_scheme)["importedColor"].value<QColor>());
    m_importedSpectra->setLinePen(currentPen);

    currentPen = (m_calculatedSpectra->linePen());
    currentPen.setColor(m_schemes->at(m_scheme)["calculatedColor"].value<QColor>());
    m_calculatedSpectra->setLinePen(currentPen);

  }

  ///////////////////////
  // Plot Manipulation //
  ///////////////////////

  void SpectraDialog::updateCurrentSpectra(QString text)
  {
    m_spectra = text;

    // Set up GUI changes (plot labels, etc) in this ladder:
    if (m_spectra == "Infrared") {
      ui.plot->setDefaultLimits( 4000.0, 400.0, 0.0, 100.0 );
      ui.plot->axis(PlotWidget::BottomAxis)->setLabel(tr("Wavenumber (cm<sup>-1</sup>)"));
      ui.plot->axis(PlotWidget::LeftAxis)->setLabel(m_IR_yaxis);
    }
    else if (m_spectra == "NMR") {
      ui.plot->setDefaultLimits( 10.0, 0.0, 0.0, 1.0 );
      ui.plot->axis(PlotWidget::BottomAxis)->setLabel(tr("Shift (ppm)"));
      ui.plot->axis(PlotWidget::LeftAxis)->setLabel("");
    }

    // Regenerate spectra plot objects and redraw plot
    regenerateCalculatedSpectra();
    regenerateImportedSpectra();
    updatePlot();
  }

  void SpectraDialog::exportSpectra()
  {
    // Prepare filename
    QFileInfo defaultFile(m_molecule->fileName());
    QString defaultPath = defaultFile.canonicalPath();
    if (defaultPath.isEmpty()) {
      defaultPath = QDir::homePath();
    }
    QString defaultFileName = defaultPath + '/' + defaultFile.baseName() + ".tsv";
    QString filename 	= QFileDialog::getSaveFileName(this, tr("Export Calculated Spectrum"), defaultFileName, tr("Tab Separated Values (*.tsv)"));

    // Open file
    QFile file (filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
      qWarning() << "Cannot open file " << filename << " for writing!";
      return;
    }
    QTextStream out(&file);

    // Set up output format
    QString format = "%1\t%2\n";

    // Dump labels depending on spectra type
    if (m_spectra == "Infrared") {
      out << "Frequencies\tIntensities\n";
    }
    else if (m_spectra == "NMR") {
      out << "Shift\tYvalue\n";
    }

    // Dump spectra
    for(int i = 0; i< m_calculatedSpectra->points().size(); i++) {
      out << format.arg(m_calculatedSpectra->points().at(i)->x(), 0, 'g').arg(m_calculatedSpectra->points().at(i)->y(), 0, 'g');
    }

    file.close();
  }


  void SpectraDialog::importSpectra()
  {
    // Setup filename
    QFileInfo defaultFile(m_molecule->fileName());
    QString defaultPath = defaultFile.canonicalPath();
    if (defaultPath.isEmpty()) {
      defaultPath = QDir::homePath();
    }
    QString defaultFileName = defaultPath + '/' + defaultFile.baseName() + ".tsv";
    QStringList filters;
    filters
      << tr("Tab Separated Values") + " (*.tsv)"
      << tr("Comma Separated Values") + " (*.csv)"
      << tr("JCAMP-DX") + " (*.jdx)"
      << tr("All Files") + " (* *.*)";

    QString filename 	= QFileDialog::getOpenFileName(this, tr("Import Spectra"),
                                                       defaultFileName, filters.join(";;"));

    // Open file
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
      qWarning() << "Error opening file \"" << filename << "\".";
      return;
    }

    // get file extension
    QStringList tmp 	= filename.split(".");
    QString ext 	= tmp.at(tmp.size()-1);

    // Clear out any old import data
    if (m_spectra == "Infrared") {
      m_imported_IRwavenumbers.clear();
      m_imported_IRtransmittances.clear();
    }
    else if (m_spectra == "NMR") {
      m_imported_NMRshifts.clear();
      m_imported_NMRintensities.clear();
    }

    // Prepare stream
    QTextStream in(&file);

    // Pick delimiter
    QString delim;
    if      (ext == "tsv" || ext == "TSV")	delim = "\t";
    else if (ext == "csv" || ext == "CSV") 	delim = ",";
    else if (ext == "jdx" || ext == "JDX") 	delim = "\\s+"; // Regex finds whitespace
    else {					// Not a supported file type....
      QMessageBox::warning(this, tr("Spectra Import"), tr("Unknown extension: %1").arg(ext));
      return;
    }

    // Extract data
    while (!in.atEnd()) {
      QString line = in.readLine();
      if (line.trimmed().startsWith("#")) continue; 	//discard comments
      QStringList data = line.split(QRegExp(delim));
      if (data.size() < 2) {
        qWarning() << "SpectraDialog::importSpectra Skipping invalid line in file " << filename << ":\n\t\"" << line << "\"";
        continue;
      }
      if (data.at(0).toDouble() && data.at(1).toDouble()) {
        if (m_spectra == "Infrared") {
          m_imported_IRwavenumbers.push_back(data.at(0).toDouble());
          m_imported_IRtransmittances.push_back(data.at(1).toDouble());
        }
        else if (m_spectra == "NMR") {
          m_imported_NMRshifts.push_back(data.at(0).toDouble());
          m_imported_NMRintensities.push_back(data.at(1).toDouble());
        }
      }
      else {
        qWarning() << "SpectraDialog::importSpectra Skipping entry as invalid:\n\t" << data;
        continue;
      }
    }

    //
    // IR cleanups:
    //
    // Check to see if the transmittances are in fractions or percents by looking for any transmittances > 1.5
    if (m_spectra == "Infrared") {
      bool convert = true;
      for (uint i = 0; i < m_imported_IRtransmittances.size(); i++) {
        if (m_imported_IRtransmittances.at(i) > 1.5) { // If transmittances exist greater than this, they're already in percent.
          convert = false;
          break;
        }
      }
      if (convert) {
        for (uint i = 0; i < m_imported_IRtransmittances.size(); i++) {
          m_imported_IRtransmittances.at(i) *= 100;
        }
      }
    }

    //
    // NMR cleanups:
    //
    // Normalize intensities
    if (m_spectra == "NMR") {
      double max = m_imported_NMRintensities.first();
      for (int i = 0; i < m_imported_NMRintensities.size(); i++) {
        if (m_imported_NMRintensities.at(i) > max) max = m_imported_NMRintensities.at(i);
      }
      for (int i = 0; i < m_imported_NMRintensities.size(); i++) {
        double tmp = m_imported_NMRintensities.at(i);
        tmp /= max;
        m_imported_NMRintensities.replace(i,tmp);
      }
    }

    // Set GUI options
    ui.push_colorImported->setEnabled(true);
    ui.cb_import->setEnabled(true);
    ui.cb_import->setChecked(true);

    // Update plot and plot objects
    getImportedSpectra(m_importedSpectra);
    updatePlot();
  }

  void SpectraDialog::loadSpectra()
  {
    // Setup filename
    QFileInfo defaultFile(m_molecule->fileName());
    QString defaultPath = defaultFile.canonicalPath();
    if (defaultPath.isEmpty()) {
      defaultPath = QDir::homePath();
    }
    QString defaultFileName = defaultPath + '/' + defaultFile.baseName();
    QStringList types;
    // Define data types here. Make sure to include "IR" for IR data and "NMR" for NMR data, etc. 
    // Put the default file extension in (*.ext), i.e. (.out)
    types
      << tr("PWscf IR data (*.out)", "Do not remove 'IR' or '(*.out)' -- needed for parsing later" );
    bool ok;
    QString type = QInputDialog::getItem(this, tr("Data Format"), tr("Format:", "noun, not verb"),
                                         types, 0, false, &ok);
    if (!ok) return;
    
    QStringList filters;
    filters
      << type
      << tr("All Files") + " (* *.*)";

    QString filename 	= QFileDialog::getOpenFileName(this, tr("Load Spectral Data"),
                                                       defaultFileName, filters.join(";;"));

    // Open file
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
      qWarning() << "Error opening file \"" << filename << "\".";
      return;
    }

    // get file extension
    QStringList tmp 	= filename.split(".");
    QString ext 	= tmp.at(tmp.size()-1);

    // Prepare stream
    QTextStream in(&file);

    if (type.contains("IR")) { // We have IR data loaded
      // Set m_spectra
      m_spectra = "Infrared";

      // Clear out any old IR data
      m_IRwavenumbers.clear();
      m_IRtransmittances.clear();
      
      // Set up some info by data type:
      QString delim;
      QString cue; // Skip to this line before reading data in
      int wavenumber_idx;
      int intensity_idx;
      if (type.contains("PWscf")) { // Plane wave self consistant field output
        delim 	= "\\s+"; // finds all whitespace
        cue	= "#  mode";
        wavenumber_idx	= 2;
        intensity_idx	= 4;
      }

      // Cue file
      while (!in.atEnd())
        if (in.readLine().contains(cue)) break;

      // Iterate through file
      int min = (wavenumber_idx < intensity_idx) ? wavenumber_idx : intensity_idx;
      QList<double> intensities;
      while (!in.atEnd()) {
        QString line = in.readLine();
        if (line.trimmed().startsWith("#")) continue; 	//discard comments
        QStringList data = line.split(QRegExp(delim));
        if (data.size() < min) {
          qWarning() << "SpectraDialog::importSpectra Skipping invalid line in file " << filename 
                     << ": Too few entries (need " << min << "\n\t\"" << line << "\"";
          continue;
        }
        if (data.at(wavenumber_idx).toDouble() && data.at(intensity_idx).toDouble()) { // Check for valid conversions and non-zero data
          m_IRwavenumbers.push_back(data.at(wavenumber_idx).toDouble());
          intensities.push_back(data.at(intensity_idx).toDouble());
        }
        else {
          qWarning() << "SpectraDialog::importSpectra Skipping entry as invalid:\n\t" << data;
          continue;
        }
      }

      // Convert intensities --> transmittances
      double maxIntensity=0;
      for (unsigned int i = 0; i < intensities.size(); i++) {
        if (intensities.at(i) >= maxIntensity) {
          maxIntensity = intensities.at(i);
        }
      }
      for (unsigned int i = 0; i < intensities.size(); i++) {
        double t = intensities.at(i);
        t = t / maxIntensity; 	// Normalize
        t = 0.97 * t;		// Keeps the peaks from extending to the limits of the plot
        t = 1.0 - t; 		// Simulate transmittance
        t *= 100.0;		// Convert to percent
        m_IRtransmittances.push_back(t);
      }
      // Setup GUI
      ui.combo_spectra->addItem(tr("Infrared", "Infrared spectra option"));
      ui.tab_widget->addTab(ui.tab_infrared, tr("&Infrared Spectra Settings"));

      // Setup signals/slots
      connect(this, SIGNAL(scaleUpdated_IR()),
              this, SLOT(regenerateCalculatedSpectra()));
      connect(ui.cb_IR_labelPeaks, SIGNAL(toggled(bool)),
              this, SLOT(regenerateCalculatedSpectra()));
      connect(ui.spin_IR_scale, SIGNAL(valueChanged(double)),
              this, SLOT(setScale_IR(double)));
      connect(ui.spin_IR_FWHM, SIGNAL(valueChanged(double)),
              this, SLOT(regenerateCalculatedSpectra()));
      connect(ui.combo_IR_yaxis, SIGNAL(currentIndexChanged(QString)),
              this, SLOT(updateYAxis_IR(QString)));
    }

    // Non-spectra-specific GUI changes
    ui.push_colorCalculated->setEnabled(true);
    ui.cb_calculate->setEnabled(true);
    ui.cb_calculate->setChecked(true);

    // Update plot and plot objects
    getCalculatedSpectra(m_calculatedSpectra);
    updateCurrentSpectra(m_spectra);
    updatePlot();
  }

  void SpectraDialog::saveImageFileDialog() {
    QStringList filters;
    filters
      << tr("Portable Network Graphics") + " (*.png)"
      << tr("jpeg") + " (*.jpg *.jpeg)"
      << tr("Tagged Image File Format") + " (*.tiff)"
      << tr("Windows Bitmap") + " (*.bmp)"
      << tr("Portable Pixmap") + " (*.ppm)"
      << tr("X11 Bitmap") + " (*.xbm)"
      << tr("X11 Pixmap") + " (*.xpm)"
      << tr("All Files") + " (*.*)";


    QString filename   = QFileDialog::getSaveFileName(this, tr("Save Spectra Image"),
                                                      ui.edit_imageFilename->text(),
                                                      filters.join(";;"));
    if (filename.isEmpty()) {
      return;
    }
    // get file extension
    QStringList tmp 	= filename.split(".");
    QString ext 	= tmp.at(tmp.size()-1);

    if (ext != "png" && ext != "PNG" &&
        ext != "jpg" && ext != "JPG" &&
        ext != "bmp" && ext != "BMP" &&
        ext != "ppm" && ext != "PPM" &&
        ext != "xbm" && ext != "XBM" &&
        ext != "xpm" && ext != "XPM" &&
        ext != "tiff" && ext != "TIFF" ) {
      qWarning() << "SpectraDialog::saveImageFileDialog Invalid file extension: " << ext;
      QMessageBox::warning(this, tr("Invalid Filename"), tr("Unknown extension: %1").arg(ext));
      return;
    }
    ui.edit_imageFilename->setText(filename);
  }

  void SpectraDialog::saveImage()
  {
    QString filename = ui.edit_imageFilename->text();
    double w,h,factor=1;
    int dpi;

    switch (ui.combo_imageUnits->currentIndex())
      {
      case 0: // cm
        factor = 0.01;
        break;
      case 1: // mm
        factor = 0.001;
        break;
      case 2: // in
        factor = 0.0254;
        break;
      case 4: // px
        factor = 1;
        break;
    }
    w = factor * ui.spin_imageWidth->value();
    h = factor * ui.spin_imageHeight->value();
    dpi = ui.spin_imageDPI->value();
    bool opt = ui.cb_imageFontAdjust->isChecked();
    if (!ui.plot->saveImage(filename, w, h, dpi, opt)) {
      qWarning() << "SpectraDialog::saveImage Error saving plot to " << filename;
      QMessageBox::warning(this, tr("Error"), tr("A problem occurred while writing file %1").arg(filename));
    } else {
      QMessageBox::information(this, tr("Success!"), tr("Image successfully written to %1").arg(filename));
    }
  }

  void SpectraDialog::toggleImported(bool state) {
    if (state) {
      ui.plot->replacePlotObject(1,m_importedSpectra);
    }
    else {
      ui.plot->replacePlotObject(1,m_nullSpectra);
    }
    updatePlot();
  }

  void SpectraDialog::toggleCalculated(bool state) {
    if (state) {
      ui.plot->replacePlotObject(0,m_calculatedSpectra);
    }
    else {
      ui.plot->replacePlotObject(0,m_nullSpectra);
    }
    updatePlot();
  }

  void SpectraDialog::toggleCustomize() {
    if (ui.tab_widget->isHidden()) {
      ui.push_customize->setText(tr("Customi&ze <<"));
      ui.tab_widget->show();
    }
    else {
      ui.push_customize->setText(tr("Customi&ze >>"));
      ui.tab_widget->hide();
    }
  }

  void SpectraDialog::regenerateCalculatedSpectra() {

    // IR spectra checks:
    if (ui.spin_IR_FWHM->value() != 0.0 && ui.cb_IR_labelPeaks->isEnabled()) {
      ui.cb_IR_labelPeaks->setEnabled(false);
      ui.cb_IR_labelPeaks->setChecked(false);
    }
    if (ui.spin_IR_FWHM->value() == 0.0 && !ui.cb_IR_labelPeaks->isEnabled()) {
      ui.cb_IR_labelPeaks->setEnabled(true);
    }
    if (!ui.cb_IR_labelPeaks->isEnabled()) {
      ui.cb_IR_labelPeaks->setChecked(false);
    }
    // NMR spectra checks:
    if (ui.spin_NMR_FWHM->value() != 0.0 && ui.cb_NMR_labelPeaks->isEnabled()) {
      ui.cb_NMR_labelPeaks->setEnabled(false);
      ui.cb_NMR_labelPeaks->setChecked(false);
    }
    if (ui.spin_NMR_FWHM->value() == 0.0 && !ui.cb_NMR_labelPeaks->isEnabled()) {
      ui.cb_NMR_labelPeaks->setEnabled(true);
    }
    if (!ui.cb_NMR_labelPeaks->isEnabled()) {
      ui.cb_NMR_labelPeaks->setChecked(false);
    }

    // Update plot object and display changes
    getCalculatedSpectra(m_calculatedSpectra);
    updatePlot();
  }

  void SpectraDialog::regenerateImportedSpectra() {
    getImportedSpectra(m_importedSpectra);
    updatePlot();
  }

  void SpectraDialog::updatePlot()
  {
    ui.plot->update();
  }

  void SpectraDialog::getCalculatedSpectra(PlotObject *plotObject)
  {
    // Clear out old data
    plotObject->clearPoints();

    // Use this ladder to select correct spectra function:
    if (m_spectra == "Infrared") {
      if (ui.spin_IR_FWHM->value() == 0.0) {
        getCalculatedSinglets_IR(plotObject);
      }
      else {
        getCalculatedGaussians_IR(plotObject);
      }
      if (ui.combo_IR_yaxis->currentText() == "Absorbance (%)") {
        for(int i = 0; i< plotObject->points().size(); i++) {
          double absorbance = 100 - plotObject->points().at(i)->y();
          plotObject->points().at(i)->setY(absorbance);
        }
      }
    } // End IR spectra
    if (m_spectra == "NMR") {
      if (ui.spin_NMR_FWHM->value() == 0.0) {
        getCalculatedSinglets_NMR(plotObject);
      }
      else {
        getCalculatedGaussians_NMR(plotObject);
      }
    } // End NMR spectra

  }// End getCalculatedSpectra

  void SpectraDialog::getImportedSpectra(PlotObject *plotObject)
  {
    // Clear out old data
    plotObject->clearPoints();

    // Use this ladder to add other spectra types
    if (m_spectra == "Infrared") {
      for (uint i = 0; i < m_imported_IRtransmittances.size(); i++) {
        double wavenumber = m_imported_IRwavenumbers.at(i);
        double y = m_imported_IRtransmittances.at(i);
        if (ui.combo_IR_yaxis->currentText() == "Absorbance (%)") {
          y = 100 - y;
        }
        plotObject->addPoint ( wavenumber, y );
      }
    }// End IR spectra
    else if (m_spectra == "NMR") {
      for (int i = 0; i < m_imported_NMRshifts.size(); i++) {
        double shift = m_imported_NMRshifts.at(i);
        double y = m_imported_NMRintensities.at(i);
        plotObject->addPoint ( shift, y );
      }
    }// End NMR spectra

  }

  //////////////////
  // IR Functions //
  //////////////////

  void SpectraDialog::getCalculatedSinglets_IR(PlotObject *plotObject)
  {
    plotObject->addPoint( 400, 100); // Initial point

    for (uint i = 0; i < m_IRtransmittances.size(); i++) {
      double wavenumber = m_IRwavenumbers.at(i) * m_IR_scale;
      double transmittance = m_IRtransmittances.at(i);
      plotObject->addPoint ( wavenumber, 100 );
      if (ui.cb_IR_labelPeaks->isChecked()) {
        plotObject->addPoint( wavenumber, transmittance, QString::number(wavenumber, 'f', 1));
      }
      else {
        plotObject->addPoint( wavenumber, transmittance );
      }
      plotObject->addPoint( wavenumber, 100 );
    }
    plotObject->addPoint( 4000, 100); // Final point
    return;
  }

  void SpectraDialog::updateYAxis_IR(QString text)
  {
    if (m_IR_yaxis == text) {
      return;
    }
    ui.plot->axis(PlotWidget::LeftAxis)->setLabel(text);
    m_IR_yaxis = text;
    regenerateCalculatedSpectra();
    regenerateImportedSpectra();
  }

  void SpectraDialog::setScale_IR(double scale)
  {
    if (scale == m_IR_scale) {
      return;
    }
    m_IR_scale = scale;
    ui.spin_IR_scale->setValue(scale);
    emit scaleUpdated_IR();
  }

  void SpectraDialog::getCalculatedGaussians_IR(PlotObject *plotObject)
  {
    // convert FWHM to sigma squared
    double FWHM = ui.spin_IR_FWHM->value();
    double s2	= pow( (FWHM / (2.0 * sqrt(2.0 * log(2.0)))), 2.0);

    // determine range
    // - find maximum and minimum
    double min = 0.0 + 2*FWHM;
    double max = 4000.0 - 2*FWHM;
    for (uint i = 0; i < m_IRwavenumbers.size(); i++) {
      double cur = m_IRwavenumbers.at(i);
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
      double y = 100;
      for (uint i = 0; i < m_IRtransmittances.size(); i++) {
        double t = m_IRtransmittances.at(i);
        double w = m_IRwavenumbers.at(i) * m_IR_scale;
        y += (t-100) * exp( - ( pow( (x - w), 2 ) ) / (2 * s2) );
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
      // 100 / (max - min)	: Conversion factor for current spread -> percent
      // * 0.97 + 3		: makes plot stay away from 0 transmittance
      //			: (easier to see multiple peaks on strong signals)
      plotObject->points().at(i)->setY( (cur - min) * 100 / (max - min) * 0.97 + 3);
    }
  }

  ///////////////////
  // NMR Functions //
  ///////////////////

  void SpectraDialog::setNMRAtom(const QString & symbol)
  {
    if (!m_NMRdata->contains(symbol)) return;

    m_NMRshifts = m_NMRdata->value(symbol);
    updatePlotAxes_NMR();
    regenerateCalculatedSpectra();
  }

  void SpectraDialog::getCalculatedSinglets_NMR(PlotObject *plotObject)
  {
    for (int i = 0; i < m_NMRshifts.size(); i++) {
      double shift = m_NMRshifts.at(i) - m_NMR_ref;
      //      double intensity = m_NMRintensities.at(i);
      plotObject->addPoint ( shift, 0);
            if (ui.cb_NMR_labelPeaks->isChecked()) {
              plotObject->addPoint( shift, 1.0 /* intensity */, QString::number(shift, 'f', 2));
            }
            else {
              plotObject->addPoint( shift, 1.0 /* intensity */ );
            }
      plotObject->addPoint( shift, 0 );
    }
    return;
  }

  void SpectraDialog::updatePlotAxes_NMR()
  {
    QList<double> tmp (m_NMRshifts);
    qSort(tmp);
    double FWHM = ui.spin_NMR_FWHM->value();
    if (tmp.size() == 1) {
      double center 	= tmp.first() - m_NMR_ref;
      double ext	= 5 + FWHM;
      ui.plot->setDefaultLimits( center + ext, center - ext, 0.0, 1.0 );
    }
    else {
      double min = tmp.last() - m_NMR_ref;
      double max = tmp.first() - m_NMR_ref;
      double ext;
      if (fabs(min-max) < 0.1) { // If the spread of the peaks is less than 0.1, the nuclei are likely equivalent, so zoom out a bit.
        ext = 5;
      }
      else {
        ext = ( min - max ) * 0.1 + FWHM;
      }
      ui.plot->setDefaultLimits( min + ext, max - ext, 0.0, 1.0 );
    }
  }

  void SpectraDialog::setReference_NMR(double ref)
  {
    if (ref == m_NMR_ref) {
      return;
    }
    m_NMR_ref = ref;
    ui.spin_NMR_ref->setValue(ref);
    emit refUpdated_NMR();
  }

   void SpectraDialog::getCalculatedGaussians_NMR(PlotObject *plotObject)
  {
    if (m_NMRshifts.isEmpty()) return;

    // convert FWHM to sigma squared
    double FWHM = ui.spin_NMR_FWHM->value();
    double s2	= pow( (FWHM / (2.0 * sqrt(2.0 * log(2.0)))), 2.0);

    // determine range
    // - find maximum and minimum
    double min = m_NMRshifts.first() + 2*FWHM - m_NMR_ref;
    double max = m_NMRshifts.first() - 2*FWHM - m_NMR_ref;
    for (int i = 0; i < m_NMRshifts.size(); i++) {
      double cur = m_NMRshifts.at(i) - m_NMR_ref;
      if (cur > max) max = cur;
      if (cur < min) min = cur;
    }
    min -= 2*FWHM;
    max += 2*FWHM;
    // - get resolution (TODO)
    double res = (FWHM/10.0 < 0.01) ? FWHM/10.0 : 0.01;
    if (res < 0.001) res = 0.001;
    // create points
    for (double x = min; x < max; x += res) {
      double y = 0;
      for (int i = 0; i < m_NMRshifts.size(); i++) {
        double t = 1.0; //m_NMRintensities.at(i);
	double w = m_NMRshifts.at(i) - m_NMR_ref;
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
  }
}
#include "spectradialog.moc"
