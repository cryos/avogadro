/**********************************************************************
  Molecule - MoleculeTest class provides unit testing for the Molecule class

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

  /**
   * Tests the calculation of the center of the Molecule.
   */
  void center();

  /**
   * Tests the calculation of the normal vector of the Molecule.
   */
  void normalVector();

  /**
   * Tests the calculation of the radius of the Molecule.
   */
  void radius();

  /**
   * Tests the determination of the farthest atom.
   */
  void farthestAtom();

  /**
   * Tests the translation of the Molecule.
   */
  void translate();
};

void MoleculeTest::prepareMolecule()
{
  Atom *a1 = m_molecule->addAtom();
  a1->setPos(Vector3d(0.0, 0.0, 0.0));
  Atom *a2 = m_molecule->addAtom();
  a2->setPos(Vector3d(1.5, 0.0, 0.0));
  Atom *a3 = m_molecule->addAtom();
  a3->setPos(Vector3d(0.0, 1.5, 0.0));
  Atom *a4 = m_molecule->addAtom();
  a4->setPos(Vector3d(0.0, 0.0, 1.5));
  Bond *b1 = m_molecule->addBond();
  b1->setAtoms(a1->id(), a2->id(), 1);
  Bond *b2 = m_molecule->addBond();
  b2->setAtoms(a2->id(), a3->id(), 1);
  Bond *b3 = m_molecule->addBond();
  b3->setAtoms(a3->id(), a4->id(), 1);
}

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

void MoleculeTest::center()
{
  prepareMolecule();
  Vector3d center = m_molecule->center();
  QCOMPARE(center.x(), 0.375);
  QCOMPARE(center.y(), 1.5 / 4.0);
  QCOMPARE(center.z(), 1.5 / 4.0);
}

void MoleculeTest::normalVector()
{
  Vector3d normal = m_molecule->normalVector();
  QCOMPARE(normal.x(), -0.57735026919);
  QCOMPARE(normal.y(), -0.57735026919);
  QCOMPARE(normal.z(), -0.57735026919);
}

void MoleculeTest::radius()
{
  QCOMPARE(m_molecule->radius(), 1.24373429638327494);
}

void MoleculeTest::farthestAtom()
{
  QVERIFY(m_molecule->farthestAtom()->index() == 1);
}

void MoleculeTest::translate()
{
  m_molecule->translate(Vector3d(1.0, 1.1, 1.2));
  QCOMPARE(m_molecule->atom(0)->pos()->x(), 1.0);
  QCOMPARE(m_molecule->atom(0)->pos()->y(), 1.1);
  QCOMPARE(m_molecule->atom(0)->pos()->z(), 1.2);
  QCOMPARE(m_molecule->atom(1)->pos()->x(), 2.5);
  // Check the center was correctly updated
  QCOMPARE(m_molecule->center().x(), 1.5 / 4.0 + 1.0);
  QCOMPARE(m_molecule->center().y(), 1.5 / 4.0 + 1.1);
  QCOMPARE(m_molecule->center().z(), 1.5 / 4.0 + 1.2);
}

QTEST_MAIN(MoleculeTest)

#include "moc_moleculetest.cxx"
