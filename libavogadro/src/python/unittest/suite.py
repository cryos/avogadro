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
from tool import *
from color import *
from engine import *
from extension import *
from glwidget import *
from camera import *

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
  suite12 = unittest.TestLoader().loadTestsFromTestCase(TestTool)
  suite13 = unittest.TestLoader().loadTestsFromTestCase(TestColor)
  suite14 = unittest.TestLoader().loadTestsFromTestCase(TestEngine)
  suite15 = unittest.TestLoader().loadTestsFromTestCase(TestExtension)
  suite16 = unittest.TestLoader().loadTestsFromTestCase(TestGLWidget)
  suite17 = unittest.TestLoader().loadTestsFromTestCase(TestCamera)

  alltests = unittest.TestSuite([suite1, suite2, suite3, suite4, suite5, suite6, suite7, suite8, suite9, suite10, 
      suite12, suite13, suite14, suite15, suite16, suite17, suite0])
  
  app = QApplication(sys.argv)
  unittest.TextTestRunner(verbosity=2).run(alltests)
  #sys.exit(app.exec_())
