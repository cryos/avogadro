import Avogadro
import unittest
from numpy import *

from PyQt4.Qt import *
import sys

class TestExtension(unittest.TestCase):
  def setUp(self):
    self.extensions = []
    for extension in Avogadro.PluginManager.instance.extensions(None):
      self.extensions.append(extension)
    
    self.assertNotEqual(len(self.extensions), 0)

  def test_typeName(self):
    for extension in self.extensions:
      self.assertEqual(extension.type, Avogadro.PluginType.ExtensionType)
      self.assertEqual(extension.typeName, "Extensions")

  def test_settingsWidget(self):
    for extension in self.extensions:
      widget = extension.settingsWidget

  def test_actions(self):
    for extension in self.extensions:
      actions = extension.actions
      widget = extension.dockWidget
      for action in actions:
        action.text()
        extension.menuPath(action)

  def test_performAction(self):
    glwidget = Avogadro.GLWidget()
    molecule = Avogadro.molecules.addMolecule()
    #glwidget.molecule = molecule
    molecule.addAtom().atomicNumber = 6

    for extension in self.extensions:
      if extension.name == "Hydrogens":
        extension.setMolecule(molecule)
        actions = extension.actions
        for action in actions:
          #print action.text(), " = ", extension.menuPath(action)
          if action.text() == "Add Hydrogens":
            command = extension.performAction(action, glwidget)
            command.redo()
            self.assertEqual(molecule.numAtoms, 5)

   
  def test_setMolecule(self):
    molecule = Avogadro.molecules.addMolecule()
    for extension in self.extensions:
      extension.setMolecule(molecule)
   



if __name__ == "__main__":
  app = QApplication(sys.argv)
  unittest.main()
  sys.exit(app.exec_())
