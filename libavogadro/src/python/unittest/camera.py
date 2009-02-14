from PyQt4.Qt import *
from numpy import *
import Avogadro
import sys
import unittest


class TestCamera(unittest.TestCase):
  def setUp(self):
    # create the GLWidget and load the default engines
    self.glwidget = Avogadro.GLWidget()
    self.glwidget.loadDefaultEngines()
    
    self.molecule = Avogadro.molecules.addMolecule()
    self.molecule.addAtom()
    self.glwidget.molecule = self.molecule
    self.assertNotEqual(self.glwidget.camera, None)

  def tearDown(self):
    # create the GLWidget and load the default engines
    None
    

  def test_parent(self):
    self.assertNotEqual(self.glwidget.camera.parent, None)
  
  def test_angleOfViewY(self):
    self.assert_(self.glwidget.camera.angleOfViewY)
    self.glwidget.camera.angleOfViewY = 40.0
    self.assertEqual(self.glwidget.camera.angleOfViewY, 40.0)

  def test_modelview(self):
    self.glwidget.camera.modelview
    m = self.glwidget.camera.modelview
    self.glwidget.camera.modelview = m
  
  def test_various(self):
    self.glwidget.camera.applyPerspective()
    self.glwidget.camera.applyModelview()
    self.glwidget.camera.initializeViewPoint()
    dist = self.glwidget.camera.distance(array([0., 0., 0.]))
    self.glwidget.camera.translate(array([0., 0., 0.]))
    self.glwidget.camera.pretranslate(array([0., 0., 0.]))
    self.glwidget.camera.rotate(3.14, array([0., 0., 0.]))
    self.glwidget.camera.prerotate(3.14, array([0., 0., 0.]))
    self.glwidget.camera.normalize()
  
  def test_axes(self):
    self.glwidget.camera.transformedXAxis
    self.glwidget.camera.transformedYAxis
    self.glwidget.camera.transformedZAxis
    self.glwidget.camera.backTransformedXAxis
    self.glwidget.camera.backTransformedYAxis
    self.glwidget.camera.backTransformedZAxis
  
  def test_project(self):
    
    point = QPoint(10,20)
    self.assertEqual(len(self.glwidget.camera.unProject(point)), 3)
    self.assertEqual(len(self.glwidget.camera.unProject(point, array([1., 0., 0.]))), 3)
    # added to fix name conflict WithZ
    self.assertEqual(len(self.glwidget.camera.unProjectWithZ(array([1., 2., 0.]))), 3)

    self.assertEqual(len(self.glwidget.camera.project(array([1., 2., 3.]))), 3)






if __name__ == "__main__":
  # create a new application
  # (must be done before creating a GLWidget)
  app = QApplication(sys.argv)
  unittest.main()
  sys.exit(app.exec_())
