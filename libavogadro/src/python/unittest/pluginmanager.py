import Avogadro
import unittest

from PyQt4.Qt import *
import sys


class TestPluginManager(unittest.TestCase):
  def setUp(self):
    self.pm = Avogadro.PluginManager.instance
    self.pm.loadFactories()

  #def test_factories(self):
  #  factories = self.pm.factories(Avogadro.PluginType.EngineType)
  #  self.assertNotEqual(len(factories), 0)
  #  for factory in factories:
  #    self.assertEqual(factory.type, Avogadro.PluginType.EngineType)
  #
  #  factories = self.pm.factories(Avogadro.PluginType.ToolType)
  #  self.assertNotEqual(len(factories), 0)
  #  for factory in factories:
  #    self.assertEqual(factory.type, Avogadro.PluginType.ToolType)
  
  #def test_factory(self):
  #  factory = self.pm.factory("Navigate Tool", Avogadro.PluginType.ToolType)
  #  self.assertNotEqual(factory, None)
  #  factory = self.pm.factory("SomeNonExistingTool", Avogadro.PluginType.ToolType)
  #  self.assertEqual(factory, None)

  def test_extensions(self):
    extensions = self.pm.extensions(None) 
    self.assertNotEqual(len(extensions), 0)

  def test_tools(self):
    tools = self.pm.tools(None) 
    self.assertNotEqual(len(tools), 0)

  def test_colors(self):
    colors = self.pm.colors(None) 
    self.assertNotEqual(len(colors), 0)

  def test_names(self):
    names = self.pm.names(Avogadro.PluginType.ToolType) 
    self.assertNotEqual(len(names), 0)
    names = self.pm.names(Avogadro.PluginType.EngineType) 
    self.assertNotEqual(len(names), 0)
    names = self.pm.names(Avogadro.PluginType.ExtensionType) 
    self.assertNotEqual(len(names), 0)
    names = self.pm.names(Avogadro.PluginType.ColorType) 
    self.assertNotEqual(len(names), 0)

  def test_descriptions(self):
    descriptions = self.pm.descriptions(Avogadro.PluginType.ToolType) 
    self.assertNotEqual(len(descriptions), 0)
    descriptions = self.pm.descriptions(Avogadro.PluginType.EngineType) 
    self.assertNotEqual(len(descriptions), 0)
    descriptions = self.pm.descriptions(Avogadro.PluginType.ExtensionType) 
    self.assertNotEqual(len(descriptions), 0)
    descriptions = self.pm.descriptions(Avogadro.PluginType.ColorType) 
    self.assertNotEqual(len(descriptions), 0)


  #
  # Test if the methods are there ...
  #

  def test_extension(self):
    self.pm.extension("foo", None)

  def test_tool(self):
    self.pm.tool("foo", None)

  def test_color(self):
    self.pm.color("foo", None)

  def test_engine(self):
    self.pm.engine("foo", None)





if __name__ == "__main__":
  app = QApplication(sys.argv)
  unittest.main()
  sys.exit(app.exec_()) 
