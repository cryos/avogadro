/**********************************************************************
  OrbitalExtension - Extension for generating cubes and meshes

  Copyright (C) 2008-2009 Marcus D. Hanwell

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

#ifndef ORBITALEXTENSION_H
#define ORBITALEXTENSION_H

#include "basisset.h"
#include "orbitaldialog.h"

#include <avogadro/glwidget.h>
#include <avogadro/extension.h>

class QProgressDialog;
class QTime;

namespace Avogadro
{
  class BasisSet;
  class SlaterSet;
  class Mesh;
  class MeshGenerator;
  class VdWSurface;

  class OrbitalExtension : public Extension
  {
  Q_OBJECT

  public:
    OrbitalExtension(QObject* parent = 0);
    virtual ~OrbitalExtension();

    virtual QString name() const { return QObject::tr("Surfaces"); }
    virtual QString description() const
    {
      return QObject::tr("Create and edit cubes and surfaces.");
    }

    virtual QList<QAction *> actions() const;

    virtual QString menuPath(QAction* action) const;

    virtual QUndoCommand* performAction(QAction *action, GLWidget *widget);

    void setMolecule(Molecule *molecule);

    void setDefaultCube();

    bool loadBasis();

  private:
    GLWidget* m_glwidget;
    OrbitalDialog* m_orbitalDialog;
    QList<QAction *> m_actions;
    Molecule *m_molecule;
    BasisSet* m_basis;
    SlaterSet *m_slater;
    QString m_loadedFileName;
    QProgressDialog *m_progress;
    QTime *m_timer;
    unsigned int m_currentMO;
    double m_stepSize;
    Eigen::Vector3d m_origin;
    Eigen::Vector3i m_steps;

    Mesh *m_mesh1, *m_mesh2;
    MeshGenerator *m_meshGen1;
    MeshGenerator *m_meshGen2;

    VdWSurface *m_VdWsurface;

    void calculateMO(int MO, const Eigen::Vector3d &origin,
                     const Eigen::Vector3i &steps, double stepSize);

  private Q_SLOTS:
    void calculateMO(int n);
    void calculateAll();
    void calculateDensity();
    void calculationDone();
    void calculation2Done();
    void calculationCanceled();
    void calculation2Canceled();
    void slaterDone();
    void slaterCanceled();

    /** Mesh rendering. */
    void generateMesh(int cube, double isoValue, int calc);
    void meshGenerated();

    /** VdW calculations/rendering */
    void calculateVdWCube();
    void calculateVdWDone();
    void calculateVdWCanceled();
    void generateVdWMesh(int cube, double isoValue);
    void VdWMeshGenerated();

  };

  class OrbitalExtensionFactory : public QObject, public PluginFactory
  {
      Q_OBJECT
      Q_INTERFACES(Avogadro::PluginFactory)

      AVOGADRO_EXTENSION_FACTORY(OrbitalExtension,
          tr("Orbital Extension"),
          tr("Extension for calculating orbitals."))

  };

} // End namespace Avogadro

#endif
