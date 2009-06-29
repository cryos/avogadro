/**********************************************************************
  SurfaceExtension - Extension for generating cubes and meshes

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

#ifndef SURAFCEEXTENSION_H
#define SURFACEEXTENSION_H

#include "surfacedialog.h"

#include <avogadro/extension.h>

#include <QVector>
#include <QList>

class QProgressDialog;

namespace Avogadro
{
  class Cube;
  class BasisSet;
  class SlaterSet;
  class Mesh;
  class MeshGenerator;
  class VdWSurface;
  class SurfaceDialog;

  class SurfaceExtension : public Extension
  {
    Q_OBJECT
    AVOGADRO_EXTENSION("Surfaces", tr("Surfaces"),
                       tr("Calculate molecular orbitals and other surfaces"))

  public:
    SurfaceExtension(QObject* parent = 0);
    virtual ~SurfaceExtension();

    virtual QList<QAction *> actions() const;

    virtual QString menuPath(QAction* action) const;

    virtual QUndoCommand* performAction(QAction *action, GLWidget *widget);

    void setMolecule(Molecule *molecule);

  private:
    QList<unsigned long> m_cubes; // These are the standard cubes
    QVector<unsigned long> m_moCubes; // These are the MO cubes
    int m_calculationPhase;        // The calculation phase
    GLWidget* m_glwidget;
    SurfaceDialog *m_surfaceDialog;
    QList<QAction *> m_actions;
    Molecule *m_molecule;
    BasisSet* m_basis;
    SlaterSet *m_slater;
    QString m_loadedFileName;
    QProgressDialog *m_progress;

    Mesh *m_mesh1, *m_mesh2;
    MeshGenerator *m_meshGen1;
    MeshGenerator *m_meshGen2;

    VdWSurface *m_VdWsurface;

    Cube *m_cube;
    Cube *m_cubeColor;

    //! Load the appropriate basis set (if possible)
    bool loadBasis();

    //! Calculate the ESP from the partial charges of the atoms on the supplied
    //! Mesh object.
    void calculateESP(Mesh *mesh);

    //! Convenience function - creates a new cube with the correct dimensions.
    Cube * newCube();

    //! Calculate the VdW cube
    void calculateVdW(Cube *cube);

    //! Calculate an MO cube
    void calculateMo(Cube *cube, int mo);

    //! Calculate electron density cube
    void calculateElectronDensity(Cube *cube);

    //! Calculate a mesh isosurface for the given cube
    void calculateMesh(Cube *cube, double isoValue);

    /**
     * Figure out which kind of calculation is required.
     * @param type the type of cube to be calculated.
     * @param mo the MO number, default of -1 is invalid/ignored.
     * @param calculateCube true if the cube needs to be calculated.
     * @return the relevant Cube as requested in the form.
     */
    Cube * startCubeCalculation(SurfaceDialog::Type type, int mo,
                                bool &calculateCube);

  private slots:
    /**
     * New calculate function - queries the dialog and calculates the cubes and
     * the meshes. This is much more demand based.
     */
    void calculate();

    /**
     * This is called once the calculation is complete - check for more
     * calculations, clean up once complete.
     */
    void calculateDone();

    /**
     * This is called once the calculation is canceled - clean up.
     */
    void calculateCanceled();

  };

  class SurfaceExtensionFactory : public QObject, public PluginFactory
  {
      Q_OBJECT
      Q_INTERFACES(Avogadro::PluginFactory)
      AVOGADRO_EXTENSION_FACTORY(SurfaceExtension)
  };

} // End namespace Avogadro

#endif
