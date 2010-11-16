/**********************************************************************
  ConformerPlotExtension

  Copyright (C) 2010 David C. Lonie

  This library is free software; you can redistribute it and/or modify
  it under the terms of the GNU Library General Public License as
  published by the Free Software Foundation; either version 2.1 of the
  License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
 ***********************************************************************/

#include "conformerplotdialog.h"

#include <avogadro/molecule.h>
#include <avogadro/plotobject.h>

#include <vector>

using namespace Avogadro;

namespace ConformerPlot {

  ConformerPlotDialog::ConformerPlotDialog(QWidget *parent, Qt::WindowFlags f ) :
    QDialog( parent, f )
  {
    ui.setupUi(this);

    connect(ui.plot, SIGNAL(pointClicked(PlotPoint*)),
            this, SLOT(updateConformer(PlotPoint*)));
    connect(ui.plot, SIGNAL(pointClicked(PlotPoint*)),
            ui.plot, SLOT(clearAndSelectPoint(PlotPoint*)));
  }

  void ConformerPlotDialog::refresh(Avogadro::Molecule *mol)
  {
    ui.plot->resetPlot();

    m_molecule = mol;
    if (!m_molecule) return;

    std::vector<double> energies = m_molecule->energies();

    // Set the default energy to 0 in case there aren't any
    if (energies.size() == 0) energies.push_back(0.0);

    // find the min/max energies and create the plot object
    PlotObject *data = new PlotObject (Qt::red, PlotObject::Points, 2);
    double min, max, cur;
    min = max = energies.at(0);
    for (int i = 0; i < energies.size(); i++) {
      cur = energies.at(i);
      if (cur < min) min = cur;
      if (cur > max) max = cur;
      data->addPoint(i+1, cur, QString::number(cur));
    }

    double spread = max - min;
    double ext = spread * 0.05;

    ui.plot->setDefaultLimits(0,
                              energies.size() + 1,
                              min - ext,
                              max + ext);

    ui.plot->setJailedInDefaults(true);

    ui.plot->setAntialiasing(true);
    ui.plot->setMouseTracking(true);
    ui.plot->axis(PlotWidget::BottomAxis)->setLabel(tr("Conformer number"));
    ui.plot->axis(PlotWidget::LeftAxis)->setLabel(tr("Energy (kJ/mol)"));
    ui.plot->addPlotObject(data);
  }

  void ConformerPlotDialog::updateConformer(PlotPoint *p)
  {
    unsigned int index = floor(p->x()+0.5) - 1;
    m_molecule->setConformer(index);
    m_molecule->update();
  }

}

#include "conformerplotdialog.moc"
