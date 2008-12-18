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

#include <avogadro/glwidget.h>
#include <avogadro/molecule.h>
#include <avogadro/cube.h>

#include <QFileDialog>
#include <QString>
#include <QDebug>

namespace Avogadro
{
  using Eigen::Vector3d;
  using Eigen::Vector3i;

  OrbitalDialog::OrbitalDialog(QWidget* parent, Qt::WindowFlags f)
    : QDialog(parent, f), m_glwidget(0), m_molecule(0)
  {
    ui.setupUi(this);
    // Hide the advanced stuff for now
    //ui.frame->hide();
    //resize(325, 100);
    connect(ui.calculate, SIGNAL(clicked()),
            this, SLOT(calculate()));
    connect(ui.calculateAll, SIGNAL(clicked()),
            this, SLOT(calculateAllClicked()));
    connect(ui.calculateDensity, SIGNAL(clicked()),
            this, SLOT(calculateDensityClicked()));

    // Connect up the various edits to their slots - electron cube calculation
    connect(ui.originX, SIGNAL(editingFinished()),
            this, SLOT(originChanged()));
    connect(ui.originY, SIGNAL(editingFinished()),
            this, SLOT(originChanged()));
    connect(ui.originZ, SIGNAL(editingFinished()),
            this, SLOT(originChanged()));
    connect(ui.maxX, SIGNAL(editingFinished()),
            this, SLOT(maxChanged()));
    connect(ui.maxY, SIGNAL(editingFinished()),
            this, SLOT(maxChanged()));
    connect(ui.maxZ, SIGNAL(editingFinished()),
            this, SLOT(maxChanged()));
    connect(ui.stepsX, SIGNAL(editingFinished()),
            this, SLOT(stepsChanged()));
    connect(ui.stepsY, SIGNAL(editingFinished()),
            this, SLOT(stepsChanged()));
    connect(ui.stepsZ, SIGNAL(editingFinished()),
            this, SLOT(stepsChanged()));
    connect(ui.stepSize, SIGNAL(editingFinished()),
            this, SLOT(stepSizeChanged()));

    // Mesh calculation
    connect(ui.cubeCombo, SIGNAL(currentIndexChanged(int)),
            this, SLOT(orbitalComboChanged(int)));
    connect(ui.isoValue, SIGNAL(editingFinished()),
            this, SLOT(isoEditChanged()));
    connect(ui.isoSlider, SIGNAL(sliderMoved(int)),
            this, SLOT(isoSliderChanged(int)));
  }

  OrbitalDialog::~OrbitalDialog()
  {
  }

  void OrbitalDialog::setMOs(int num)
  {
    ui.orbitalCombo->setEnabled(true);
    ui.orbitalCombo->clear();
    for (int i = 1; i <= num; ++i)
      ui.orbitalCombo->addItem("MO " + QString::number(i));
    ui.calculate->setEnabled(true);
    ui.calculateAll->setEnabled(true);
    ui.calculateDensity->setEnabled(true);
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
    // Just do two decimal places
    Vector3d tmp = (origin * 100).cast<int>().cast<double>() / 100.0;
    ui.originX->setText(QString::number(tmp.x()));
    ui.originY->setText(QString::number(tmp.y()));
    ui.originZ->setText(QString::number(tmp.z()));
    ui.maxX->setText(QString::number(tmp.x()+step*nx));
    ui.maxY->setText(QString::number(tmp.y()+step*ny));
    ui.maxZ->setText(QString::number(tmp.z()+step*nz));
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

  Vector3i OrbitalDialog::steps()
  {
    Vector3i tmp(QString(ui.stepsX->text()).toInt()+1,
                 QString(ui.stepsY->text()).toInt()+1,
                 QString(ui.stepsZ->text()).toInt()+1);
    return tmp;
  }
  double OrbitalDialog::stepSize()
  {
    return QString(ui.stepSize->text()).toDouble();
  }

  void OrbitalDialog::calculate()
  {
    emit calculateMO(ui.orbitalCombo->currentIndex());
  }

  void OrbitalDialog::calculateAllClicked()
  {
    emit calculateAll();
  }

  void OrbitalDialog::calculateDensityClicked()
  {
    emit calculateDensity();
  }

  void OrbitalDialog::enableCalculation(bool enable)
  {
    ui.calculate->setEnabled(enable);
    ui.calculateAll->setEnabled(enable);
    ui.calculateDensity->setEnabled(enable);
  }

  void OrbitalDialog::setCurrentTab(int n)
  {
    ui.tabWidget->setCurrentIndex(n);
  }

  void OrbitalDialog::setGLWidget(const GLWidget *gl)
  {
    m_glwidget = gl;
    ui.engineCombo->clear();
    foreach (Engine *engine, m_glwidget->engines()) {
      if (engine->name() == "Orbitals")
        ui.engineCombo->addItem(engine->alias());
    }
  }

  void OrbitalDialog::setMolecule(const Molecule *mol)
  {
    if (m_molecule)
      disconnect(m_molecule, 0, this, 0);
    m_molecule = mol;
    updateCubes(0);
    connect(m_molecule, SIGNAL(primitiveAdded(Primitive *)),
            this, SLOT(updateCubes(Primitive *)));
    connect(m_molecule, SIGNAL(primitiveUpdated(Primitive *)),
            this, SLOT(updateCubes(Primitive *)));
    connect(m_molecule, SIGNAL(primitiveRemoved(Primitive *)),
            this, SLOT(updateCubes(Primitive *)));
  }

  void OrbitalDialog::updateCubes(Primitive *)
  {
    ui.cubeCombo->clear();
    qDebug() << "updateCubes()";
    foreach (Cube *cube, m_molecule->cubes()) {
      ui.cubeCombo->addItem(cube->name());
    }
  }

  void OrbitalDialog::originChanged()
  {
    // The origin has been changed - the size of the cube should be contracted
    Vector3d delta(ui.maxX->text().toDouble() - ui.originX->text().toDouble(),
                   ui.maxY->text().toDouble() - ui.originY->text().toDouble(),
                   ui.maxZ->text().toDouble() - ui.originZ->text().toDouble());
    double step = ui.stepSize->text().toDouble();
    Vector3i steps = delta / step;
    Vector3d max(ui.originX->text().toDouble() + step * steps.x(),
                 ui.originY->text().toDouble() + step * steps.y(),
                 ui.originZ->text().toDouble() + step * steps.z());
    ui.maxX->setText(QString::number(max.x()));
    ui.maxY->setText(QString::number(max.y()));
    ui.maxZ->setText(QString::number(max.z()));
    ui.stepsX->setText(QString::number(steps.x()));
    ui.stepsY->setText(QString::number(steps.y()));
    ui.stepsZ->setText(QString::number(steps.z()));
  }

  void OrbitalDialog::maxChanged()
  {
    // The max has been changed - the size of the cube should be contracted
    Vector3d delta(ui.maxX->text().toDouble() - ui.originX->text().toDouble(),
                   ui.maxY->text().toDouble() - ui.originY->text().toDouble(),
                   ui.maxZ->text().toDouble() - ui.originZ->text().toDouble());
    double step = ui.stepSize->text().toDouble();
    Vector3i steps = delta / step;
    Vector3d max(ui.originX->text().toDouble() + step * steps.x(),
                 ui.originY->text().toDouble() + step * steps.y(),
                 ui.originZ->text().toDouble() + step * steps.z());
    ui.maxX->setText(QString::number(max.x()));
    ui.maxY->setText(QString::number(max.y()));
    ui.maxZ->setText(QString::number(max.z()));
    ui.stepsX->setText(QString::number(steps.x()));
    ui.stepsY->setText(QString::number(steps.y()));
    ui.stepsZ->setText(QString::number(steps.z()));
  }

  void OrbitalDialog::stepsChanged()
  {
    // The steps have been changed - the size of the cube should be contracted
    double step = ui.stepSize->text().toDouble();
    Vector3i steps(ui.stepsX->text().toInt(),
                   ui.stepsY->text().toInt(),
                   ui.stepsZ->text().toInt());
    Vector3d max(ui.originX->text().toDouble() + step * steps.x(),
                 ui.originY->text().toDouble() + step * steps.y(),
                 ui.originZ->text().toDouble() + step * steps.z());
    ui.maxX->setText(QString::number(max.x()));
    ui.maxY->setText(QString::number(max.y()));
    ui.maxZ->setText(QString::number(max.z()));
    ui.stepsX->setText(QString::number(steps.x()));
    ui.stepsY->setText(QString::number(steps.y()));
    ui.stepsZ->setText(QString::number(steps.z()));
  }

  void OrbitalDialog::stepSizeChanged()
  {
    Vector3d delta(ui.maxX->text().toDouble() - ui.originX->text().toDouble(),
                   ui.maxY->text().toDouble() - ui.originY->text().toDouble(),
                   ui.maxZ->text().toDouble() - ui.originZ->text().toDouble());
    double step = ui.stepSize->text().toDouble();
    Vector3i steps = delta / step;
    Vector3d max(ui.originX->text().toDouble() + step * steps.x(),
                 ui.originY->text().toDouble() + step * steps.y(),
                 ui.originZ->text().toDouble() + step * steps.z());
    ui.maxX->setText(QString::number(max.x()));
    ui.maxY->setText(QString::number(max.y()));
    ui.maxZ->setText(QString::number(max.z()));
    ui.stepsX->setText(QString::number(steps.x()));
    ui.stepsY->setText(QString::number(steps.y()));
    ui.stepsZ->setText(QString::number(steps.z()));
  }

  void OrbitalDialog::orbitalComboChanged(int n)
  {
    Cube *cube = m_molecule->cube(n);
    if (cube) {
      m_min = cube->minValue();
      m_max = cube->maxValue();
      if (m_min < 0) {
        if (m_min * -1.0 > m_max)
          m_max = m_min * -1.0;
        m_min = 0.0;
      }
      ui.cubeLow->setText(QString::number(m_min));
      ui.cubeHigh->setText(QString::number(m_max));
    }
    else
      qDebug() << "OrbitalDialog failed - cube = 0.";
  }

  void OrbitalDialog::isoSliderChanged(int n)
  {
    ui.isoValue->setText(QString::number(
                         n/99.0 * (m_max-m_min) + m_min ));
  }

  void OrbitalDialog::isoEditChanged()
  {
    ui.isoSlider->setValue(
         (ui.isoValue->text().toDouble()-m_min) / (m_max-m_min) * 99.0 );
  }

} // End namespace Avogadro

#include "orbitaldialog.moc"
