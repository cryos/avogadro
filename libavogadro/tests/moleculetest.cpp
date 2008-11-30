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

using Avogadro::Molecule;

class MoleculeTest : public QObject
{
  Q_OBJECT

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
     * Instantiates a new Molecule object, adds a couple of atoms and ensures
     * they are successfully added to the Molecule.
     */
    void testAtoms();

    /**
     * Instantiates a new Molecule object, adds a couple of bonds and ensures
     * they are successfully added to the Molecule.
     */
    void testBonds();
};

void MoleculeTest::initTestCase()
{
}

void MoleculeTest::cleanupTestCase()
{
}

void MoleculeTest::init()
{
}

void MoleculeTest::cleanup()
{
}

void MoleculeTest::testAtoms()
{
  Molecule mol;
  mol.newAtom();
  QVERIFY(mol.numAtoms() == 1);
  mol.newAtom();
  QVERIFY(mol.numAtoms() == 2);
}

void MoleculeTest::testBonds()
{
  Molecule mol;
  mol.newBond();
  QVERIFY(mol.numBonds() == 1);
  mol.newBond();
  QVERIFY(mol.numBonds() == 2);
}

QTEST_MAIN(MoleculeTest)

#include "moc_moleculetest.cxx"

