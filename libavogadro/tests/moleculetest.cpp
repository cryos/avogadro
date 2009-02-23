/**********************************************************************
  Molecule - MoleculeTest class provides unit testing for the Molecule class

  Copyright (C) 2008 Marcus D. Hanwell

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

class MoleculeTest : public QObject
{
  Q_OBJECT

  private:
    Molecule *m_molecule; /// Molecule object for use by the test class.

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
     * Called before each test function is executed.
     */
    void init();

    /**
     * Called after every test function.
     */
    void cleanup();

    /**
     * Tests the addition of new Atom objects to the Molecule.
     */
    void addAtom();

    /**
     * Tests the setting of the Atom position.
     */
    void setAtomPos();

    /**
     * Tests the retrieval of the Atom position.
     */
    void atomPos();

    /**
     * Tests the access of Atom objects by index.
     */
    void atom();

    /**
     * Tests the access of Atom objects by index.
     */
    void atomById();

    /**
     * Tests the access of all Atom objects in Molecule.
     */
    void atoms();

    /**
     * Tests the deletion of Atom objects from the Molecule.
     */
    void removeAtom();

    /**
     * Tests the addition of new Bond objects to the Molecule.
     */
    void addBond();

    /**
     * Tests the deletion of Bond objects from the Molecule.
     */
    void removeBond();
};

void MoleculeTest::initTestCase()
{
  m_molecule = new Molecule;
}

void MoleculeTest::cleanupTestCase()
{
  delete m_molecule;
  m_molecule = 0;
}

void MoleculeTest::init()
{
}

void MoleculeTest::cleanup()
{
}

void MoleculeTest::addAtom()
{
  m_molecule->addAtom();
  QVERIFY(m_molecule->numAtoms() == 1);
  m_molecule->addAtom();
  QVERIFY(m_molecule->numAtoms() == 2);
}

void MoleculeTest::setAtomPos()
{
  m_molecule->setAtomPos(1, Vector3d(1.0, 2.0, 3.0));
}

void MoleculeTest::atomPos()
{
  const Vector3d *pos = m_molecule->atomPos(1);
  QCOMPARE(pos->x(), 1.0);
  QCOMPARE(pos->y(), 2.0);
  QCOMPARE(pos->z(), 3.0);
  // Atom positions that are not set should return a null pointer
  pos = m_molecule->atomPos(0);
  QCOMPARE(pos->x(), 0.0);
  // Atom positions that do not exist should also return a null pointer
  pos = m_molecule->atomPos(2);
  QVERIFY(pos == 0);
}

void MoleculeTest::atom()
{
  Atom *a = m_molecule->atom(0);
  QVERIFY(a != 0);
  a = m_molecule->atom(1);
  QVERIFY(a != 0);
}

void MoleculeTest::atoms()
{
  QList<Atom *> atoms = m_molecule->atoms();
  QCOMPARE(atoms.size(), 2);
}

void MoleculeTest::atomById()
{
  Atom *a = m_molecule->atomById(0);
  QVERIFY(a != 0);
  a = m_molecule->atomById(1);
  QVERIFY(a != 0);
}

void MoleculeTest::removeAtom()
{
  // Should now to two atoms in the Molecule with ids 0 and 1.
  Atom *a = m_molecule->atom(1);
  m_molecule->removeAtom(a);
  QVERIFY(m_molecule->numAtoms() == 1);
  m_molecule->removeAtom(0ul);
  QVERIFY(m_molecule->numAtoms() == 0);
}

void MoleculeTest::addBond()
{
  m_molecule->addBond();
  QVERIFY(m_molecule->numBonds() == 1);
  m_molecule->addBond();
  QVERIFY(m_molecule->numBonds() == 2);
}

void MoleculeTest::removeBond()
{
  // Should now to two atoms in the Molecule with ids 0 and 1.
  Bond *b = m_molecule->bond(1);
  m_molecule->removeBond(b);
  QVERIFY(m_molecule->numBonds() == 1);
  m_molecule->removeBond(0ul);
  QVERIFY(m_molecule->numBonds() == 0);
}

QTEST_MAIN(MoleculeTest)

#include "moc_moleculetest.cxx"
