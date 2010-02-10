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

  class SpectraType;
  class IRSpectra;
  class NMRSpectra;
#ifdef OPENBABEL_IS_NEWER_THAN_2_2_99
  class DOSSpectra;
  class UVSpectra;
  class CDSpectra;
  class RamanSpectra;
#endif
  class SpectraDialog : public QDialog
  {
    Q_OBJECT

  public:
    explicit SpectraDialog( QWidget *parent = 0, Qt::WindowFlags f = 0 );
    ~SpectraDialog();

    void setMolecule(Molecule *molecule);
    void writeSettings() const;
    void readSettings();
    Ui::SpectraDialog * getUi() {return &ui;};

  public slots:
    void regenerateCalculatedSpectra();
    void regenerateImportedSpectra();

  private slots:
    void changeBackgroundColor();
    void changeForegroundColor();
    void changeCalculatedSpectraColor();
    void changeImportedSpectraColor();
    void changeFont();
    void loadSpectra(); 	// Used to load calculated data from an external source
    void importSpectra(); 	// Used to load experimental spectra
    void updatePlot();
    void toggleImported(bool state);
    void toggleCalculated(bool state);
    void saveImage();
    void toggleCustomize();
    void schemeChanged();
    void updateScheme(int scheme);
    void updateCurrentSpectra(const QString & text);
    void updateComboSpectra(int index);
    void addScheme();
    void removeScheme();
    void renameScheme();
    void exportSpectra();
    void saveImageFileDialog();

  signals:

  private:
    Ui::SpectraDialog ui;

    IRSpectra *m_spectra_ir;
    NMRSpectra *m_spectra_nmr;
#ifdef OPENBABEL_IS_NEWER_THAN_2_2_99
    DOSSpectra *m_spectra_dos;
    UVSpectra *m_spectra_uv;
    CDSpectra *m_spectra_cd;
    RamanSpectra *m_spectra_raman;
#endif

    Molecule *m_molecule;
    int m_scheme;
    QList<QHash<QString, QVariant> > *m_schemes;

    QString m_spectra;
    SpectraType * currentSpectra();
    PlotObject *m_calculatedSpectra;
    PlotObject *m_importedSpectra;
    PlotObject *m_nullSpectra;
  };
}

#endif
