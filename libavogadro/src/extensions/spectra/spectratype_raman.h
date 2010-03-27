/**********************************************************************
  SpectraDialog - Visualize spectral data from QM calculations

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
//#ifdef OPENBABEL_IS_NEWER_THAN_2_2_99

#ifndef SPECTRATYPE_RAMAN_H
#define SPECTRATYPE_RAMAN_H

#include "spectratype_abstract_ir.h"

namespace Avogadro {

  class RamanSpectra : public AbstractIRSpectra
  {
    Q_OBJECT

  public:
    RamanSpectra( SpectraDialog *parent = 0 );
    ~RamanSpectra();

    void writeSettings();
    void readSettings();

    bool checkForData(Molecule* mol);
    void setupPlot(PlotWidget * plot);

    void getCalculatedPlotObject(PlotObject *plotObject);
    QString getTSV();

  private slots:
    void updateT(double);
    void updateW(double);

  private:
    double m_W;
    double m_T;
    QList<double> m_yList_orig;
  };
}

#endif
//#endif
