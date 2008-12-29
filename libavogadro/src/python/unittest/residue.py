import Avogadro
import unittest
from numpy import *

class TestResidue(unittest.TestCase):
  def setUp(self):
    self.molecule = Avogadro.Molecule()

  def test_number(self):
    residue = self.molecule.addResidue()
    residue.number = "5A"
    self.assertEqual(residue.number, "5A")

  def test_chainNumber(self):
    residue = self.molecule.addResidue()
    residue.chainNumber = 5
    self.assertEqual(residue.chainNumber, 5)
  
  def test_atomIds(self):
    residue = self.molecule.addResidue()
    # add 4 atoms
    for i in range(4):
      atom = self.molecule.addAtom()
      residue.addAtom(atom.id)

    # set the atoms ids
    ids = ["N", "C", "CA", "O"]
    residue.atomIds = ids
    
    self.assert_("N" in residue.atomIds)
    self.assert_("C" in residue.atomIds)
    self.assert_("CA" in residue.atomIds)
    self.assert_("O" in residue.atomIds)

    # change an id
    self.assertEqual(residue.atomId(2), "CA")
    self.assertEqual(residue.setAtomId(2, "CE3"), True)
    self.assertEqual(residue.atomId(2), "CE3")





if __name__ == "__main__":
  unittest.main()
