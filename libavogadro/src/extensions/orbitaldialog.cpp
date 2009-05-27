/**********************************************************************
  OrbitalExtension - Extension for generating orbital cubes

  Copyright (C) 2008-2009 Marcus D. Hanwell

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.openmolecules.net/>

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
#include <avogadro/engine.h>

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
    connect(ui.isoSlider, SIGNAL(valueChanged(int)),
            this, SLOT(isoSliderChanged(int)));
    connect(ui.calculateMesh, SIGNAL(clicked()),
            this, SLOT(calculateMeshClicked()));

    // VdW stuff
    connect(ui.calculateVdW, SIGNAL(clicked()),
            this, SLOT(calculateVdWCubeClicked()));
    connect(ui.surfaceDistance, SIGNAL(editingFinished()),
            this, SLOT(VdWEditChanged()));
    connect(ui.surfaceSlider, SIGNAL(valueChanged(int)),
            this, SLOT(VdWSliderChanged(int)));
    connect(ui.calculateVdWMesh, SIGNAL(clicked()),
            this, SLOT(calculateVdWMeshClicked()));
  }

  OrbitalDialog::~OrbitalDialog()
  {
  }

  void OrbitalDialog::setMOs(int num)
  {
    ui.orbitalCombo->setEnabled(true);
    ui.orbitalCombo->clear();
    for (int i = 1; i <= num; ++i)
      ui.orbitalCombo->addItem(tr("MO %L1", "Molecular Orbital").arg(i));
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
    QString format("%L1");
    ui.originX->setText(format.arg(tmp.x()));
    ui.originY->setText(format.arg(tmp.y()));
    ui.originZ->setText(format.arg(tmp.z()));
    ui.maxX->setText(format.arg(tmp.x()+step*nx));
    ui.maxY->setText(format.arg(tmp.y()+step*ny));
    ui.maxZ->setText(format.arg(tmp.z()+step*nz));
    ui.stepsX->setText(format.arg(nx));
    ui.stepsY->setText(format.arg(ny));
    ui.stepsZ->setText(format.arg(nz));
    ui.stepSize->setText(format.arg(step));
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

  Engine * OrbitalDialog::currentEngine()
  {
    foreach (Engine *engine, m_glwidget->engines()) {
      if (ui.tabWidget->currentIndex() == 1) {
        if (engine->alias() == ui.engineCombo->currentText())
          return engine;
      }
      else if (ui.tabWidget->currentIndex() == 2) {
        if (engine->alias() == ui.surfaceEngineCombo->currentText())
          return engine;
      }
    }
    return 0;
  }

  int OrbitalDialog::colorBy()
  {
    // Returns the cube to color by. 0 means none, 1 means estimated partial
    // charge and >1 means an actual cube (find by number returned -2)
    if (ui.tabWidget->currentIndex() == 1)
      return ui.colorCubeCombo->currentIndex();
    else if (ui.tabWidget->currentIndex() == 2)
      return ui.surfaceColorCubeCombo->currentIndex();
    else
      return 0;
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
      if (engine->identifier() == "Orbitals")
        ui.engineCombo->addItem(engine->alias());
      if (engine->identifier() == "Surface")
        ui.surfaceEngineCombo->addItem(engine->alias());
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
    int cubeIndex = ui.cubeCombo->currentIndex();
    int colorCubeIndex = ui.colorCubeCombo->currentIndex();
    int surfaceCubeIndex = ui.surfaceCubeCombo->currentIndex();
    int surfaceColorCubeIndex = ui.surfaceColorCubeCombo->currentIndex();
    ui.cubeCombo->clear();
    ui.colorCubeCombo->clear();
    ui.colorCubeCombo->addItem(tr("None"));
    ui.colorCubeCombo->addItem(tr("ESP"));
    ui.surfaceCubeCombo->clear();
    ui.surfaceColorCubeCombo->clear();
    ui.surfaceColorCubeCombo->addItem(tr("None"));
    ui.surfaceColorCubeCombo->addItem(tr("ESP"));
    foreach (Cube *cube, m_molecule->cubes()) {
      ui.cubeCombo->addItem(cube->name());
      ui.colorCubeCombo->addItem(cube->name());
      ui.surfaceCubeCombo->addItem(cube->name());
      ui.surfaceColorCubeCombo->addItem(cube->name());
    }
    ui.calculateMesh->setEnabled(true);
    ui.calculateVdWMesh->setEnabled(true);
    // Set the indices of the combos
    ui.cubeCombo->setCurrentIndex(cubeIndex>-1 ? cubeIndex : 0);
    ui.colorCubeCombo->setCurrentIndex(colorCubeIndex>-1 ? colorCubeIndex : 0);
    ui.surfaceCubeCombo->setCurrentIndex(surfaceCubeIndex>-1 ?
                                         surfaceCubeIndex : 0);
    ui.surfaceColorCubeCombo->setCurrentIndex(surfaceColorCubeIndex>-1 ?
                                              surfaceColorCubeIndex : 0);
  }

  void OrbitalDialog::originChanged()
  {
    // The origin has been changed - the size of the cube should be contracted
    Vector3d delta(ui.maxX->text().toDouble() - ui.originX->text().toDouble(),
                   ui.maxY->text().toDouble() - ui.originY->text().toDouble(),
                   ui.maxZ->text().toDouble() - ui.originZ->text().toDouble());
    double step = ui.stepSize->text().toDouble();
    Vector3i steps = (delta / step).cast<int>();
    Vector3d max(ui.originX->text().toDouble() + step * steps.x(),
                 ui.originY->text().toDouble() + step * steps.y(),
                 ui.originZ->text().toDouble() + step * steps.z());
    QString format("%L1");
    ui.maxX->setText(format.arg(max.x()));
    ui.maxY->setText(format.arg(max.y()));
    ui.maxZ->setText(format.arg(max.z()));
    ui.stepsX->setText(format.arg(steps.x()));
    ui.stepsY->setText(format.arg(steps.y()));
    ui.stepsZ->setText(format.arg(steps.z()));
  }

  void OrbitalDialog::maxChanged()
  {
    // The max has been changed - the size of the cube should be contracted
    Vector3d delta(ui.maxX->text().toDouble() - ui.originX->text().toDouble(),
                   ui.maxY->text().toDouble() - ui.originY->text().toDouble(),
                   ui.maxZ->text().toDouble() - ui.originZ->text().toDouble());
    double step = ui.stepSize->text().toDouble();
    Vector3i steps = (delta / step).cast<int>();
    Vector3d max(ui.originX->text().toDouble() + step * steps.x(),
                 ui.originY->text().toDouble() + step * steps.y(),
                 ui.originZ->text().toDouble() + step * steps.z());
    QString format("%L1");
    ui.maxX->setText(format.arg(max.x()));
    ui.maxY->setText(format.arg(max.y()));
    ui.maxZ->setText(format.arg(max.z()));
    ui.stepsX->setText(format.arg(steps.x()));
    ui.stepsY->setText(format.arg(steps.y()));
    ui.stepsZ->setText(format.arg(steps.z()));
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
    QString format("%L1");
    ui.maxX->setText(format.arg(max.x()));
    ui.maxY->setText(format.arg(max.y()));
    ui.maxZ->setText(format.arg(max.z()));
    ui.stepsX->setText(format.arg(steps.x()));
    ui.stepsY->setText(format.arg(steps.y()));
    ui.stepsZ->setText(format.arg(steps.z()));
  }

  void OrbitalDialog::stepSizeChanged()
  {
    Vector3d delta(ui.maxX->text().toDouble() - ui.originX->text().toDouble(),
                   ui.maxY->text().toDouble() - ui.originY->text().toDouble(),
                   ui.maxZ->text().toDouble() - ui.originZ->text().toDouble());
    double step = ui.stepSize->text().toDouble();
    Vector3i steps = (delta / step).cast<int>();
    Vector3d max(ui.originX->text().toDouble() + step * steps.x(),
                 ui.originY->text().toDouble() + step * steps.y(),
                 ui.originZ->text().toDouble() + step * steps.z());
    QString format("%L1");
    ui.maxX->setText(format.arg(max.x()));
    ui.maxY->setText(format.arg(max.y()));
    ui.maxZ->setText(format.arg(max.z()));
    ui.stepsX->setText(format.arg(steps.x()));
    ui.stepsY->setText(format.arg(steps.y()));
    ui.stepsZ->setText(format.arg(steps.z()));
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
      QString format("%L1");
      ui.cubeLow->setText(format.arg(m_min));
      ui.cubeHigh->setText(format.arg(m_max));
    }
  }

  void OrbitalDialog::isoSliderChanged(int n)
  {
    ui.isoValue->setText(QString("%L1")
                         .arg(n/99.0 * (m_max-m_min) + m_min ));
  }

  void OrbitalDialog::isoEditChanged()
  {
    ui.isoSlider->setValue(
         (ui.isoValue->text().toDouble()-m_min) / (m_max-m_min) * 99.0 );
  }

  void OrbitalDialog::calculateMeshClicked()
  {
    emit calculateMesh(ui.cubeCombo->currentIndex(),
                       ui.isoValue->text().toDouble(),
                       0);
  }

  void OrbitalDialog::calculateVdWCubeClicked()
  {
    emit calculateVdWCube();
  }

  void OrbitalDialog::VdWSliderChanged(int n)
  {
    ui.surfaceDistance->setText(QString("%L1").arg(n/25.0));
  }

  void OrbitalDialog::VdWEditChanged()
  {
    ui.surfaceSlider->setValue(
         (ui.surfaceDistance->text().toDouble()) * 25.0 );
  }

  void OrbitalDialog::calculateVdWMeshClicked()
  {
    emit calculateVdWMesh(ui.surfaceCubeCombo->currentIndex(),
                          ui.surfaceDistance->text().toDouble());
  }

} // End namespace Avogadro

