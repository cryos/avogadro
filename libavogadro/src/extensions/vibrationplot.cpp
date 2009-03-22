/**********************************************************************
  VibrationPlot - Visualize vibrational modes graphically

  Copyright (C) 2009 by David Lonie

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.openmolecules.net/>

  Some code is based on Open Babel
  For more information, see <http://openbabel.sourceforge.net/>

  The plotting interface is provided by a version of KPlotWidget that 
  is included with avogadro. The KPlotWidget, etc, files are copied 
  the kde tree.
  For more information, see <http://www.kde.org/>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation version 2 of the License.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
 ***********************************************************************/

#include "vibrationplot.h"

#include <QPushButton>
#include <QButtonGroup>
#include <QDebug>

#include <avogadro/molecule.h>
#include <openbabel/mol.h>
#include <openbabel/generic.h>

using namespace OpenBabel;
using namespace std;

namespace Avogadro {

  VibrationPlot::VibrationPlot( QWidget *parent, Qt::WindowFlags f ) : 
    QDialog( parent, f )
  {
    qDebug("VibrationPlot: Constructor called");
    ui.setupUi(this);
    qDebug("Are we getting this far?");
    // setting the limits for the plot
    ui.plot->setLimits( 4000.0, 400.0, 0.0, 1.0 );

    connect(ui.scaleSlider, SIGNAL(valueChanged(int)),
            this, SLOT(setScale(int)));
  }

  VibrationPlot::~VibrationPlot()
  {
    qDebug("VibrationPlot: Destructor called");
#warning dlonie: Do I need to delete anything?
  }

  void VibrationPlot::setMolecule(Molecule *molecule)
  {
    qDebug("VibrationPlot: setMolecule called");
    m_molecule = molecule;

    OBMol obmol = molecule->OBMol();
    m_vibrations = static_cast<OBVibrationData*>(obmol.GetData(OBGenericDataType::VibrationData));
    if (!m_vibrations) {
      //TODO
      return;
    }

    // OK, we have valid vibrations, so plot them
    vector<double> frequencies = m_vibrations->GetFrequencies();
    vector<double> intensities = m_vibrations->GetIntensities();

    //TODO: Actually plot the vibrations...
  }

  void VibrationPlot::accept()
  {
    qDebug("VibrationPlot: accept called");
    hide();
  }

  void VibrationPlot::reject()
  {
    qDebug("VibrationPlot: accept called");
    hide();
  }

  void VibrationPlot::setScale(int scale)
  {
    qDebug("VibrationPlot: setScale(int) called");
    emit scaleUpdated(scale / 2.0);
  }

}

#include "vibrationplot.moc"
