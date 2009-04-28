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
   * Timing to add 25,000 atoms, set atomic number and position
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

void MoleculeBench::destructor2()
{
  for (int i = 0; i < 10000; ++i)
    m_molecule->addAtom();
  QBENCHMARK{
    delete m_molecule;
    m_molecule = 0;
  }
}

void MoleculeBench::destructor3()
{
  m_molecule = new Molecule;
  for (int i = 0; i < 10000; ++i) {
    Atom *a = m_molecule->addAtom();
    a->setPos(Vector3d(1.0, 1.0, 1.0));
  }
  QBENCHMARK{
    delete m_molecule;
    m_molecule = 0;
  }
}

void MoleculeBench::destructor4()
{
  m_molecule = new Molecule;
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
  QBENCHMARK{
    delete m_molecule;
    m_molecule = 0;
  }
}

void MoleculeBench::destructor5()
{
  m_molecule = new Molecule;
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
  QBENCHMARK{
    delete m_molecule;
    m_molecule = 0;
  }
}

void MoleculeBench::destructor6()
{
  m_molecule = new Molecule;
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
  QBENCHMARK{
    delete m_molecule;
    m_molecule = 0;
  }
}

void MoleculeBench::deleteLater()
{
  m_molecule = new Molecule;
  for (int i = 0; i < 30000; ++i) {
    Atom *a = m_molecule->addAtom();
    a->setPos(Vector3d(1.0, 1.0, 1.0));
  }
  m_molecule->clear();
  qDebug() << m_molecule->children().size();
  QBENCHMARK{
    m_molecule->deleteLater();
    m_molecule = 0;
  }
}

QTEST_MAIN(MoleculeBench)

#include "moc_moleculebench.cxx"
