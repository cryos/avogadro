/**********************************************************************
  Molecule - MoleculeBench class provides benchmarking for the Molecule class

  Copyright (C) 2009 Marcus D. Hanwell
  Copyright (C) 2011 David C. Lonie

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.cc/>

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

#include "config.h"

#include <QtTest>

#include <avogadro/atom.h>
#include <avogadro/bond.h>
#include <avogadro/molecule.h>
#include <avogadro/primitivelist.h>

#include <Eigen/Core>

#include <QCoreApplication>

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
   * Called before each test function.
   */
  void init();

  /**
   * Called after each test function.
   */
  void cleanup();

  /**
   * Constructor test
   */
  void constructor();

  /**
   * Timing to add 25,000 uninitialized atoms
   */
  void addAtoms();

  /**
   * Timing to remove 25,000 uninitialized atoms
   */
  void removeAtoms();

  /**
   * Timing to add 25,000 atoms, set atomic number and position
   */
  void addAtoms2();

  /**
   * Timing to clear the 25,000 initialized atom molecule
   */
  void clear();

  /**
   * Timing to add 25,000 atoms using the type and position convenience
   * overload.
   */
  void addAtomOverload1();

  /**
   * Timing to add 25,000 more atoms using the copy overload.
   */
  void addAtomOverload2();

  /**
   * Timing to add 24,999 bonds using the convenience overloads.
   */
  void addBondOverload();

  /**
   * Copy 25,000 atoms and 24,999 bonds using the QList overload of
   * Molecule::copyAtomsAndBonds()
   */
  void copyAtomsAndBondsQList();

  /**
   * Copy 25,000 atoms and 24,999 bonds using the PrimitiveList overload of
   * Molecule::copyAtomsAndBonds()
   */
  void copyAtomsAndBondsPrimitiveList();

  /**
   * Destruct the molecule
   */
  void destructor();

  /**
   * Construct the molecule again
   */
  void constructor2();

  /**
   * Destructor test 2, with 10,000 atoms in a new Molecule
   */
  void destructor2();

  /**
   * Destructor test 3, with 10,000 atoms in a new Molecule, updated positions
   */
  void destructor3();

  /**
   * Destructor test 4, with 10,000 atoms in a new Molecule, updated positions,
   * removed, then added back.
   */
  void destructor4();

  /**
   * Destructor test 5, with 10,000 atoms in a new Molecule, updated positions,
   * removed, then added back, then removed and added back again.
   */
  void destructor5();

  /**
   * Destructor test 6, with 30,000 atoms in a new Molecule, updated positions
   */
  void destructor6();

  /**
   * deleteLater test, with 30,000 atoms in a new Molecule, updated positions
   */
  void deleteLater();

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

void MoleculeBench::init()
{
  m_molecule = new Avogadro::Molecule ();
}

void MoleculeBench::cleanup()
{
  delete m_molecule;
  m_molecule = 0;
}

void MoleculeBench::constructor()
{
  delete m_molecule;
  QBENCHMARK_ONCE {
    m_molecule = new Molecule;
  }
}

void MoleculeBench::addAtoms()
{
  QBENCHMARK_ONCE {
    for (int i = 0; i < 25000; ++i)
      m_molecule->addAtom();
  }
}

void MoleculeBench::removeAtoms()
{
  for (int i = 0; i < 25000; ++i)
    m_molecule->addAtom();

  QBENCHMARK_ONCE{
    for (int i = 25000; i >= 0; --i)
      m_molecule->removeAtom(i);
  }
}

void MoleculeBench::addAtoms2()
{
  QBENCHMARK_ONCE{
    for (int i = 0; i < 25000; ++i) {
      Atom *a = m_molecule->addAtom();
      a->setAtomicNumber(6);
      a->setPos(Vector3d(1.0, 1.0, 1.0));
    }
  }
}

void MoleculeBench::clear()
{
  for (int i = 0; i < 25000; ++i) {
    Atom *a = m_molecule->addAtom();
    a->setAtomicNumber(6);
    a->setPos(Vector3d(1.0, 1.0, 1.0));
  }
  QBENCHMARK_ONCE{
    m_molecule->clear();
  }
}

void MoleculeBench::addAtomOverload1()
{
  QBENCHMARK_ONCE{
    for (int i = 0; i < 25000; ++i) {
      m_molecule->addAtom(6, Vector3d(1.0, 1.0, 1.0));
    }
  }
}

void MoleculeBench::addAtomOverload2()
{
  Atom *atom = m_molecule->addAtom(6, Vector3d(1.0, 1.0, 1.0));
  QBENCHMARK_ONCE{
    for (int i = 0; i < 25000; ++i) {
      m_molecule->addAtom(*atom);
    }
  }
}

void MoleculeBench::addBondOverload()
{
  for (int i = 0; i < 25000; ++i) {
    m_molecule->addAtom(6, Vector3d(1.0, 1.0, 1.0));
  }
  const unsigned long max = m_molecule->numAtoms() - 1;
  QBENCHMARK_ONCE{
    for (unsigned long i = 0; i < max; ++i) {
      m_molecule->addBond(i, i+1, 1);
    }
  }
}

void MoleculeBench::copyAtomsAndBondsQList()
{
  Avogadro::Molecule mol;

  for (int i = 0; i < 25000; ++i) {
    m_molecule->addAtom(6, Vector3d(1.0, 1.0, 1.0));
  }
  for (unsigned long i = 0; i < 24999; ++i) {
    m_molecule->addBond(i, i+1, 1);
  }

  QBENCHMARK_ONCE{
    mol.copyAtomsAndBonds(m_molecule->atoms(), m_molecule->bonds());
  }
}


void MoleculeBench::copyAtomsAndBondsPrimitiveList()
{
  Avogadro::Molecule mol;

  for (int i = 0; i < 25000; ++i) {
    m_molecule->addAtom(6, Vector3d(1.0, 1.0, 1.0));
  }
  for (unsigned long i = 0; i < 24999; ++i) {
    m_molecule->addBond(i, i+1, 1);
  }

  const QList<Avogadro::Atom*> atoms (m_molecule->atoms());
  const QList<Avogadro::Bond*> bonds (m_molecule->bonds());

  Avogadro::PrimitiveList list;
  foreach (Avogadro::Atom *atom, atoms)
    list.append(atom);
  foreach (Avogadro::Bond *bond, bonds)
    list.append(bond);

  QBENCHMARK_ONCE{
    mol.copyAtomsAndBonds(list);
  }
}

void MoleculeBench::destructor()
{
  QBENCHMARK_ONCE{
    delete m_molecule;
    m_molecule = 0;
  }
}

void MoleculeBench::constructor2()
{
  delete m_molecule;
  m_molecule = 0;
  QBENCHMARK_ONCE{
    m_molecule = new Molecule;
  }
}

void MoleculeBench::destructor2()
{
  for (int i = 0; i < 10000; ++i)
    m_molecule->addAtom();
  QBENCHMARK_ONCE{
    delete m_molecule;
    m_molecule = 0;
  }
}

void MoleculeBench::destructor3()
{
  for (int i = 0; i < 10000; ++i) {
    Atom *a = m_molecule->addAtom();
    a->setPos(Vector3d(1.0, 1.0, 1.0));
  }
  QBENCHMARK_ONCE{
    delete m_molecule;
    m_molecule = 0;
  }
}

void MoleculeBench::destructor4()
{
  for (int i = 0; i < 10000; ++i) {
    Atom *a = m_molecule->addAtom();
    a->setPos(Vector3d(1.0, 1.0, 1.0));
  }
  m_molecule->clear();
  for (int i = 0; i < 10000; ++i) {
    Atom *a = m_molecule->addAtom();
    a->setPos(Vector3d(1.0, 1.0, 1.0));
  }
  qDebug() << "Before:" << m_molecule->children().size();
  QBENCHMARK_ONCE{
    delete m_molecule;
    m_molecule = 0;
  }
}

void MoleculeBench::destructor5()
{
  for (int i = 0; i < 10000; ++i) {
    Atom *a = m_molecule->addAtom();
    a->setPos(Vector3d(1.0, 1.0, 1.0));
  }
  m_molecule->clear();
  for (int i = 0; i < 10000; ++i) {
    Atom *a = m_molecule->addAtom();
    a->setPos(Vector3d(1.0, 1.0, 1.0));
  }
  m_molecule->clear();
  for (int i = 0; i < 10000; ++i) {
    Atom *a = m_molecule->addAtom();
    a->setPos(Vector3d(1.0, 1.0, 1.0));
  }
  qDebug() << "Before:" << m_molecule->children().size();
  QBENCHMARK_ONCE{
    delete m_molecule;
    m_molecule = 0;
  }
}

void MoleculeBench::destructor6()
{
  for (int i = 0; i < 25000; ++i) {
    Atom *a = m_molecule->addAtom();
    a->setPos(Vector3d(1.0, 1.0, 1.0));
  }
  m_molecule->clear();
  qDebug() << "Before:" << m_molecule->children().size();
  QCoreApplication::processEvents();
  QCoreApplication::sendPostedEvents();
  QCoreApplication::processEvents();
  qDebug() << "Thread:" << m_molecule->thread();
  qDebug() << "Pending events:" << QCoreApplication::hasPendingEvents();
  qDebug() << "After:" << m_molecule->children().size();
  QBENCHMARK_ONCE{
    delete m_molecule;
    m_molecule = 0;
  }
}

void MoleculeBench::deleteLater()
{
  for (int i = 0; i < 30000; ++i) {
    Atom *a = m_molecule->addAtom();
    a->setPos(Vector3d(1.0, 1.0, 1.0));
  }
  m_molecule->clear();
  qDebug() << m_molecule->children().size();
  QBENCHMARK_ONCE{
    m_molecule->deleteLater();
    m_molecule = 0;
  }
}

QTEST_MAIN(MoleculeBench)

#include "moc_moleculebench.cxx"
