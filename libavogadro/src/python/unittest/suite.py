import unittest

from eigen import *
from atom import *
from bond import *
from molecule import *
from fragment import *
from cube import *
from residue import *
from mesh import *
from primitivelist import *

if __name__ == "__main__":
  
  suite0 = unittest.TestLoader().loadTestsFromTestCase(TestEigen)
  
  suite1 = unittest.TestLoader().loadTestsFromTestCase(TestAtom)
  suite2 = unittest.TestLoader().loadTestsFromTestCase(TestBond)
  suite3 = unittest.TestLoader().loadTestsFromTestCase(TestMolecule)
  suite4 = unittest.TestLoader().loadTestsFromTestCase(TestFragment)
  suite5 = unittest.TestLoader().loadTestsFromTestCase(TestCube)
  suite6 = unittest.TestLoader().loadTestsFromTestCase(TestResidue)
  suite7 = unittest.TestLoader().loadTestsFromTestCase(TestMesh)
  suite8 = unittest.TestLoader().loadTestsFromTestCase(TestPrimitiveList)




  alltests = unittest.TestSuite([suite1, suite2, suite3, suite4, suite5, suite6, suite7, suite8, suite0])
  unittest.TextTestRunner(verbosity=2).run(alltests)
  #result = unittest.TestResult()
  #suite.run(result)
  #print result
