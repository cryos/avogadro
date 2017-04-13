/**********************************************************************
  SpectraDialog - Visualize spectral data from QM calculations

  Copyright (C) 2009 by David Lonie

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.cc/>

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

#ifndef SPECTRATYPE_CD_H
#define SPECTRATYPE_CD_H

#include <QHash>
#include <QVariant>

#include "spectradialog.h"
#include "spectratype.h"
#include "ui_tab_cd.h"

namespace Avogadro {

  class CDSpectra : public SpectraType
  {
    Q_OBJECT

  public:
    CDSpectra( SpectraDialog *parent = 0 );
    ~CDSpectra();

    void writeSettings();
    void readSettings();

    bool checkForData(Molecule* mol);
    void setupPlot(PlotWidget * plot);

    void getCalculatedPlotObject(PlotObject *plotObject);
    //void setImportedData(const QList<double> & xList, const QList<double> & yList);
    //void getImportedPlotObject(PlotObject *plotObject);
    QString getTSV();
    QString getDataStream(PlotObject *plotObject);
  private slots:
    void rotatoryTypeChanged(const QString & str);

  private:
    Ui::Tab_CD ui;
    QList<double> *m_yListVelocity, *m_yListLength;
    double m_fermi;
  };
}

#endif
//#endif
