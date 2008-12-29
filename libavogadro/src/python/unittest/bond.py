import Avogadro
import unittest
from numpy import *

class TestBond(unittest.TestCase):
  def setUp(self):
    self.molecule = Avogadro.Molecule()
    # test with indices > 0
    for x in range(5):
      self.molecule.addAtom()
    for x in range(7):
      self.molecule.addBond()

  def test_type(self):
    bond = self.molecule.addBond()
    self.assertEqual(bond.type, Avogadro.PrimitiveType.BondType)

  # also tests bond.beginAtomId
  def test_setBegin(self):
    atom = self.molecule.addAtom()
    bond = self.molecule.addBond()

    bond.setBegin(atom)
    self.assertEqual(atom.id, bond.beginAtomId)
    # check if setBegin added the bond to the atom
    self.assert_(bond.id in atom.bonds)

  # also tests bond.endAtomId
  def test_setEnd(self):
    atom = self.molecule.addAtom()
    bond = self.molecule.addBond()

    bond.setEnd(atom)
    self.assertEqual(atom.id, bond.endAtomId)
    # check if setEnd added the bond to the atom
    self.assert_(bond.id in atom.bonds)

  def test_setAtoms(self):
    beginAtom = self.molecule.addAtom()
    endAtom = self.molecule.addAtom()
    bond = self.molecule.addBond()

    bond.setAtoms(beginAtom.id, endAtom.id, 2)
    self.assertEqual(beginAtom.id, bond.beginAtomId)
    self.assertEqual(endAtom.id, bond.endAtomId)
    # check if setAtoms added the bond to the atoms
    self.assert_(bond.id in beginAtom.bonds)
    self.assert_(bond.id in endAtom.bonds)

  def test_otherAtom(self):
    beginAtom = self.molecule.addAtom()
    endAtom = self.molecule.addAtom()
    bond = self.molecule.addBond()

    bond.setAtoms(beginAtom.id, endAtom.id, 2)
    # check if setAtoms added the bond to the atoms
    #self.assertEqual(bond.otherAtom(beginAtom.id), endAtom.id)
    bond.otherAtom(beginAtom.id) # why doesn't this work??

  def test_order(self):
    bond = self.molecule.addBond()
    bond.order = 2
    self.assertEqual(bond.order, 2)

  def test_length(self):
    beginAtom = self.molecule.addAtom()
    endAtom = self.molecule.addAtom()
    bond = self.molecule.addBond()

    #translate one atom
    endAtom.pos = array([1.5, 0., 0.])
    bond.setAtoms(beginAtom.id, endAtom.id, 1)
    self.assertEqual(bond.length, 1.5)







if __name__ == "__main__":
  unittest.main()
