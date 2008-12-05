import Avogadro
import unittest
from numpy import *

class TestPluginFactory(unittest.TestCase):
  def setUp(self):
    self.pm = Avogadro.PluginManager.instance
    self.pm.loadFactories()

  def test_PluginFactory(self):
    factories = self.pm.factories(Avogadro.PluginType.EngineType)
    self.assertNotEqual(len(factories), 0)
    for factory in factories:
      self.assertEqual(factory.type, Avogadro.PluginType.EngineType)
      factory.name
      factory.description
      engine = factory.createInstance(None)
      self.assertEqual(engine.type, Avogadro.PluginType.EngineType)
 
 



if __name__ == "__main__":
  unittest.main()
