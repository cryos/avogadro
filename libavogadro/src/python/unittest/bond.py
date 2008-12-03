import Avogadro
import unittest
from numpy import *

class TestBond(unittest.TestCase):
  def setUp(self):
    self.molecule = Avogadro.Molecule()
    # test with indices > 0
    for x in range(5):
      self.molecule.newAtom()
    for x in range(7):
      self.molecule.newBond()

  # also tests bond.beginAtomId
  def test_setBegin(self):
    atom = self.molecule.newAtom()
    bond = self.molecule.newBond()

    bond.setBegin(atom)
    self.assertEqual(atom.id, bond.beginAtomId)
    # check if setBegin added the bond to the atom
    self.assert_(bond.id in atom.bonds)

  # also tests bond.endAtomId
  def test_setEnd(self):
    atom = self.molecule.newAtom()
    bond = self.molecule.newBond()

    bond.setEnd(atom)
    self.assertEqual(atom.id, bond.endAtomId)
    # check if setEnd added the bond to the atom
    self.assert_(bond.id in atom.bonds)

  def test_setAtoms(self):
    beginAtom = self.molecule.newAtom()
    endAtom = self.molecule.newAtom()
    bond = self.molecule.newBond()

    bond.setAtoms(beginAtom.id, endAtom.id, 2)
    self.assertEqual(beginAtom.id, bond.beginAtomId)
    self.assertEqual(endAtom.id, bond.endAtomId)
    # check if setAtoms added the bond to the atoms
    self.assert_(bond.id in beginAtom.bonds)
    self.assert_(bond.id in endAtom.bonds)

  def test_otherAtom(self):
    beginAtom = self.molecule.newAtom()
    endAtom = self.molecule.newAtom()
    bond = self.molecule.newBond()

    bond.setAtoms(beginAtom.id, endAtom.id, 2)
    # check if setAtoms added the bond to the atoms
    #self.assertEqual(bond.otherAtom(beginAtom.id), endAtom.id)
    bond.otherAtom(bond, 0)





if __name__ == "__main__":
  unittest.main()
