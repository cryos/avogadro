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

#ifndef SPECTRATYPE_NMR_H
#define SPECTRATYPE_NMR_H

#include <QHash>

#include "spectradialog.h"
#include "spectratype.h"
#include "ui_spectratabnmr.h"

#include <avogadro/plotwidget.h>

namespace Avogadro {

  class NMRSpectra : public SpectraType
  {
    Q_OBJECT

  public:
    NMRSpectra( SpectraDialog *parent = 0 );
    ~NMRSpectra();

    virtual void writeSettings();
    virtual void readSettings();

    virtual bool checkForData(Molecule* mol);
    virtual void setupPlot(PlotWidget * plot);

    virtual QWidget * getTabWidget();

    virtual void getCalculatedPlotObject(PlotObject *plotObject);
    virtual void setImportedData(const QList<double> & xList, const QList<double> & yList);
    virtual void getImportedPlotObject(PlotObject *plotObject);
    virtual QString getTSV();

  public slots:
    void setAtom(QString symbol);

  private slots:
    void setReference(double ref);
    void updatePlotAxes();

  signals:
    void plotDataChanged();

  private:
    Ui::Tab_NMR ui;
    QWidget *m_tab_widget;
    QList<double> *m_xList, *m_yList;
    QList<double> *m_xList_imp, *m_yList_imp;
    SpectraDialog *m_dialog;
    double m_ref;
    QHash<QString, QList<double>* > *m_NMRdata;
  };
}

#endif
