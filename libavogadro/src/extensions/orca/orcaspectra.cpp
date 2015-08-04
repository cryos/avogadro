/**********************************************************************
  OrcaSpectra - plot IR spectra

  Copyright (C) 2014 Dagmar Lenk

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.cc/>

  Avogadro is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  Avogadro is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.
 **********************************************************************/
#include "orcaspectra.h"

#include <avogadro/plotobject.h>

#include <vector>
using namespace Eigen;
using namespace  std;

namespace Avogadro {

OrcaSpectra::OrcaSpectra(QWidget *parent, Qt::WindowFlags f ) :
    QDialog( parent, f ), m_vibration(NULL)
{

    // This initializes the ui member function to contain pointers to
    // all GUI elements in the orcaanalysedialog.ui file
    ui.setupUi(this);

    connect (ui.plotIR, SIGNAL(pointClicked(double,double)), this, SLOT(freqChangedIdx(double,double)));

    connect(ui.plotIR, SIGNAL(pointClicked(PlotPoint*)),
            ui.plotIR, SLOT(clearAndSelectPoint(PlotPoint*)));

}
void OrcaSpectra::plotSpectra(OrcaVibrations* vibData)
{
    if (vibData->checkOK()) {
        m_vibration = vibData;
        ui.plotIR->removeAllPlotObjects();
        PlotObject *data = new PlotObject (Qt::red, PlotObject::Bars, 0.1);
        double minIntens, maxIntens, curIntens;
        double minFreq, maxFreq, curFreq;
        minIntens = maxIntens = m_vibration->intensities().at(0);
        minFreq = maxFreq = m_vibration->frequencies().at(0);

        for (uint i = 0; i < m_vibration->intensities().size(); i++) {
            curIntens = m_vibration->intensities().at(i);
            curFreq = m_vibration->frequencies().at(i);
            minIntens = min (curIntens, minIntens);
            minFreq = min(curFreq, minFreq);

            maxIntens = max(curIntens, maxIntens);
            maxFreq = max(curFreq, maxFreq);

            data->addPoint(curFreq, curIntens, QString::number(vibData->modes().at(i)), 0.1);
        }
        double spreadX = maxFreq - minFreq;
        double extX = spreadX * 0.05;
        double spreadY = maxIntens - minIntens;
        double extY = spreadY * 0.05;

        ui.plotIR->setDefaultLimits(minFreq-extX,
                                        maxFreq+extX,
                                        minIntens - extY,
                                        maxIntens + extY);

        ui.plotIR->setJailedInDefaults(true);

        ui.plotIR->setAntialiasing(true);
        ui.plotIR->setMouseTracking(true);
        ui.plotIR->axis(PlotWidget::BottomAxis)->setLabel(tr("Frequencies"));
        ui.plotIR->axis(PlotWidget::LeftAxis)->setLabel(tr("Intensities"));
        ui.plotIR->addPlotObject(data);

        ui.plotIR->setVisible(true);
    } else {
        QMessageBox msgBox;

        msgBox.setWindowTitle(tr("OrcaExtension"));
        msgBox.setText(tr(" OrcaSpectra::No vibration data found!"));
        msgBox.exec();

    }
}

void OrcaSpectra::freqChangedIdx(double x, double y)
{
    qDebug() << "OrcaSpectra::freqChangedIdx" << x << " " << y << " ";
    for (uint i = 0; i < m_vibration->frequencies().size(); i++) {
        double curFreq = m_vibration->frequencies().at(i);
        if (curFreq >= x) {
            emit selectNewVibration(i);
            return;
        }
    }

}
} // end namespace

