/**********************************************************************
  Molecule - MoleculeBench class provides benchmarking for the Molecule class

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
#include <avogadro/atom.h>
#include <avogadro/bond.h>

#include <Eigen/Core>

using Avogadro::Molecule;
using Avogadro::Atom;
using Avogadro::Bond;

using Eigen::Vector3d;

class MoleculeBench : public QObject
{
  Q_OBJECT

private:
  Molecule *m_molecule; /// Molecule object for use by the test class.

  /**
   * Prepare a starting molecule of known geometry for other tests.
   */
  void prepareMolecule();

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
   * Constructor test
   */
  void constructor();

  /**
   * Timing to add 10,000 atoms
   */
  void addAtoms();

  /**
   * Timing to remove 10,000 atoms
   */
  void removeAtoms();

  /**
   * Timing to add 10,000 atoms, set atomic number and position
   */
  void addAtoms2();

  /**
   * Timing to clear the molecule
   */
  void clear();

  /**
   * Destruct the molecule
   */
  void destructor();

  /**
   * Construct the molecule again
   */
  void constructor2();

};

void MoleculeBench::initTestCase()
{
  m_molecule = 0;
}

void MoleculeBench::cleanupTestCase()
{
  delete m_molecule;
  m_molecule = 0;
}

void MoleculeBench::constructor()
{
  delete m_molecule;
  m_molecule = 0;
  QBENCHMARK{
    m_molecule = new Molecule;
  }
}

void MoleculeBench::addAtoms()
{
  QBENCHMARK{
    for (int i = 0; i < 10000; ++i)
      m_molecule->addAtom();
  }
}

void MoleculeBench::removeAtoms()
{
  QBENCHMARK{
    for (int i = 10000; i >= 0; --i)
      m_molecule->removeAtom(i);
  }
}

void MoleculeBench::addAtoms2()
{
  QBENCHMARK{
    for (int i = 0; i < 25000; ++i) {
      Atom *a = m_molecule->addAtom();
      a->setAtomicNumber(6);
      a->setPos(Vector3d(1.0, 1.0, 1.0));
    }
  }
}

void MoleculeBench::clear()
{
  QBENCHMARK{
    m_molecule->clear();
  }
}

void MoleculeBench::destructor()
{
  QBENCHMARK{
    delete m_molecule;
    m_molecule = 0;
  }
}

void MoleculeBench::constructor2()
{
  delete m_molecule;
  m_molecule = 0;
  QBENCHMARK{
    m_molecule = new Molecule;
  }
}

QTEST_MAIN(MoleculeBench)

#include "moc_moleculebench.cxx"
