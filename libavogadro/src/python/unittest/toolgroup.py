import Avogadro
import unittest
from numpy import *

from PyQt4.Qt import *
import sys

class TestToolGroup(unittest.TestCase):
  def setUp(self):
    self.toolGroup = Avogadro.ToolGroup()


  def test_append(self):
    for tool in Avogadro.PluginManager.instance.tools(None):
      self.toolGroup.append(tool)

    self.assertNotEqual(len(self.toolGroup.tools), 0)

  def test_appendList(self):
    tools = []
    for tool in Avogadro.PluginManager.instance.tools(None):
      tools.append(tool)
    self.toolGroup.append(tools)
    
    self.assertNotEqual(len(self.toolGroup.tools), 0)

  def test_activeTool(self):
    self.assertEqual(self.toolGroup.activeTool, None)

    tools = Avogadro.PluginManager.instance.tools(None)
    self.assertNotEqual(len(tools), 0)
    self.toolGroup.activeTool = tools[0]
    self.assertNotEqual(self.toolGroup.activeTool, None)
    

  def test_tool(self):
    self.assertEqual(self.toolGroup.tool(99), None)
    toolNames = []
    for tool in Avogadro.PluginManager.instance.tools(None):
      self.toolGroup.append(tool)
      toolNames.append(tool.name)
    
    self.assertNotEqual(len(self.toolGroup.tools), 0)
    
    for i in range(len(toolNames)):
      tool = self.toolGroup.tool(i)
  
  def test_tools(self):
    for tool in Avogadro.PluginManager.instance.tools(None):
      tool = self.toolGroup.append(tool)
    
    self.assertNotEqual(len(self.toolGroup.tools), 0)
    
  def test_setActiveTools(self):
    self.toolGroup.setActiveTool(99)
    for tool in Avogadro.PluginManager.instance.tools(None):
      tool = self.toolGroup.append(tool)

    self.toolGroup.setActiveTool("SomeNonExistingTool")
    self.assertEqual(self.toolGroup.activeTool, None)
 
    self.toolGroup.setActiveTool(0)
    self.assertNotEqual(self.toolGroup.activeTool, None)
    
    self.toolGroup.setActiveTool("Navigate")
    self.assertNotEqual(self.toolGroup.activeTool, None)
    
  def test_setMolecule(self):
    molecule = Avogadro.molecules.addMolecule()
    self.toolGroup.setMolecule(molecule)
   

    
 




if __name__ == "__main__":
  app = QApplication(sys.argv)
  unittest.main()
  sys.exit(app.exec_())
