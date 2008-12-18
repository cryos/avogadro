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

#ifndef ORBITALDIALOG_H
#define ORBITALDIALOG_H

#include <QDialog>
#include <QList>
#include <Eigen/Core>

#include "ui_orbitaldialog.h"

namespace Avogadro
{
  class GLWidget;
  class Primitive;
  class Molecule;

  class OrbitalDialog : public QDialog
  {
  Q_OBJECT

  public:
    explicit OrbitalDialog(QWidget* parent = 0, Qt::WindowFlags f = 0);
    ~OrbitalDialog();
    void setMOs(int num);
    void setHOMO(int num);
    void setLUMO(int num);
    void setCube(Eigen::Vector3d origin, int nx, int ny, int nz, double step);
    Eigen::Vector3d origin();
    Eigen::Vector3i steps();
    double stepSize();

  private:
    Ui::OrbitalDialog ui;
    const GLWidget *m_glwidget;
    const Molecule *m_molecule;
    double m_min, m_max;

  public Q_SLOTS:
    void calculate();
    void calculateAllClicked();
    void calculateDensityClicked();
    void enableCalculation(bool enable);
    void setCurrentTab(int n);
    void setGLWidget(const GLWidget *gl);
    void setMolecule(const Molecule *mol);
    void updateCubes(Primitive *p);

  private Q_SLOTS:
    /**
     * Slot responsible for updating the parameters of the cube when any of the
     * text edit boxes are changed.
     */
    void originChanged();

    void maxChanged();

    void stepsChanged();

    void stepSizeChanged();

    void orbitalComboChanged(int n);

    void isoSliderChanged(int);

    void isoEditChanged();

  Q_SIGNALS:
    void calculateMO(int MO);
    void calculateAll();
    void calculateDensity();
    void calculateMesh(unsigned int cube, double isoValue, int calc);

  };

} // End namespace Avogadro

#endif
