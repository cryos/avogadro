import Avogadro
import unittest
from numpy import *

class TestAtom(unittest.TestCase):
  def setUp(self):
    self.molecule = Avogadro.molecules.addMolecule()

  def test_type(self):
    atom = self.molecule.addAtom()
    self.assertEqual(atom.type, Avogadro.PrimitiveType.AtomType)

  def test_pos(self):
    atom = self.molecule.addAtom()
    atom.pos
    vec = array([1., 2., 3.])
    atom.pos = vec
    self.assertEqual(atom.pos[0], 1.)
    self.assertEqual(atom.pos[1], 2.)
    self.assertEqual(atom.pos[2], 3.)

  def test_atomicNumber(self):
    atom = self.molecule.addAtom()
    self.assertEqual(atom.atomicNumber, 0)
    atom.atomicNumber = 6
    self.assertEqual(atom.atomicNumber, 6)

#  def test_addBond(self):
#    atom = self.molecule.addAtom()
#    bond1 = self.molecule.addBond()
#    bond2 = self.molecule.addBond()
#
#    # test addBond(Bond*)
#    atom.addBond(bond1)
#    self.assert_(bond1.id in atom.bonds)
#    # void addBond(unsigned long int)
#    atom.addBond(bond2.id)
#    self.assert_(bond2.id in atom.bonds)

#  def test_removeBond(self):
#    atom = self.molecule.addAtom()
#    # add two bonds
#    bond1 = self.molecule.addBond()
#    bond2 = self.molecule.addBond()
#    atom.addBond(bond1)
#    atom.addBond(bond2)
#    self.assertEqual(len(atom.bonds), 2)
#
#    # test removeBond(Bond*)
#    atom.removeBond(bond1)
#    self.assertEqual(len(atom.bonds), 1)
#    # void removeBond(unsigned long int)
#    atom.removeBond(bond2.id)
#    self.assertEqual(len(atom.bonds), 0)

  def test_bonds(self):
    atom = self.molecule.addAtom()
    # add 5 bonds
    for i in range(5):
      bond = self.molecule.addBond()
      bond.setBegin(atom)
    # test the length
    self.assertEqual(len(atom.bonds), 5)
    # test the items
    for i in range(5):
      self.assertEqual(atom.bonds[i], i)

  def test_neighbors(self):
    # add 4 atoms
    atom1 = self.molecule.addAtom()
    atom2 = self.molecule.addAtom()
    atom3 = self.molecule.addAtom()
    atom4 = self.molecule.addAtom()
    # add 3 bonds
    bond1 = self.molecule.addBond()
    bond2 = self.molecule.addBond()
    bond3 = self.molecule.addBond()

    # bond.setAtoms() calls atom.addBond()
    bond1.setAtoms(atom1.id, atom2.id, 1)
    bond2.setAtoms(atom1.id, atom3.id, 1)
    bond3.setAtoms(atom1.id, atom4.id, 1)
    
    # test the length
    self.assertEqual(len(atom1.neighbors), 3)

  def test_valence(self):
    # add 3 atoms
    atom1 = self.molecule.addAtom()
    atom2 = self.molecule.addAtom()
    atom3 = self.molecule.addAtom()
    # add 2 bonds
    bond1 = self.molecule.addBond()
    bond2 = self.molecule.addBond()

    # bond.setAtoms() calls atom.addBond()
    bond1.setAtoms(atom1.id, atom2.id, 1)
    bond2.setAtoms(atom1.id, atom3.id, 1)

    print atom1.bonds
    
    self.assertEqual(atom1.valence, 2)

  def test_isHydrogen(self):
    # add 3 atoms
    atom = self.molecule.addAtom()
    atom.atomicNumber = 1
    
    self.assert_(atom.isHydrogen)

  # ask Marcus...
  def test_partialCharge(self):
    atom = self.molecule.addAtom()
    atom.atomicNumber = 35
    self.assertEqual(atom.partialCharge, 0.0)
    atom.partialCharge = 0.325
    self.assertEqual(atom.partialCharge, 0.325)



if __name__ == "__main__":
  unittest.main()
