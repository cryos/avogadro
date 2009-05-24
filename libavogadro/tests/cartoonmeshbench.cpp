/**********************************************************************
  Molecule - CartoonBench class provides benchmarking for the Molecule class

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

#include <QtTest>
#include <avogadro/molecule.h>
#include <avogadro/openbabelwrapper.h>
#include <avogadro/moleculefile.h>
#include <avogadro/mesh.h>
#include <avogadro/cartoonmeshgenerator.h>

#include <Eigen/Core>

#include <QCoreApplication>

using Avogadro::Molecule;
using Avogadro::MoleculeFile;
using Avogadro::Mesh;
using Avogadro::OpenbabelWrapper;
using Avogadro::CartoonMeshGenerator;

using Eigen::Vector3d;

class CartoonBench : public QObject
{
  Q_OBJECT

private:
  Molecule *m_molecule; /// Molecule object for use by the test class.

  void loadPdb(const QString &fileName);

private slots:
    /**
   * Called before the first test function is executed.
   */
  void initTestCase();

  /**
   * Called after the last test function is executed.
   */
  void cleanupTestCase();

  /**
   * Timing to add create mesh for 1DRF
   */
  void createMesh();

};

void CartoonBench::initTestCase()
{
  m_molecule = 0;
}

void CartoonBench::cleanupTestCase()
{
  delete m_molecule;
  m_molecule = 0;
}

void CartoonBench::createMesh()
{
  MoleculeFile *file = OpenbabelWrapper::readFile("1DRF.pdb");

  m_molecule = file->molecule(0);

  Mesh *mesh = m_molecule->addMesh();

  QBENCHMARK{
    CartoonMeshGenerator *generator = new CartoonMeshGenerator(m_molecule, mesh);
    generator->start();
    generator->wait();

    qDebug() << "numVertices = " << mesh->numVertices();
    qDebug() << "numNormals = " << mesh->numNormals();
  }
}

QTEST_MAIN(CartoonBench)

#include "moc_cartoonmeshbench.cxx"
