import Avogadro
import unittest
from numpy import *

from PyQt4.Qt import *
import sys

class TestEngine(unittest.TestCase):
  def setUp(self):
    self.engines = []
    for name in Avogadro.PluginManager.instance.names(Avogadro.PluginType.EngineType):
      engine = Avogadro.PluginManager.instance.engine(name, None)
      self.engines.append(engine)
    
    self.assertNotEqual(len(self.engines), 0)

  def test_typeName(self):
    for engine in self.engines:
      self.assertEqual(engine.type, Avogadro.PluginType.EngineType)
      self.assertEqual(engine.typeName, "Engines")

  def test_settingsWidget(self):
    for engine in self.engines:
      print(engine.name)
      if engine.name != "Orbitals":
        widget = engine.settingsWidget
      else:
        print("FIXME: OrbitalEngine::settingsWidget() not working!")

  def test_alias(self):
    for engine in self.engines:
      self.assertEqual(engine.alias, engine.name)
      engine.alias = "testing"
      self.assertEqual(engine.alias, "testing")

  def test_shader(self):
    for engine in self.engines:
      self.assertEqual(engine.shader, 0)
      engine.shader = 1
      self.assertEqual(engine.shader, 1)

  def test_layers(self):
    for engine in self.engines:
      engine.layers
    
#  def test_primitives(self):
#    molecule = Avogadro.molecules.addMolecule()
#    for i in range(10):
#      molecule.addAtom()
#
#    list = Avogadro.PrimitiveList(molecule.atoms)
#
#    for engine in self.engines:
#      self.assertEqual(engine.primitives.size, 0)
#      engine.primitives = list
#      self.assertNotEqual(engine.primitives.size, 0)
#      engine.clearPrimitives()
#      self.assertEqual(engine.primitives.size, 0)
#
#      for i in range(10):
#        self.assertEqual(engine.primitives.size, i)
#        engine.updatePrimitive(molecule.atom(i))
#        engine.addPrimitive(molecule.atom(i))
#        engine.updatePrimitive(molecule.atom(i))
#
#      for i in range(10):
#        self.assertEqual(engine.primitives.size, 10 - i)
#        engine.removePrimitive(molecule.atom(i))


  def test_enabled(self):
    for engine in self.engines:
      engine.enabled = True
      self.assertEqual(engine.enabled, True)

  def test_colorMap(self):
    color = Avogadro.Color()
    for engine in self.engines:
      self.assertEqual(engine.colorMap, None)
      engine.colorMap = color
      self.assertNotEqual(engine.colorMap, None)

  def test_clone(self):
    for engine in self.engines:
      if engine.name != "Python":
        name = engine.name
        clonedEngine = engine.clone()
        self.assertNotEqual(clonedEngine, None)
        del clonedEngine
        self.assertEqual(engine.name, name)
      else:
        print("FIXME: PythonEngine::clone() is not working...")






if __name__ == "__main__":
  app = QApplication(sys.argv)
  unittest.main()
  sys.exit(app.exec_())
