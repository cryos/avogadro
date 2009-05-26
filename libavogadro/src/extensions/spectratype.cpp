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

#include <QObject>

#include <avogadro/primitive.h>
#include <avogadro/plotwidget.h>
#include <avogadro/molecule.h>

namespace Avogadro {

  SpectraType::SpectraType( SpectraDialog *parent ) : QObject(parent) {}
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
}


