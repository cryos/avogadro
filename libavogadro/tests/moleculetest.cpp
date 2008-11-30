/**********************************************************************
  Molecule - MoleculeTest class provides unit testing for the Molecule class

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

#include <QtTest>
#include <avogadro/molecule.h>
#include <avogadro/atom.h>
#include <avogadro/bond.h>

using Avogadro::Molecule;
using Avogadro::Atom;
using Avogadro::Bond;

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
    void testNewAtoms();

    /**
     * Tests the deletion of Atom objects from the Molecule.
     */
    void testDeleteAtoms();

    /**
     * Tests the addition of new Bond objects to the Molecule.
     */
    void testNewBonds();

    /**
     * Tests the deletion of Bond objects from the Molecule.
     */
    void testDeleteBonds();
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

void MoleculeTest::testNewAtoms()
{
  m_molecule->newAtom();
  QVERIFY(m_molecule->numAtoms() == 1);
  m_molecule->newAtom();
  QVERIFY(m_molecule->numAtoms() == 2);
}

void MoleculeTest::testDeleteAtoms()
{
  // Should now to two atoms in the Molecule with ids 0 and 1.
  Atom *a = m_molecule->atom(1);
  m_molecule->deleteAtom(a);
  QVERIFY(m_molecule->numAtoms() == 1);
  m_molecule->deleteAtom(0ul);
  QVERIFY(m_molecule->numAtoms() == 0);
}

void MoleculeTest::testNewBonds()
{
  m_molecule->newBond();
  QVERIFY(m_molecule->numBonds() == 1);
  m_molecule->newBond();
  QVERIFY(m_molecule->numBonds() == 2);
}

void MoleculeTest::testDeleteBonds()
{
  // Should now to two atoms in the Molecule with ids 0 and 1.
  Bond *b = m_molecule->bond(1);
  m_molecule->deleteBond(b);
  QVERIFY(m_molecule->numBonds() == 1);
  m_molecule->deleteBond(0ul);
  QVERIFY(m_molecule->numBonds() == 0);
}

QTEST_MAIN(MoleculeTest)

#include "moc_moleculetest.cxx"

