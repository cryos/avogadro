import Avogadro
import unittest
from numpy import *

from PyQt4.Qt import *
import sys

class TestTool(unittest.TestCase):
  def setUp(self):
    self.tools = []
    for tool in Avogadro.PluginManager.instance.tools(None):
      self.tools.append(tool)
    
    self.assertNotEqual(len(self.tools), 0)


  def test_typeName(self):
    for tool in self.tools:
      self.assertEqual(tool.type, Avogadro.PluginType.ToolType)
      self.assertEqual(tool.typeName, "Tools")

  def test_settingsWidget(self):
    for tool in self.tools:
      widget = tool.settingsWidget

  def test_usefulness(self):
    for tool in self.tools:
      tool.usefulness


    
  def test_setMolecule(self):
    molecule = Avogadro.molecules.addMolecule()
    for tool in self.tools:
      tool.setMolecule(molecule)
   

    
 




if __name__ == "__main__":
  app = QApplication(sys.argv)
  unittest.main()
  sys.exit(app.exec_())
