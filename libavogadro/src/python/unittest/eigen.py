import Avogadro
import unittest
from numpy import *

class TestEigen(unittest.TestCase):
  def setUp(self):
    self.helper = Avogadro.EigenUnitTestHelper()

  def test_vector3d(self):
    vec = self.helper.vector3d()
    vec = self.helper.vector3d_ref()
    vec = self.helper.const_vector3d_ref()
    vec = self.helper.vector3d_ptr()
    vec = self.helper.const_vector3d_ptr()
    
    self.helper.set_vector3d(vec)
    self.helper.set_vector3d_ref(vec)
    self.helper.set_const_vector3d_ref(vec)
    self.helper.set_vector3d_ptr(vec)
    self.helper.set_const_vector3d_ptr(vec)

  def test_transform3d(self):
    vec = self.helper.transform3d()
    vec = self.helper.transform3d_ref()
    vec = self.helper.const_transform3d_ref()
    vec = self.helper.transform3d_ptr()
    vec = self.helper.const_transform3d_ptr()
    
    self.helper.set_transform3d(vec)
    self.helper.set_transform3d_ref(vec)
    self.helper.set_const_transform3d_ref(vec)
    self.helper.set_transform3d_ptr(vec)
    self.helper.set_const_transform3d_ptr(vec)

  



if __name__ == "__main__":
  unittest.main()
