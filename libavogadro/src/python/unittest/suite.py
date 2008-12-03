import unittest

from atom import *
from bond import *
from eigen import *

if __name__ == "__main__":
  suite1 = unittest.TestLoader().loadTestsFromTestCase(TestAtom)
  suite2 = unittest.TestLoader().loadTestsFromTestCase(TestBond)
  suite3 = unittest.TestLoader().loadTestsFromTestCase(TestEigen)

  alltests = unittest.TestSuite([suite1, suite2, suite3])
  unittest.TextTestRunner(verbosity=2).run(alltests)
  #result = unittest.TestResult()
  #suite.run(result)
  #print result
