from PyQt4.Qt import *
from numpy import *
import Avogadro
import sys
import unittest


class TestGLWidget(unittest.TestCase):
  def setUp(self):
    # create the GLWidget and load the default engines
    self.glwidget = Avogadro.GLWidget()
    self.glwidget.loadDefaultEngines()
    
    self.molecule = Avogadro.Molecule()
    self.molecule.newAtom()
    self.glwidget.molecule = self.molecule

  def tearDown(self):
    # create the GLWidget and load the default engines
    None
    


  def _test_constructors(self):
    self.qwidget = QWidget()
    self.glwidget2 = Avogadro.GLWidget(self.qwidget)
    #Avogadro.toPyQt(glwidget).show()
    #del self.qwidget

  def test_quickRender(self):
    self.glwidget.quickRender
    self.glwidget.quickRender = True
    self.assertEqual(self.glwidget.quickRender, True)
 
  def test_quality(self):
    print self.glwidget.quality
    self.glwidget.quality = 4
    print self.glwidget.quality
    self.assertEqual(self.glwidget.quality, 3)
  
  def test_fogLevel(self):
    self.glwidget.fogLevel
    self.glwidget.fogLevel = True
    self.assertEqual(self.glwidget.fogLevel, True)

  def test_renderAxes(self):
    self.glwidget.renderAxes
    self.glwidget.renderAxes = True
    self.assertEqual(self.glwidget.renderAxes, True)

  def test_renderDebug(self):
    self.glwidget.renderDebug
    self.glwidget.renderDebug = True
    self.assertEqual(self.glwidget.renderDebug, True)

  def test_colorMap(self):
    self.glwidget.colorMap
    self.glwidget.colorMap = Avogadro.Color()
    self.assertNotEqual(self.glwidget.colorMap, None)

  def test_undoStack(self):
    self.glwidget.undoStack
    self.stack = QUndoStack()
    self.glwidget.undoStack = self.stack
    self.assertNotEqual(self.glwidget.undoStack, None)

  def test_updateGeometry(self):
    self.glwidget.updateGeometry()

  def test_camera(self):
    self.assertNotEqual(self.glwidget.camera, None)

  def test_engines(self):
    self.assertNotEqual(len(self.glwidget.engines), 0)

  def test_hits(self):
    self.assertNotEqual(self.glwidget.hits(0,0,50,50), None)
    for hit in self.glwidget.hits(0, 0, self.glwidget.deviceWidth, self.glwidget.deviceHeight):
      self.assert_(hit.name)
      self.assert_(hit.type)
      self.assert_(hit.minZ)
      self.assert_(hit.maxZ)

  def test_computeClicked(self):
    point = QPoint(10,45)
    self.glwidget.computeClickedPrimitive(point)
    self.glwidget.computeClickedAtom(point)
    self.glwidget.computeClickedBond(point)


  def test_toolGroup(self):
    self.glwidget.toolGroup
    self.glwidget.toolGroup = Avogadro.ToolGroup()
    self.assertNotEqual(self.glwidget.toolGroup, None)


  def test_deviceWidthHeight(self):
    self.assertNotEqual(self.glwidget.deviceWidth, 0)
    self.assertNotEqual(self.glwidget.deviceHeight, 0)

  def test_radius(self):
    self.glwidget.radius()
    self.glwidget.radius(None)
  
  def test_radius(self):
    self.assertNotEqual(self.glwidget.farthestAtom, None)

  def test_tool(self):
    tool = Avogadro.PluginManager.instance.tool("Navigate Tool", None)
    self.assertNotEqual(tool, None)
    self.assertEqual(self.glwidget.tool, None)
    self.glwidget.tool = tool
    self.assertNotEqual(self.glwidget.tool, None)

  def test_painter(self):
    self.assertNotEqual(self.glwidget.painter, None)
  
  def test_background(self):
    self.assertNotEqual(self.glwidget.background, None)
    color = QColor()
    self.glwidget.background = color

  
  def test_primitives(self):
    l = Avogadro.PrimitiveList([ self.molecule.atom(0) ])
    self.assertNotEqual(self.glwidget.primitives, None)
    self.assertNotEqual(self.glwidget.selectedPrimitives, None)
    self.glwidget.toggleSelected(self.glwidget.primitives)
    self.glwidget.setSelected(l, True)
    self.assertEqual(self.glwidget.isSelected(self.molecule.atom(0)), True)
    self.glwidget.clearSelected()
    self.assertEqual(self.glwidget.isSelected(self.molecule.atom(0)), False)
  
  def test_namedSelections(self):
    l = Avogadro.PrimitiveList([ self.molecule.atom(0) ])
    self.glwidget.addNamedSelection("test", l)
    
    self.assertNotEqual(len(self.glwidget.namedSelections), 0)
    self.assertEqual(self.glwidget.namedSelections[0], "test")
    
    self.glwidget.removeNamedSelection("test")
    self.assertEqual(len(self.glwidget.namedSelections), 0)
    
    self.glwidget.addNamedSelection("test", l)
    self.glwidget.removeNamedSelection(0)
    self.assertEqual(len(self.glwidget.namedSelections), 0)
    
    self.glwidget.addNamedSelection("test", l)
    self.glwidget.renameNamedSelection(0, "test2")
    self.assertEqual(self.glwidget.namedSelections[0], "test2")

    self.assertNotEqual(self.glwidget.namedSelectionPrimitives("test2"), None)
    self.assertNotEqual(self.glwidget.namedSelectionPrimitives(0), None)

  def test_eigen(self):
    self.assertEqual(len(self.glwidget.center), 3)
    self.assertEqual(len(self.glwidget.normalVector), 3)

  def test_unitcells(self):
    self.glwidget.aCells
    self.glwidget.bCells
    self.glwidget.cCells
    self.glwidget.clearUnitCell()

  def test_slots(self):
    self.glwidget.addPrimitive(self.molecule.atom(0))
    self.glwidget.updatePrimitive(self.molecule.atom(0))
    self.glwidget.removePrimitive(self.molecule.atom(0))

  def test_engine(self):
    engine = Avogadro.PluginManager.instance.engine("Label", Avogadro.toPyQt(self.glwidget))
    #engine = Avogadro.PluginManager.instance.engine("foo", N)
    self.assertNotEqual(engine, None)
    self.glwidget.addEngine(engine)
    self.glwidget.removeEngine(engine)


if __name__ == "__main__":
  # create a new application
  # (must be done before creating a GLWidget)
  app = QApplication(sys.argv)
  unittest.main()

  sys.exit(app.exec_())
