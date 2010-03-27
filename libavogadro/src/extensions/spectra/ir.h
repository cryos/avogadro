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

#ifndef SPECTRATYPE_IR_H
#define SPECTRATYPE_IR_H

#include "abstract_ir.h"

namespace Avogadro {

  class IRSpectra : public AbstractIRSpectra
  {
    Q_OBJECT

  public:
    IRSpectra( SpectraDialog *parent = 0 );
    ~IRSpectra();

    void writeSettings();
    void readSettings();

    bool checkForData(Molecule* mol);
    void setupPlot(PlotWidget * plot);

    void getCalculatedPlotObject(PlotObject *plotObject);
    void setImportedData(const QList<double> & xList, const QList<double> & yList);
    QString getTSV();
  };
}

#endif
