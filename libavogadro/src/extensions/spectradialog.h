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
  GNU General Public icense for more details.
 ***********************************************************************/

#ifndef SPECTRADIALOG_H
#define SPECTRADIALOG_H

#include <QDialog>
#include <QHash>
#include <QVariant>
#include <QSettings>

#include <avogadro/primitive.h>
#include <avogadro/plotwidget.h>
#include <avogadro/molecule.h>


#include <openbabel/mol.h>
#include <openbabel/generic.h>

#include "ui_spectradialog.h"

namespace Avogadro {

  class SpectraDialog : public QDialog
  {
    Q_OBJECT

  public:
    explicit SpectraDialog( QWidget *parent = 0, Qt::WindowFlags f = 0 );
    ~SpectraDialog();

    void setMolecule(Molecule *molecule);
    void writeSettings() const;
    void readSettings();

  public slots:
    void setScale_IR(double scale);

  private slots:
    void changeBackgroundColor();
    void changeForegroundColor();
    void changeCalculatedSpectraColor();
    void changeImportedSpectraColor();
    void changeFont();
    void importSpectra();
    void updatePlot();
    void toggleImported(bool state);
    void toggleCalculated(bool state);
    void regenerateCalculatedSpectra();
    void regenerateImportedSpectra();
    void saveImage();
    void toggleCustomize();
    void updateYAxis_IR(QString);
    void schemeChanged();
    void updateScheme(int scheme);
    void updateCurrentSpectra(QString text);
    void addScheme();
    void removeScheme();
    void renameScheme();
    void exportSpectra();
    void saveImageFileDialog();

  signals:
    void scaleUpdated_IR();

  private:
    Ui::SpectraDialog ui;

    void getCalculatedSpectra(PlotObject *plotObject);
    void getCalculatedSinglets_IR(PlotObject *plotObject);
    void getCalculatedGaussians_IR(PlotObject *plotObject);
    void getImportedSpectra(PlotObject *plotObject);

    double m_IR_scale;
    QString m_IR_yaxis;
    int m_scheme;
    QList<QHash<QString, QVariant> > *m_schemes;
    PlotObject *m_calculatedSpectra;
    PlotObject *m_importedSpectra;
    PlotObject *m_nullSpectra;
    QString m_spectra;
    std::vector<double> m_IRwavenumbers;
    std::vector<double> m_IRtransmittances;
    std::vector<double> m_imported_IRwavenumbers;
    std::vector<double> m_imported_IRtransmittances;
    Molecule *m_molecule;
    OpenBabel::OBVibrationData *m_vibrations;
  };
}

#endif
