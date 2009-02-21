/**********************************************************************
  HydrogensCommandTest - unit testing for the the hydrogen QUndoCommands used by the hydrogen extension

  Copyright (C) 2009 Tim Vandermeersch

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
#include <Eigen/Core>
#include <avogadro/molecule.h>
#include <avogadro/atom.h>
#include <avogadro/bond.h>
#include <avogadro/glwidget.h>

#include "../src/extensions/hydrogensextension.cpp"
// HydrogensCommand(Molecule *molecule, enum Action action, GLWidget *widget, double pH = 7.4);
using Avogadro::HydrogensCommand;

using Avogadro::Molecule;
using Avogadro::Atom;
using Avogadro::Bond;
using Avogadro::GLWidget;

using Eigen::Vector3d;

class HydrogensCommandTest : public QObject
{
  Q_OBJECT

  private:
    QUndoStack *m_undoStack; /// QUndoStack object for use by the test class.
    Molecule   *m_molecule; /// Molecule object for use by the test class.
    GLWidget   *m_glwidget; /// Needed for selections....

  private:
    void debugMolecule();

    void loopUndoRedo();

  private slots:
    /**
     * Test AdjustHydrogensPreCommand
     */
    void AddHydrogens();
    /**
     * Test AdjustHydrogensPreCommand
     */
    void RemoveHydrogens();
    
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


};

void HydrogensCommandTest::initTestCase()
{
}

void HydrogensCommandTest::cleanupTestCase()
{
}

void HydrogensCommandTest::init()
{
  m_molecule = new Molecule;
  m_undoStack = new QUndoStack;
  m_glwidget = new GLWidget;
  m_glwidget->setMolecule(m_molecule);
}

void HydrogensCommandTest::cleanup()
{
  delete m_molecule;
  m_molecule = 0;
  delete m_undoStack;
  m_undoStack = 0;
  delete m_glwidget;
  m_glwidget = 0;
}
    
void HydrogensCommandTest::debugMolecule()
{
  qDebug() << "Atoms: index, id, atomicNumber";
  foreach (Atom *atom, m_molecule->atoms())
    qDebug() << "  index=" << atom->index() << ", id=" << atom->id() << ", atomicNumber=" << atom->atomicNumber()
             << ", valence=" << atom->valence();
  
  qDebug() << "Bonds: index, id, begin, end";
  foreach (Bond *bond, m_molecule->bonds())
    qDebug() << "  index=" << bond->index() << ", id=" << bond->id() << ", begin=" 
             << bond->beginAtomId() << ", end=" << bond->endAtomId();
}


void HydrogensCommandTest::loopUndoRedo()
{
  // save some variables to check after undo/redo loop
  unsigned int numAtoms = m_molecule->numAtoms();
  // save the index
  int cmdIndex = m_undoStack->index();

  // save all atom & bond ids
  QList<unsigned long> atomIds, bondIds;
  foreach (Atom *atom, m_molecule->atoms())
    atomIds.append(atom->id());
  foreach (Bond *bond, m_molecule->bonds())
    bondIds.append(bond->id());

  for (int i = 0; i < 10; ++i) {
    m_undoStack->setIndex(0); // undo all
    QCOMPARE(m_molecule->numAtoms(), (unsigned int) 0);
    m_undoStack->setIndex(cmdIndex); // redo all
    QCOMPARE(m_molecule->numAtoms(), numAtoms);
  }

  // check if all atom & bond ids are still the same
  foreach (Atom *atom, m_molecule->atoms())
    QCOMPARE(atom->id(), atomIds.at(atom->index()));
  foreach (Bond *bond, m_molecule->bonds())
    QCOMPARE(bond->id(), bondIds.at(bond->index()));


}

void HydrogensCommandTest::AddHydrogens()
{
  Atom *atom1 = m_molecule->addAtom();
  atom1->setAtomicNumber(6);
  Atom *atom2 = m_molecule->addAtom();
  atom2->setAtomicNumber(6);
  Bond *bond1 = m_molecule->addBond();
  bond1->setAtoms(atom1->id(), atom2->id(), 1);
  
  m_undoStack->push( new HydrogensCommand(m_molecule, HydrogensCommand::AddHydrogens, m_glwidget) );
  QCOMPARE(m_molecule->numAtoms(), (unsigned int) 8);
  QCOMPARE(m_molecule->numBonds(), (unsigned int) 7);

  //debugMolecule();
 
  QList<unsigned long> atomIds, bondIds;
  foreach (Atom *atom, m_molecule->atoms())
    atomIds.append(atom->id());
  foreach (Bond *bond, m_molecule->bonds())
    bondIds.append(bond->id());

  //debugMolecule();
  loopUndoRedo();
  //debugMolecule();

  QCOMPARE(m_molecule->numAtoms(), (unsigned int) 8);
  QCOMPARE(m_molecule->numBonds(), (unsigned int) 7);

  // check if all atom & bond ids are still the same
  foreach (Atom *atom, m_molecule->atoms())
    QCOMPARE(atom->id(), atomIds.at(atom->index()));
  foreach (Bond *bond, m_molecule->bonds())
    QCOMPARE(bond->id(), bondIds.at(bond->index()));

}

void HydrogensCommandTest::RemoveHydrogens()
{
  Atom *atom1 = m_molecule->addAtom();
  atom1->setAtomicNumber(6);
  Atom *atom2 = m_molecule->addAtom();
  atom2->setAtomicNumber(6);
  Bond *bond1 = m_molecule->addBond();
  bond1->setAtoms(atom1->id(), atom2->id(), 1);

  QCOMPARE(m_molecule->numAtoms(), (unsigned int) 2);
  QCOMPARE(m_molecule->numBonds(), (unsigned int) 1);
 
  m_molecule->addHydrogens();
  QCOMPARE(m_molecule->numAtoms(), (unsigned int) 8);
  QCOMPARE(m_molecule->numBonds(), (unsigned int) 7);
  
  m_undoStack->push( new HydrogensCommand(m_molecule, HydrogensCommand::RemoveHydrogens, m_glwidget) );

  QCOMPARE(m_molecule->numAtoms(), (unsigned int) 2);
  QCOMPARE(m_molecule->numBonds(), (unsigned int) 1);
 
}



QTEST_MAIN(HydrogensCommandTest)

#include "moc_hydrogenscommandtest.cxx"

