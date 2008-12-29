import Avogadro
import unittest
from numpy import *

class TestPrimitiveList(unittest.TestCase):
  def setUp(self):
    self.molecule = Avogadro.Molecule()
    self.list = Avogadro.PrimitiveList()

  def test_constructors(self):
    qlist = []
    for i in range(10):
      atom = self.molecule.addAtom()
      self.list.append(atom)
      qlist.append(atom)
    self.assertEqual(self.list.size, 10)

    list2 = Avogadro.PrimitiveList(self.list)
    self.assertEqual(list2.size, 10)
    list3 = Avogadro.PrimitiveList(qlist)
    self.assertEqual(list3.size, 10)

  def test_subList(self):
    for i in range(3):
      self.list.append(self.molecule.addAtom())
      self.list.append(self.molecule.addBond())
      self.list.append(self.molecule.addCube())

    atomList = self.list.subList(Avogadro.PrimitiveType.AtomType)
    self.assertEqual(len(atomList), 3)
    for atom in atomList:
      self.assertEqual(atom.type, Avogadro.PrimitiveType.AtomType)
      
  def test_list(self):
    for i in range(3):
      self.list.append(self.molecule.addAtom())
      self.list.append(self.molecule.addBond())
      self.list.append(self.molecule.addCube())

    self.assertEqual(len(self.list.list), 9)
 
  def test_contains(self):
    for i in range(3):
      self.list.append(self.molecule.addAtom())
      self.list.append(self.molecule.addBond())
      self.list.append(self.molecule.addCube())

    atom1 = self.molecule.atom(0)
    atom2 = self.molecule.addAtom()
    self.assertEqual(self.list.contains(atom1), True)
    self.assertEqual(self.list.contains(atom2), False)
 
  def test_removeAll(self):
    for i in range(3):
      self.list.append(self.molecule.addAtom())
      self.list.append(self.molecule.addBond())
      self.list.append(self.molecule.addCube())

    atom = self.molecule.addAtom()
    self.list.removeAll(atom)
    self.assertEqual(len(self.list.list), 9)
    self.list.removeAll(self.molecule.atom(0))
    self.assertEqual(len(self.list.list), 8)

  def test_isEmpty(self):
    self.assertEqual(self.list.isEmpty, True)
    self.list.append(self.molecule.addAtom())
    self.assertEqual(self.list.isEmpty, False)

  def test_count(self):
    for i in range(3):
      self.list.append(self.molecule.addAtom())
      self.list.append(self.molecule.addBond())
      self.list.append(self.molecule.addCube())
    
    self.assertEqual(self.list.count(Avogadro.PrimitiveType.AtomType), 3)
    self.assertEqual(self.list.count(Avogadro.PrimitiveType.BondType), 3)
    self.assertEqual(self.list.count(Avogadro.PrimitiveType.CubeType), 3)
    
    self.assertEqual(self.list.size, 9)

  def test_clear(self):
    for i in range(3):
      self.list.append(self.molecule.addAtom())
      self.list.append(self.molecule.addBond())
      self.list.append(self.molecule.addCube())
    
    self.assertEqual(self.list.size, 9)
    self.list.clear()
    self.assertEqual(self.list.size, 0)



   




if __name__ == "__main__":
  unittest.main()
