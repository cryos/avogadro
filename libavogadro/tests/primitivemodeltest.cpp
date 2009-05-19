/**********************************************************************
  PrimitiveModelTest - Provides unit testing for the PrimitiveItemModel class

  Copyright (C) 2009 Geoffrey Hutchison

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
#include <avogadro/primitiveitemmodel.h>
#include <avogadro/molecule.h>
#include <avogadro/atom.h>
#include <avogadro/bond.h>

#include "modeltest.h"

using Avogadro::PrimitiveItemModel;

using Avogadro::Molecule;
using Avogadro::Atom;
using Avogadro::Bond;

class PrimitiveModelTest : public QObject
{
  Q_OBJECT

  private:
  Molecule *m_molecule;
  void prepareMolecule();

  private slots:

  void initTestCase();
  void cleanupTestCase();

  void testMoleculeModel();
};

  void PrimitiveModelTest::prepareMolecule()
  {
    Atom *a1 = m_molecule->addAtom();
    a1->setPos(Eigen::Vector3d(0.0, 0.0, 0.0));
    Atom *a2 = m_molecule->addAtom();
    a2->setPos(Eigen::Vector3d(1.5, 0.0, 0.0));
    Atom *a3 = m_molecule->addAtom();
    a3->setPos(Eigen::Vector3d(0.0, 1.5, 0.0));
    Atom *a4 = m_molecule->addAtom();
    a4->setPos(Eigen::Vector3d(0.0, 0.0, 1.5));
    Bond *b1 = m_molecule->addBond();
    b1->setAtoms(a1->id(), a2->id(), 1);
    Bond *b2 = m_molecule->addBond();
    b2->setAtoms(a2->id(), a3->id(), 1);
    Bond *b3 = m_molecule->addBond();
    b3->setAtoms(a3->id(), a4->id(), 1);
  }


void PrimitiveModelTest::initTestCase()
{
  m_molecule = new Molecule;
  prepareMolecule();
}

void PrimitiveModelTest::cleanupTestCase()
{
  delete m_molecule;
  m_molecule = 0;
}

  void PrimitiveModelTest::testMoleculeModel()
  {
    PrimitiveItemModel *model = new PrimitiveItemModel(m_molecule, this);
    new ModelTest(model, this);
  }

QTEST_MAIN(PrimitiveModelTest)

#include "moc_primitivemodeltest.cxx"

