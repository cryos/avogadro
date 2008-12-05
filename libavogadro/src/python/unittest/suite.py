import unittest

from PyQt4.Qt import *
import sys

from eigen import *
from atom import *
from bond import *
from molecule import *
from fragment import *
from cube import *
from residue import *
from mesh import *
from primitivelist import *
from pluginmanager import *
from toolgroup import *
from pluginfactory import *

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
  suite9 = unittest.TestLoader().loadTestsFromTestCase(TestPluginManager)
  suite10 = unittest.TestLoader().loadTestsFromTestCase(TestToolGroup)
  suite11 = unittest.TestLoader().loadTestsFromTestCase(TestToolGroup)

  alltests = unittest.TestSuite([suite1, suite2, suite3, suite4, suite5, suite6, suite7, suite8, suite9, suite10, 
      suite11, suite0])
  
  app = QApplication(sys.argv)
  unittest.TextTestRunner(verbosity=2).run(alltests)
  #sys.exit(app.exec_())
