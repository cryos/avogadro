import Avogadro
import unittest
from numpy import *

class TestMolecule(unittest.TestCase):
  def setUp(self):
    self.molecule = Avogadro.molecules.addMolecule()

  def test_fileName(self):
    self.molecule.fileName = "somefile.ext"
    self.assertEqual(self.molecule.fileName, "somefile.ext")

  def test_addAtom(self):
    # add 5 atoms
    for i in range(5):
      # test addAtom()
      atom = self.molecule.addAtom()
      self.assertEqual(atom.index, i)
      self.assertEqual(atom.id, i)
      self.assertEqual(atom.type, Avogadro.PrimitiveType.AtomType)

    # take atom with index 2
    atom = self.molecule.atom(2)
    # store the id
    id = atom.id
    # delete it
    self.molecule.removeAtom(id)

    # test addAtom(id)
    atom = self.molecule.addAtom(id)
    self.assertEqual(atom.id, id)

  def test_atomPos(self):
    atom = self.molecule.addAtom()

    pos = self.molecule.atomPos(atom.id)
    self.assertEqual(pos[0], 0.0)
    self.assertEqual(pos[0], 0.0)
    self.assertEqual(pos[0], 0.0)
    
    self.molecule.setAtomPos(atom.id, array([1., 2., 3.]))
    pos = self.molecule.atomPos(atom.id)
    self.assertEqual(pos[0], 1.0)
    self.assertEqual(pos[1], 2.0)
    self.assertEqual(pos[2], 3.0)

  def test_removeAtom(self):
    # add 2 atoms
    atom1 = self.molecule.addAtom()
    atom2 = self.molecule.addAtom()
    
    self.assertEqual(self.molecule.numAtoms, 2)
    # test removeAtom(Atom)
    self.molecule.removeAtom(atom1)
    self.assertEqual(self.molecule.numAtoms, 1)
    # test removeAtom(id)
    self.molecule.removeAtom(atom2.id)
    self.assertEqual(self.molecule.numAtoms, 0)

  def test_addBond(self):
    # add 5 bonds
    for i in range(5):
      # test addBond()
      bond = self.molecule.addBond()
      self.assertEqual(bond.index, i)
      self.assertEqual(bond.id, i)
      self.assertEqual(bond.type, Avogadro.PrimitiveType.BondType)

    # take bond with index 2
    bond = self.molecule.bond(2)
    # store the id
    id = bond.id
    # delete it
    self.molecule.removeBond(id)

    # test addBond(id)
    bond = self.molecule.addBond(id)
    self.assertEqual(bond.id, id)

  def test_removeBond(self):
    # add 2 bonds
    bond1 = self.molecule.addBond()
    bond2 = self.molecule.addBond()
    
    self.assertEqual(self.molecule.numBonds, 2)
    # test removeBond(Bond)
    self.molecule.removeBond(bond1)
    self.assertEqual(self.molecule.numBonds, 1)
    # test removeBond(id)
    self.molecule.removeBond(bond2.id)
    self.assertEqual(self.molecule.numBonds, 0)

  def test_addHydrogens(self):
    atom = self.molecule.addAtom()
    atom.atomicNumber = 6
    self.molecule.addHydrogens(atom)
    self.assertEqual(self.molecule.numAtoms, 5)
    
  def test_removeHydrogens(self):
    atom = self.molecule.addAtom()
    atom.atomicNumber = 6
    self.molecule.addHydrogens(atom)
    self.assertEqual(self.molecule.numAtoms, 5)
    self.molecule.removeHydrogens(atom)
    self.assertEqual(self.molecule.numAtoms, 1)

  def test_calculatePartialCharges(self):
    print("FIXME: implement test_calculate_partialCharges")

  def test_addCube(self):
    for i in range(5):
      cube = self.molecule.addCube()
      self.assertEqual(cube.index, i)
      self.assertEqual(cube.id, i)
      self.assertEqual(cube.type, Avogadro.PrimitiveType.CubeType)

  def test_removeCube(self):
    # add 2 cubes
    cube1 = self.molecule.addCube()
    cube2 = self.molecule.addCube()
    
    self.assertEqual(self.molecule.numCubes, 2)
    # test removeCube(Cube)
    self.molecule.removeCube(cube1)
    self.assertEqual(self.molecule.numCubes, 1)
    # test removeCube(id)
    self.molecule.removeCube(cube2.id)
    self.assertEqual(self.molecule.numCubes, 0)

  def test_addResidue(self):
    for i in range(5):
      residue = self.molecule.addResidue()
      self.assertEqual(residue.index, i)
      self.assertEqual(residue.id, i)
      self.assertEqual(residue.type, Avogadro.PrimitiveType.ResidueType)

  def test_removeResidue(self):
    # add 2 residues
    residue1 = self.molecule.addResidue()
    residue2 = self.molecule.addResidue()
    
    self.assertEqual(self.molecule.numResidues, 2)
    # test removeResidue(Residue)
    self.molecule.removeResidue(residue1)
    self.assertEqual(self.molecule.numResidues, 1)
    # test removeResidue(id)
    self.molecule.removeResidue(residue2.id)
    self.assertEqual(self.molecule.numResidues, 0)

  def test_addRing(self):
    for i in range(5):
      ring = self.molecule.addRing()
      self.assertEqual(ring.index, i)
      self.assertEqual(ring.id, i)
      self.assertEqual(ring.type, Avogadro.PrimitiveType.FragmentType)

  def test_removeRing(self):
    # add 2 rings
    ring1 = self.molecule.addRing()
    ring2 = self.molecule.addRing()
    
    self.assertEqual(self.molecule.numRings, 2)
    # test removeRing(Ring)
    self.molecule.removeRing(ring1)
    self.assertEqual(self.molecule.numRings, 1)
    # test removeRing(id)
    self.molecule.removeRing(ring2.id)
    self.assertEqual(self.molecule.numRings, 0)

  def test_numXXX(self):
    for i in range(5):
      self.molecule.addAtom()
      self.molecule.addBond()
      self.molecule.addResidue()
      self.molecule.addCube()
      self.molecule.addRing()
    
    self.assertEqual(self.molecule.numAtoms, 5)
    self.assertEqual(self.molecule.numBonds, 5)
    self.assertEqual(self.molecule.numResidues, 5)
    self.assertEqual(self.molecule.numCubes, 5)
    self.assertEqual(self.molecule.numRings, 5)

  def test_atom(self):
    # add 3 atoms
    for i in range(3):
      self.molecule.addAtom()
    # delete the 2nd
    self.molecule.removeAtom(1)
    # atom with index 1 should now have id 2
    self.assertEqual(self.molecule.atom(1).id, 2)

  def test_atomById(self):
    # add 3 atoms
    for i in range(3):
      self.molecule.addAtom()
    # delete the 2nd
    self.molecule.removeAtom(1)
    # atom with id 2 should now have inex 1
    self.assertEqual(self.molecule.atomById(2).index, 1)

  def test_bond(self):
    # add 3 bonds
    for i in range(3):
      self.molecule.addBond()
    # delete the 2nd
    self.molecule.removeBond(1)
    # bond with index 1 should now have id 2
    self.assertEqual(self.molecule.bond(1).id, 2)

  def test_bond_id(self):
    # add 10 atoms & bonds
    for i in range(10):
      self.molecule.addAtom()
      self.molecule.addBond()
 
    # add a bonds
    bond = self.molecule.addBond()
    # set the atoms to id 4 & 5, order 1
    bond.setAtoms(4, 5, 1)

    # test molecule.bond(id, id)
    self.assertEqual(self.molecule.bond(4, 5).id, bond.id)

  def test_bond_Bond(self):
    # add 10 atoms & bonds
    for i in range(10):
      self.molecule.addAtom()
      self.molecule.addBond()
 
    # get atom with id 4 & 5
    atom4 = self.molecule.atomById(4)
    atom5 = self.molecule.atomById(5)

    # add a bonds
    bond = self.molecule.addBond()
    # set the atoms to id 4 & 5, order 1
    bond.setAtoms(atom4.id, atom5.id, 1)

    # test molecule.bond(id, id)
    self.assertEqual(self.molecule.bond(atom4, atom5).id, bond.id)

  def test_bondById(self):
    # add 3 bonds
    for i in range(3):
      self.molecule.addBond()
    # delete the 2nd
    self.molecule.removeBond(1)
    # bond with id 2 should now have inex 1
    self.assertEqual(self.molecule.bondById(2).index, 1)

  def test_residue(self):
    # add 3 residues
    for i in range(3):
      self.molecule.addResidue()
    # delete the 2nd
    self.molecule.removeResidue(1)
    # residue with index 1 should now have id 2
    self.assertEqual(self.molecule.residue(1).id, 2)

  def test_residueById(self):
    # add 3 residues
    for i in range(3):
      self.molecule.addResidue()
    # delete the 2nd
    self.molecule.removeResidue(1)
    # residue with id 2 should now have inex 1
    self.assertEqual(self.molecule.residueById(2).index, 1)

  def test_atoms(self):
    # add 10 atoms
    for i in range(10):
      self.molecule.addAtom()

    i = 0
    for atom in self.molecule.atoms:
      self.assertEqual(atom.type, Avogadro.PrimitiveType.AtomType)
      self.assertEqual(atom.id, i)
      i += 1
      
  def test_bonds(self):
    # add 10 bonds
    for i in range(10):
      self.molecule.addBond()

    i = 0
    for bond in self.molecule.bonds:
      self.assertEqual(bond.type, Avogadro.PrimitiveType.BondType)
      self.assertEqual(bond.id, i)
      i += 1
      
  def test_cubes(self):
    # add 10 cubes
    for i in range(10):
      self.molecule.addCube()

    i = 0
    for cube in self.molecule.cubes:
      self.assertEqual(cube.type, Avogadro.PrimitiveType.CubeType)
      self.assertEqual(cube.id, i)
      i += 1
 
  def test_residues(self):
    # add 10 residues
    for i in range(10):
      self.molecule.addResidue()

    i = 0
    for residue in self.molecule.residues:
      self.assertEqual(residue.type, Avogadro.PrimitiveType.ResidueType)
      self.assertEqual(residue.id, i)
      i += 1
 
  def test_rings(self):
    # add 10 rings
    for i in range(10):
      self.molecule.addAtom()

    i = 0
    for ring in self.molecule.rings:
      self.assertEqual(ring.type, Avogadro.PrimitiveType.FragmentType)
      self.assertEqual(ring.id, i)
      i += 1
 
  def test_clear(self):
    for i in range(5):
      self.molecule.addAtom()
      self.molecule.addBond()
      self.molecule.addResidue()
      self.molecule.addCube()
      self.molecule.addRing()
   
    self.molecule.clear()

    self.assertEqual(self.molecule.numAtoms, 0)
    self.assertEqual(self.molecule.numBonds, 0)
    self.assertEqual(self.molecule.numResidues, 0)
    self.assertEqual(self.molecule.numCubes, 0)
    self.assertEqual(self.molecule.numRings, 0)

  def test_center(self):
    atom1 = self.molecule.addAtom()
    atom2 = self.molecule.addAtom()

    atom1.pos = array([1.0, 2.0, 3.0])
    atom2.pos = array([4.0, 5.0, 6.0])

    # compute the center
    center = (atom1.pos + atom2.pos) / 2.0

    self.assertEqual(self.molecule.center[0], center[0])
    self.assertEqual(self.molecule.center[1], center[1])
    self.assertEqual(self.molecule.center[2], center[2])
  
  def test_normalVector(self):
    atom1 = self.molecule.addAtom()
    atom2 = self.molecule.addAtom()
    atom1.pos = array([1.0, 2.0, 3.0])
    atom2.pos = array([4.0, 5.0, 6.0])

    n = self.molecule.normalVector
    # just check we got an array with size 3
    self.assertEqual(len(n), 3)

  def test_radius(self):
    # just check the method is there
    self.molecule.radius

  def test_farthestAtom(self):
    # just check the method is there
    self.molecule.farthestAtom

  def test_translate(self):
    print("FIXME: Molecule::translate(Eigen::Vector3d isn't implemented)")
    # just check the method is there and accepts the array
    atom = self.molecule.addAtom()
    vec = array([1., 2., 3.])
    self.molecule.translate(vec)




   






if __name__ == "__main__":
  unittest.main()
