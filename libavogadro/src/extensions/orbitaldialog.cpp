/**********************************************************************
  OrbitalExtension - Extension for generating orbital cubes

  Copyright (C) 2008 Marcus D. Hanwell

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.sourceforge.net/>

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

#include "orbitaldialog.h"

#include <QFileDialog>
#include <QString>
#include <QDebug>

namespace Avogadro
{
  using Eigen::Vector3d;

  OrbitalDialog::OrbitalDialog(QWidget* parent, Qt::WindowFlags f)
    : QDialog(parent, f)
  {
    ui.setupUi(this);
    connect(ui.loadFile, SIGNAL(clicked()), this, SLOT(loadFile()));
    connect(ui.calculate, SIGNAL(clicked()),
            this, SLOT(calculate()));
  }

  OrbitalDialog::~OrbitalDialog()
  {
  }

  void OrbitalDialog::setMOs(int num)
  {
    ui.orbitalCombo->clear();
    for (int i = 1; i <= num; ++i)
      ui.orbitalCombo->addItem("MO " + QString::number(i));
  }

  void OrbitalDialog::setHOMO(int n)
  {
    ui.orbitalCombo->setItemText(n, ui.orbitalCombo->itemText(n) + " (HOMO)");
    ui.orbitalCombo->setCurrentIndex(n);
  }

  void OrbitalDialog::setLUMO(int n)
  {
    ui.orbitalCombo->setItemText(n, ui.orbitalCombo->itemText(n) + " (LUMO)");
  }

  void OrbitalDialog::setCube(Eigen::Vector3d origin, int nx, int ny, int nz,
                              double step)
  {
    ui.originX->setText(QString::number(origin.x()));
    ui.originY->setText(QString::number(origin.y()));
    ui.originZ->setText(QString::number(origin.z()));
    ui.stepsX->setText(QString::number(nx));
    ui.stepsY->setText(QString::number(ny));
    ui.stepsZ->setText(QString::number(nz));
    ui.stepSize->setText(QString::number(step));
  }

  Vector3d OrbitalDialog::origin()
  {
    return Vector3d(QString(ui.originX->text()).toDouble(),
                    QString(ui.originY->text()).toDouble(),
                    QString(ui.originZ->text()).toDouble());
  }
  
  QList<int> OrbitalDialog::steps()
  {
    QList<int> tmp;
    tmp.push_back(QString(ui.stepsX->text()).toInt());
    tmp.push_back(QString(ui.stepsY->text()).toInt());
    tmp.push_back(QString(ui.stepsZ->text()).toInt());
    return tmp;
  }
  double OrbitalDialog::stepSize()
  {
    return QString(ui.stepSize->text()).toDouble();
  }

  void OrbitalDialog::loadFile()
  {
    // Load a file
    QString file = QFileDialog::getOpenFileName(this,
      tr("Open quantum file"), ui.fileName->text(),
      tr("Quantum files (*.fchk *.gamout *.g03)"));
    ui.fileName->setText(file);
    emit fileName(file);
  }

  void OrbitalDialog::calculate()
  {
    emit calculateMO(ui.orbitalCombo->currentIndex());
  }

} // End namespace Avogadro

#include "orbitaldialog.moc"
