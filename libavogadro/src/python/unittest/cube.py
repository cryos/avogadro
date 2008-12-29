import Avogadro
import unittest
from numpy import *

class TestCube(unittest.TestCase):
  def setUp(self):
    self.molecule = Avogadro.Molecule()

  def test_min(self):
    cube = self.molecule.addCube()
    self.assertEqual(len(cube.min), 3)

  def test_max(self):
    cube = self.molecule.addCube()
    self.assertEqual(len(cube.max), 3)

  def test_spacing(self):
    cube = self.molecule.addCube()
    self.assertEqual(len(cube.spacing), 3)

  def test_dimensions(self):
    cube = self.molecule.addCube()
    self.assertEqual(len(cube.dimensions), 3)

  # test setLimits(Vector3d, Vector3d, Vector3i)
  def test_setLimits_points(self):
    cube = self.molecule.addCube()
    min = array([-10.0, -10.0, -10.0])
    max = array([10.0, 10.0, 10.0])
    dimensions = array([20, 10, 4])
    # setLimits(min, max, dimensions)
    cube.setLimits(min, max, dimensions)
    
    self.assertEqual(cube.min[0], min[0])
    self.assertEqual(cube.min[1], min[1])
    self.assertEqual(cube.min[2], min[2])
    
    self.assertEqual(cube.max[0], max[0])
    self.assertEqual(cube.max[1], max[1])
    self.assertEqual(cube.max[2], max[2])
 
    self.assertEqual(cube.dimensions[0], dimensions[0])
    self.assertEqual(cube.dimensions[1], dimensions[1])
    self.assertEqual(cube.dimensions[2], dimensions[2])
   
    self.assertEqual(cube.spacing[0], 20.0 / 19)
    self.assertEqual(cube.spacing[1], 20.0 / 9 )
    self.assertEqual(cube.spacing[2], 20.0 / 3)

  # test setLimits(Vector3d, Vector3d, double)
  def test_setLimits_spacing(self):
    cube = self.molecule.addCube()
    min = array([-10.0, -10.0, -10.0])
    max = array([10.0, 10.0, 10.0])
    # setLimits(min, max, spacing)
    cube.setLimits(min, max, 1.0)
    
    self.assertEqual(cube.min[0], min[0])
    self.assertEqual(cube.min[1], min[1])
    self.assertEqual(cube.min[2], min[2])
    
    self.assertEqual(cube.max[0], max[0])
    self.assertEqual(cube.max[1], max[1])
    self.assertEqual(cube.max[2], max[2])
 
    self.assertEqual(cube.dimensions[0], 20)
    self.assertEqual(cube.dimensions[1], 20)
    self.assertEqual(cube.dimensions[2], 20)
   
    self.assertEqual(cube.spacing[0], 20.0 / 19)
    self.assertEqual(cube.spacing[1], 20.0 / 19)
    self.assertEqual(cube.spacing[2], 20.0 / 19)

  # test setLimits(Vector3d, Vector3i, double)
  def test_setLimits_dimensions_spacing(self):
    cube = self.molecule.addCube()
    min = array([-10.0, -10.0, -10.0])
    dimensions = array([20, 10, 4])
    # setLimits(min, dimensions, spacing)
    cube.setLimits(min, dimensions, 1.0)
    
    self.assertEqual(cube.min[0], min[0])
    self.assertEqual(cube.min[1], min[1])
    self.assertEqual(cube.min[2], min[2])
    
    self.assertEqual(cube.max[0], 9.0)
    self.assertEqual(cube.max[1], -1.0)
    self.assertEqual(cube.max[2], -7.0)
 
    self.assertEqual(cube.dimensions[0], 20)
    self.assertEqual(cube.dimensions[1], 10)
    self.assertEqual(cube.dimensions[2], 4)
   
    self.assertEqual(cube.spacing[0], 1.0)
    self.assertEqual(cube.spacing[1], 1.0)
    self.assertEqual(cube.spacing[2], 1.0)

  # test setLimits(Molecule, double, double)
  def test_setLimits_molecule(self):
    cube = self.molecule.addCube()
    atom = self.molecule.addAtom()
    atom.pos = array([1., 1., 1.])
    # setLimits(Molecle, spacing, padding)
    cube.setLimits(self.molecule, 1.0, 2.0)
    
    self.assertEqual(cube.min[0], -1.0)
    self.assertEqual(cube.min[1], -1.0)
    self.assertEqual(cube.min[2], -1.0)
    
    self.assertEqual(cube.max[0], 3.0)
    self.assertEqual(cube.max[1], 3.0)
    self.assertEqual(cube.max[2], 3.0)
 
    self.assertEqual(cube.dimensions[0], 4)
    self.assertEqual(cube.dimensions[1], 4)
    self.assertEqual(cube.dimensions[2], 4)
   
    self.assertEqual(cube.spacing[0], 4.0 / 3)
    self.assertEqual(cube.spacing[1], 4.0 / 3)
    self.assertEqual(cube.spacing[2], 4.0 / 3)

  def test_data(self):
    cube = self.molecule.addCube()
    self.assertEqual(len(cube.data), 0)
  
    # 5x5x5 = 125 data points 
    min = array([0.0, 0.0, 0.0])
    dimensions = array([5, 5, 5])
    cube.setLimits(min, dimensions, 1.0)

    # create the data list
    data = []
    # check if setting data with incorrect # of points return false
    self.assertEqual(cube.setData(data), False)

    for i in range(125):
      data.append(i)
    self.assertEqual(len(data), 125)

    cube.data = data
    self.assertEqual(len(cube.data), 125)
  
  def test_index(self):
    cube = self.molecule.addCube()
    min = array([0.0, 0.0, 0.0])
    dimensions = array([5, 5, 5])
    cube.setLimits(min, dimensions, 1.0)

    # just check the methods are there and they are callable
    cube.closestIndex(array([1., 1., 1.]))
    vec = cube.indexVector(array([1., 1., 1.]))
    self.assertEqual(len(vec), 3)
    vec = cube.position(3)

  def test_value(self):
    cube = self.molecule.addCube()
    # 5x5x5 = 125 data points 
    min = array([0.0, 0.0, 0.0])
    dimensions = array([5, 5, 5])
    cube.setLimits(min, dimensions, 2.0)

    # create the data list
    data = []
    for i in range(125):
      data.append(i)
    self.assertEqual(cube.setData(data), True)

    # test value(int, int, int)
    value1 = cube.value(2, 3, 4)
    # test value(Vector3i)
    value2 = cube.value(array([2, 3, 4]))
    # test value(Vector3d)
    value3 = cube.value(array([2.0, 3.0, 4.0]))
    # make sure they return the same.
    self.assertEqual(value1, value2)
    self.assertNotEqual(value1, value3)

    # change the value
    cube.setValue(2, 3, 4, 5.5)
    self.assertEqual(cube.value(2, 3, 4), 5.5)


 
  def test_name(self):
    cube = self.molecule.addCube()
    cube.name = "testing"
    self.assertEqual(cube.name, "testing")
 


    

 






if __name__ == "__main__":
  unittest.main()
