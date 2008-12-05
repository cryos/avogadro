import Avogadro
import unittest
from numpy import *

#
# Rings are Fragments...
#
class TestFragment(unittest.TestCase):
  def setUp(self):
    self.molecule = Avogadro.Molecule()

  def test_name(self):
    fragment = self.molecule.newRing() # Ring = Fragment
    fragment.name = "testing"
    self.assertEqual(fragment.name, "testing")
    
  def test_atom(self):
    fragment = self.molecule.newRing() # Ring = Fragment

    # create two atoms
    atom1 = self.molecule.newAtom()
    atom2 = self.molecule.newBond()
    # add atom 2 to the fragment - test addAtom(id)
    fragment.addAtom(atom2.id)

    # test atoms()
    self.assertEqual(len(fragment.atoms), 1)
    self.assert_(atom2.id in fragment.atoms)

    # test removeAtom(id)
    fragment.removeAtom(atom2.id)
    self.assertEqual(len(fragment.atoms), 0)

  def test_bond(self):
    fragment = self.molecule.newRing() # Ring = Fragment

    # create two bonds
    bond1 = self.molecule.newBond()
    bond2 = self.molecule.newBond()
    # add bond 2 to the fragment - test addBond(id)
    fragment.addBond(bond2.id)

    # test bonds()
    self.assertEqual(len(fragment.bonds), 1)
    self.assert_(bond2.id in fragment.bonds)

    # test removeBond(id)
    fragment.removeBond(bond2.id)
    self.assertEqual(len(fragment.bonds), 0)





if __name__ == "__main__":
  unittest.main()
