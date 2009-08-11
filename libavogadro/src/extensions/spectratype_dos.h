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

#ifndef SPECTRATYPE_DOS_H
#define SPECTRATYPE_DOS_H

#include <QHash>
#include <QVariant>

#include "spectradialog.h"
#include "spectratype.h"
#include "ui_spectratabdos.h"

#include <avogadro/plotwidget.h>

namespace Avogadro {

  class DOSSpectra : public SpectraType
  {
    Q_OBJECT

  public:
    DOSSpectra( SpectraDialog *parent = 0 );
    ~DOSSpectra();

    void writeSettings();
    void readSettings();

    bool checkForData(Molecule* mol);
    void setupPlot(PlotWidget * plot);

    QWidget * getTabWidget();

    void getCalculatedPlotObject(PlotObject *plotObject);
    void setImportedData(const QList<double> & xList, const QList<double> & yList);
    void getImportedPlotObject(PlotObject *plotObject);
    QString getTSV();

  public slots:

  private slots:

  signals:
    void plotDataChanged();

  private:
    Ui::Tab_DOS ui;
    QWidget *m_tab_widget;
    QList<double> *m_xList, *m_yList;
    QList<double> *m_xList_imp, *m_yList_imp;
    SpectraDialog *m_dialog;
  };
}

#endif
