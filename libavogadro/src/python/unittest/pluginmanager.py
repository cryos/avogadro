import Avogadro
import unittest

class TestPluginManager(unittest.TestCase):
  def setUp(self):
    self.pm = Avogadro.PluginManager.instance
    self.pm.loadFactories()

  def test_factories(self):
    factories = self.pm.factories(Avogadro.PluginType.EngineType)
    self.assertNotEqual(len(factories), 0)
    for factory in factories:
      self.assertEqual(factory.type, Avogadro.PluginType.EngineType)
 
    factories = self.pm.factories(Avogadro.PluginType.ToolType)
    self.assertNotEqual(len(factories), 0)
    for factory in factories:
      self.assertEqual(factory.type, Avogadro.PluginType.ToolType)
  
  def test_factory(self):
    factory = self.pm.factory("Navigate Tool", Avogadro.PluginType.ToolType)
    self.assertNotEqual(factory, None)
    factory = self.pm.factory("SomeNonExistingTool", Avogadro.PluginType.ToolType)
    self.assertEqual(factory, None)

  def test_extensions(self):
    extensions = self.pm.extensions(None) 
    self.assertNotEqual(len(extensions), 0)

  def test_tools(self):
    tools = self.pm.tools(None) 
    self.assertNotEqual(len(tools), 0)

  def test_colors(self):
    colors = self.pm.colors(None) 
    self.assertNotEqual(len(colors), 0)





if __name__ == "__main__":
  unittest.main()
