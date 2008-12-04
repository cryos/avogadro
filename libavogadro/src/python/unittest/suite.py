import unittest

from eigen import *
from atom import *
from bond import *
from molecule import *
from fragment import *

if __name__ == "__main__":
  
  suite0 = unittest.TestLoader().loadTestsFromTestCase(TestEigen)
  
  suite1 = unittest.TestLoader().loadTestsFromTestCase(TestAtom)
  suite2 = unittest.TestLoader().loadTestsFromTestCase(TestBond)
  suite3 = unittest.TestLoader().loadTestsFromTestCase(TestMolecule)
  suite4 = unittest.TestLoader().loadTestsFromTestCase(TestFragment)




  alltests = unittest.TestSuite([suite1, suite2, suite3, suite4, suite0])
  unittest.TextTestRunner(verbosity=2).run(alltests)
  #result = unittest.TestResult()
  #suite.run(result)
  #print result
