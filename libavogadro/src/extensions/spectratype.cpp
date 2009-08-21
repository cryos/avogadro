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

#include "spectratype.h"
#include "spectradialog.h"

#include <QList>
#include <QObject>

#include <avogadro/primitive.h>
#include <avogadro/plotwidget.h>
#include <avogadro/molecule.h>

namespace Avogadro {

  SpectraType::SpectraType( SpectraDialog *parent ) : QObject(parent), m_tab_widget(0), m_xList(0), 
                                                      m_yList(0), m_xList_imp(0), m_yList_imp(0) {}
  SpectraType::~SpectraType() {}

  void SpectraType::writeSettings() {}
  void SpectraType::readSettings() {}

  bool SpectraType::checkForData(Molecule* mol) {Q_UNUSED(mol);return false;}
  void SpectraType::setupPlot(PlotWidget * plot) {Q_UNUSED(plot);}

  QWidget * SpectraType::getTabWidget() {return new QWidget;}

  void SpectraType::getCalculatedPlotObject(PlotObject *plotObject) {Q_UNUSED(plotObject);}
  void SpectraType::setImportedData(const QList<double> & xList, const QList<double> & yList) {Q_UNUSED(xList); Q_UNUSED(yList);}
  void SpectraType::getImportedPlotObject(PlotObject *plotObject) {Q_UNUSED(plotObject);}
  QString SpectraType::getTSV() {return QString("");}

  QList<double> SpectraType::getXPoints(double FWHM, uint dotsPerPeak) {
    QList<double> xPoints;
    for (int i = 0; i < m_xList->size(); i++) {
      double x = m_xList->at(i) - (2*FWHM);
      for (uint j = 0; j < dotsPerPeak; j++) {
        xPoints << x;
        x += 4*FWHM / (int(dotsPerPeak));
      }
    }
    qSort(xPoints);
    return xPoints;
  }
}


