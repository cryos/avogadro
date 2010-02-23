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

#include "spectratype.h"
#include "spectradialog.h"

#include <QtCore/QList>
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QTextStream>
#include <QtCore/QDebug>

#include <avogadro/primitive.h>
#include <avogadro/plotwidget.h>
#include <avogadro/molecule.h>

namespace Avogadro {

  SpectraType::SpectraType( SpectraDialog *parent ) : QObject(parent), m_dialog(parent)
  {
    m_tab_widget = new QWidget;
  }
  
  SpectraType::~SpectraType()
  {
    clear();
    disconnect(m_dialog->getUi()->combo_spectra, SIGNAL(currentIndexChanged(QString)),
        m_dialog, SLOT(updateCurrentSpectra(QString)));    
    delete m_tab_widget;
  }

  void SpectraType::clear()
  {
    m_xList.clear();
    m_yList.clear();
    m_xList_imp.clear();
    m_yList_imp.clear();
  }

  void SpectraType::getCalculatedPlotObject(PlotObject *plotObject)
  {
    plotObject->clearPoints();
    for (int i = 0; i < m_xList.size(); i++)
      plotObject->addPoint(m_xList.at(i), m_yList.at(i));
  }
  
  void SpectraType::setImportedData(const QList<double> & xList, const QList<double> & yList)
  {
    m_xList_imp = xList;
    m_yList_imp = yList;
  }
  
  void SpectraType::getImportedPlotObject(PlotObject *plotObject)
  {
    plotObject->clearPoints();
    for (int i = 0; i < m_xList_imp.size(); i++) {
      plotObject->addPoint(m_xList_imp.at(i), m_yList_imp.at(i));
    }
  }
  
  QString SpectraType::getTSV(QString xTitle, QString yTitle)
  {
    QString str;
    QTextStream out (&str);
    QString format = "%1\t%2\n";
    out << xTitle << "\t" << yTitle << "\n";
    for(int i = 0; i< m_xList.size(); i++) {
      out << format.arg(m_xList.at(i), 6, 'g').arg(m_yList.at(i), 6, 'g');
    }
    return str;
  }

  void SpectraType::updateDataTable()
  {
    if ((!m_dialog) || (m_xList.size()==0))
      return;
    //m_dialog->getUi()->dataTable->clear();
    m_dialog->getUi()->dataTable->setRowCount(m_xList.size());
    QString format("%1");
    for (int i = 0; i < m_xList.size(); i++) {
      QString xString = format.arg(m_xList.at(i), 0, 'f', 2);
      QString yString;
      if (i < m_yList.size()) {
        yString = format.arg(m_yList.at(i), 0, 'f', 3);
      } else {
        yString = "-";
      }
      if (!m_dialog->getUi()->dataTable->item(i,0)) {
        QTableWidgetItem *newX = new QTableWidgetItem(xString);
        newX->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);
        QTableWidgetItem *newY = new QTableWidgetItem(yString);
        newY->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);
        m_dialog->getUi()->dataTable->setItem(i, 0, newX);
        m_dialog->getUi()->dataTable->setItem(i, 1, newY);
      } else {
        m_dialog->getUi()->dataTable->item(i,0)->setText(xString);
        m_dialog->getUi()->dataTable->item(i,1)->setText(yString);
      }      
    }
  }

  QList<double> SpectraType::getXPoints(double FWHM, uint dotsPerPeak)
  {
    QList<double> xPoints;
    for (int i = 0; i < m_xList.size(); i++) {
      double x = m_xList.at(i) - (2*FWHM);
      for (uint j = 0; j < dotsPerPeak; j++) {
        xPoints << x;
        x += 4*FWHM / (int(dotsPerPeak));
      }
    }
    qSort(xPoints);
    return xPoints;
  }
}


