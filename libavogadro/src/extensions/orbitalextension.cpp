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
#include <openbabel/math/vector3.h>
#include <openbabel/griddata.h>
#include <openbabel/grid.h>

#include <QProgressDialog>
#include <QCoreApplication>

using namespace std;
using namespace OpenBabel;
using namespace Eigen;

namespace Avogadro
{

  OrbitalExtension::OrbitalExtension(QObject* parent) : Extension(parent),
    m_orbitalDialog(0), m_molecule(0), m_basis(0)
  {
    QAction* action = new QAction(this);
    action->setText(tr("Molecular Orbitals"));
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

  QUndoCommand* OrbitalExtension::performAction(QAction *, GLWidget *)
  {
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
    qDebug() << "Load file name" << fileName;
    if (m_basis)
      delete m_basis;
    m_basis = new BasisSet;
    GaussianFchk fchk(fileName, m_basis);
    // Add the molecule, perceive bonds etc
    m_molecule->Clear();
    m_basis->addAtoms(m_molecule);
    m_molecule->ConnectTheDots();
    m_molecule->PerceiveBondOrders();
    // Debug output of the basis set
//    m_basis->outputAll();

    m_orbitalDialog->setMOs(m_basis->numMOs());
    for (int i = 0; i < m_basis->numMOs(); ++i)
    {
      if (m_basis->HOMO(i)) m_orbitalDialog->setHOMO(i);
      else if (m_basis->LUMO(i)) m_orbitalDialog->setLUMO(i);
    }
  }

  void OrbitalExtension::calculateMO(int n)
  {
    static const double BOHR_TO_ANGSTROM = 0.529177249;
    // Calculate MO n and add the cube to the molecule...
    qDebug() << "Calculating MO" << n;
    OBFloatGrid grid;
    double step = 0.25;
    grid.Init(*m_molecule, step * BOHR_TO_ANGSTROM, 2.5);
    vector3 origin = grid.GetMin() / BOHR_TO_ANGSTROM;
    int nx = grid.GetXdim();
    int ny = grid.GetYdim();
    int nz = grid.GetZdim();
    double x, y, z;

    // Set up a progress dialog
    QProgressDialog progress("Calculating MO...", "Abort Calculation", 0, nx,
                             m_orbitalDialog);
    progress.setWindowModality(Qt::WindowModal);
    progress.setValue(0);

    vector<double> values;
    values.reserve(nx*ny*nz);

    for (int i = 0; i < nx; ++i)
    {
      progress.setValue(i);
      if (progress.wasCanceled())
        break;
      // Give the event loop a chance...
      QCoreApplication::processEvents();
      x = origin.x() + double(i)*step;
      for (int j = 0; j < ny; ++j)
      {
        y = origin.y() + double(j)*step;
        for (int k = 0; k < nz; ++k)
        {
          z = origin.z() + double(k)*step;
          values.push_back(m_basis->calculateMO(Vector3d(x, y, z), n));
        }
      }
    }
    progress.setValue(nx);

    // Bohr to Angstrom
    origin *= BOHR_TO_ANGSTROM;
    step *= BOHR_TO_ANGSTROM;
    // Make a grid and assign values to it
    OBGridData* obgrid = new OBGridData;
    obgrid->SetAttribute(QString("MO " + QString::number(n+1)).toStdString().c_str());
    obgrid->SetNumberOfPoints(nx, ny, nz);
    vector3 xa = vector3(step, 0.0, 0.0);
    vector3 ya = vector3(0.0, step, 0.0);
    vector3 za = vector3(0.0, 0.0, step);
    obgrid->SetLimits(origin, xa, ya, za);
    obgrid->SetUnit(OBGridData::BOHR);
    obgrid->SetValues(values);

    m_molecule->BeginModify();
    m_molecule->SetData(obgrid);
    m_molecule->EndModify();
    m_molecule->update();

    qDebug() << "Cube generated...";
  }

} // End namespace Avogadro

#include "orbitalextension.moc"

Q_EXPORT_PLUGIN2(orbitalextension, Avogadro::OrbitalExtensionFactory)
