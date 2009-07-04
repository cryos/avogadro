/**********************************************************************
  SurfaceDialog - Dialog for generating cubes and meshes

  Copyright (C) 2009 Marcus D. Hanwell

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

#ifndef SURFACEDIALOG_H
#define SURFACEDIALOG_H

#include <QDialog>

#include "ui_surfacedialog.h"

#include <avogadro/cube.h>

#include <QList>
#include <Eigen/Core>

namespace Avogadro
{
  class GLWidget;
  class Primitive;
  class Molecule;
  class Engine;

  class SurfaceDialog : public QDialog
  {
    Q_OBJECT

  public:
    explicit SurfaceDialog(QWidget* parent = 0, Qt::WindowFlags f = 0);
    ~SurfaceDialog();
    void setMOs(int num);
    void setHOMO(int num);
    void setLUMO(int num);
    double stepSize();
    Engine * currentEngine();

    /**
     * @return the requested cube type.
     */
    Cube::Type cubeType();

    /**
     * @return the MO number (if applicable), or -1 if not.
     */
    int moNumber();

    /**
     * @return the requested cube color type.
     */
    Cube::Type cubeColorType();

    /**
     * @return the MO number (if applicable), or -1 if not.
     */
    int moColorNumber();

    /**
     * @return the iso value specified in the form.
     */
    double isoValue();

    void enableCalculation(bool enable);

  private:
    Ui::SurfaceDialog ui;
    const GLWidget *m_glwidget;
    const Molecule *m_molecule;

    // Lists of different properties we need to keep track of
    int m_moIndex; // The index of the MO entry
    int m_moColorIndex; // The index of the MO entry
    QList<Cube::Type> m_surfaceTypes;  // Mapping of the surface type combo
    QList<Cube::Type> m_colorTypes;    // Mapping of the color by combo
    QList<Engine *> m_engines;   // List of engines, as displayed in the combo

    // Update the engine list
    void updateEngines();

  public slots:
    void setGLWidget(const GLWidget *gl);
    void setMolecule(const Molecule *mol);
    void updateCubes(Primitive *p);

    void engineAdded(Engine *engine);
    void engineRemoved(Engine *engine);

  private slots:
    void calculateClicked();

    void surfaceComboChanged(int n);
    void colorByComboChanged(int n);

  signals:
    void calculate();
  };

} // End namespace Avogadro

#endif // SURFACEDIALOG_H
