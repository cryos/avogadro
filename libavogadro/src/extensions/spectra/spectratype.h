/**********************************************************************
  SpectraDialog - Visualize spectral data from QM calculations

  Copyright (C) 2009 by David Lonie
  Copyright (C) 2010 by Konstantin Tokarev

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

#ifndef SPECTRATYPE_H
#define SPECTRATYPE_H

#include <QtGui/QDialog>
#include <QtCore/QHash>
#include <QtCore/QVariant>
#include <QtCore/QSettings>

#include <avogadro/primitive.h>
#include <avogadro/plotwidget.h>
#include <avogadro/molecule.h>

#include <openbabel/mol.h>
#include <openbabel/generic.h>

namespace Avogadro {

  class SpectraDialog;

  // Abstract data type - no instance of it can be created
  class SpectraType : public QObject
  {
    Q_OBJECT

   public:
    SpectraType( SpectraDialog *parent );
    virtual ~SpectraType();

    // These functions must be implemented in inherited classes
    virtual void writeSettings() = 0;
    virtual void readSettings() = 0;
    virtual bool checkForData(Molecule* mol) = 0;
    virtual void setupPlot(PlotWidget * plot) = 0;
    virtual QString getTSV() = 0;

    // These function have default implementations, but may be overridden    
    virtual void getCalculatedPlotObject(PlotObject *plotObject);
    virtual void setImportedData(const QList<double> & xList, const QList<double> & yList);
    virtual void getImportedPlotObject(PlotObject *plotObject);
    virtual void updateDataTable();

    // No need to override these functions
    QList<double> getXPoints(double FWHM, uint dotsPerPeak);
    QWidget * getTabWidget() {return m_tab_widget;}
    QString getTSV(QString xTitle, QString yTitle);
    void clear();
    void gaussianWiden(PlotObject *plotObject, const double fwhm);
    static void assignGaussianLabels(PlotObject *plotObject, const bool findMax);

  signals:
    void plotDataChanged();

  protected:
    SpectraDialog *m_dialog;
    QWidget *m_tab_widget;
    QList<double> m_xList, m_yList, m_xList_imp, m_yList_imp;
  };
}

#endif
