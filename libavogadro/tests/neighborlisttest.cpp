/**********************************************************************
  Molecule - NeighborListTest class provides unit testing for the Molecule class

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
#include <avogadro/neighborlist.h>
#include <avogadro/molecule.h>
#include <avogadro/atom.h>

#include <Eigen/Core>

using Avogadro::NeighborList;
using Avogadro::Molecule;
using Avogadro::Atom;

using Eigen::Vector3d;

class NeighborListTest : public QObject
{
  Q_OBJECT

  private:
    Molecule *m_molecule; /// Molecule object for use by the test class.

    unsigned int m_correct5;
    unsigned int m_correct10;

    unsigned int test(int n, double r);

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

    void test5A_1n();
    void test5A_2n();
    void test5A_3n();

    void test10A_1n();
    void test10A_2n();
    void test10A_3n();

};

void NeighborListTest::initTestCase()
{
  m_molecule = new Molecule;

  // create a 10x10x10 regular grid with atoms
  for (int i = 0; i < 10; ++i)
    for (int j = 0; j < 10; ++j)
      for (int k = 0; k < 10; ++k) {
        Atom *atom = m_molecule->addAtom();
        atom->setPos(Eigen::Vector3d((double)i, (double)j, (double)k));
      }

  // compute the correct number of pairs
  m_correct5 = 0;
  m_correct10 = 0;
  for (unsigned int i = 0; i < m_molecule->numAtoms(); ++i)
    for (unsigned int j = 0; j < m_molecule->numAtoms(); ++j) {
      if (i >= j)
        continue;
      
      Atom *a = m_molecule->atom(i);
      Atom *b = m_molecule->atom(j);
      
      double r2 = ( *(a->pos()) - *(b->pos()) ).squaredNorm();

      if (r2 <= 25.0)
        m_correct5++;
      if (r2 <= 100.0)
        m_correct10++;
    }
}

void NeighborListTest::cleanupTestCase()
{
  delete m_molecule;
  m_molecule = 0;
}

void NeighborListTest::init()
{
}

void NeighborListTest::cleanup()
{
}

unsigned int NeighborListTest::test(int n, double r)
{
  NeighborList *nbrList = new NeighborList(m_molecule, r, false, n);
  
  unsigned int count = 0;
  for (unsigned int i = 0; i < m_molecule->numAtoms(); ++i) {
    QList<Atom*> nbrs = nbrList->nbrs(m_molecule->atom(i));

    foreach(Atom *nbr, nbrs) {
      Q_UNUSED(nbr);
      count++;
    }
  }

  return count;
}

void NeighborListTest::test5A_1n()
{
  unsigned int count = test(1, 5.);
  QCOMPARE(m_correct5, count);
}

void NeighborListTest::test5A_2n()
{
  unsigned int count = test(2, 5.);
  QCOMPARE(m_correct5, count);
}

void NeighborListTest::test5A_3n()
{
  unsigned int count = test(3, 5.);
  QCOMPARE(m_correct5, count);
}

void NeighborListTest::test10A_1n()
{
  unsigned int count = test(1, 10.);
  QCOMPARE(m_correct10, count);
}

void NeighborListTest::test10A_2n()
{
  unsigned int count = test(2, 10.);
  QCOMPARE(m_correct10, count);
}

void NeighborListTest::test10A_3n()
{
  unsigned int count = test(3, 10.);
  QCOMPARE(m_correct10, count);
}

QTEST_MAIN(NeighborListTest)

#include "moc_neighborlisttest.cxx"
