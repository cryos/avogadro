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

#include "orbitalextension.h"

#include "gaussianfchk.h"

#include <vector>
#include <avogadro/toolgroup.h>
#include <avogadro/molecule.h>
#include <avogadro/atom.h>
#include <avogadro/cube.h>
#include <Eigen/Core>

#include <QProgressDialog>
#include <QCoreApplication>

using namespace std;
using namespace OpenBabel;

namespace Avogadro
{

  using Eigen::Vector3d;
  using Eigen::Vector3i;

  OrbitalExtension::OrbitalExtension(QObject* parent) : Extension(parent),
    m_glwidget(0), m_orbitalDialog(0), m_molecule(0), m_basis(0)
  {
    QAction* action = new QAction(this);
    action->setText(tr("Import Molecular Orbitals..."));
    m_actions.append(action);
  }

  OrbitalExtension::~OrbitalExtension()
  {
    if (m_orbitalDialog)
    {
      delete m_orbitalDialog;
      m_orbitalDialog = 0;
    }
  }

  QList<QAction *> OrbitalExtension::actions() const
  {
    return m_actions;
  }

  QString OrbitalExtension::menuPath(QAction*) const
  {
    return tr("&Extensions");
  }

  QUndoCommand* OrbitalExtension::performAction(QAction *, GLWidget *widget)
  {
    m_glwidget = widget;
    if (!m_orbitalDialog)
    {
      m_orbitalDialog = new OrbitalDialog();
      connect(m_orbitalDialog, SIGNAL(fileName(QString)),
              this, SLOT(loadBasis(QString)));
      connect(m_orbitalDialog, SIGNAL(calculateMO(int)),
              this, SLOT(calculateMO(int)));
      m_orbitalDialog->show();
    }
    else
      m_orbitalDialog->show();
    return 0;
  }

  void OrbitalExtension::setMolecule(Molecule *molecule)
  {
    m_molecule = molecule;
  }

  void OrbitalExtension::loadBasis(QString fileName)
  {
    // Reset the basis class, then load up the checkpoint file
    if (fileName.isEmpty())
      return;

    if (m_basis)
      delete m_basis;
    m_basis = new BasisSet;
    GaussianFchk fchk(fileName, m_basis);
    // Add the molecule, perceive bonds etc
    m_molecule->clear();
    m_basis->addAtoms(m_molecule);
//    m_molecule->ConnectTheDots();
//    m_molecule->PerceiveBondOrders();
    // Debug output of the basis set
//    m_basis->outputAll();

    m_orbitalDialog->setMOs(m_basis->numMOs());
    for (int i = 0; i < m_basis->numMOs(); ++i)
    {
      if (m_basis->HOMO(i)) m_orbitalDialog->setHOMO(i);
      else if (m_basis->LUMO(i)) m_orbitalDialog->setLUMO(i);
    }

    // Now to set the default cube...
    Cube cube;
    double step = 0.1;
    cube.setLimits(m_molecule, step, 3.0);
    Vector3d min = cube.min();// / BOHR_TO_ANGSTROM;
    Vector3i dim = cube.dimensions();
    // Set these values on the form - they can then be altered by the user
    m_orbitalDialog->setCube(min, dim.x(), dim.y(), dim.z(), step);

    // Set the tool to navigate
    if (m_glwidget)
      m_glwidget->toolGroup()->setActiveTool("Navigate");
  }

  void OrbitalExtension::calculateMO(int n)
  {
    if (!m_basis)
      return;

    static const double BOHR_TO_ANGSTROM = 0.529177249;
    static const double ANGSTROM_TO_BOHR = 1.0/BOHR_TO_ANGSTROM;
    // Calculate MO n and add the cube to the molecule...
    n++; // MOs are 1 based, not 0 based...
    qDebug() << "Calculating MO" << n;
    double step = m_orbitalDialog->stepSize() * ANGSTROM_TO_BOHR;
    Vector3d origin = ANGSTROM_TO_BOHR * m_orbitalDialog->origin();
    Vector3i nSteps = m_orbitalDialog->steps();
    double x, y, z;

    // Debug output
    qDebug() << "Origin = " << origin.x() << origin.y() << origin.z()
             << "\nStep = " << step << ", nz = " << nSteps.z();

    // Set up a progress dialog
    QProgressDialog progress(tr("Calculating MO..."), tr("Abort Calculation"), 0,
                             nSteps.x(), m_orbitalDialog);
    progress.setWindowModality(Qt::WindowModal);
    progress.setValue(0);

    Cube *cube = m_molecule->newCube();
    cube->setLimits(origin * BOHR_TO_ANGSTROM, nSteps, step * BOHR_TO_ANGSTROM);

    for (int i = 0; i < nSteps.x(); ++i) {
      progress.setValue(i);
      if (progress.wasCanceled())
        break;
      // Give the event loop a chance...
      QCoreApplication::processEvents();
      x = origin.x() + double(i)*step;
      for (int j = 0; j < nSteps.y(); ++j) {
        y = origin.y() + double(j)*step;
        for (int k = 0; k < nSteps.z(); ++k) {
          z = origin.z() + double(k)*step;
          cube->setValue(i, j, k, m_basis->calculateMO(Vector3d(x, y, z), n));
        }
      }
    }
    progress.setValue(nSteps.x());

    cube->setName(QString(tr("MO ") + QString::number(n)));

    m_molecule->update();

    // Output the first line of the cube file too...
    for (int i = 0; i < nSteps.z(); ++i)
      qDebug() << i << cube->value(0, 0, i);

    qDebug() << "Cube generated...";
  }

} // End namespace Avogadro

#include "orbitalextension.moc"

Q_EXPORT_PLUGIN2(orbitalextension, Avogadro::OrbitalExtensionFactory)
