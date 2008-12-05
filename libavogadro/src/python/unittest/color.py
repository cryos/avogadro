import Avogadro
import unittest
from numpy import *

from PyQt4.Qt import *
import sys

class TestColor(unittest.TestCase):
  def setUp(self):
    self.colors = []
    for color in Avogadro.PluginManager.instance.colors(None):
      self.colors.append(color)
    
    self.assertNotEqual(len(self.colors), 0)

  def test_constructors(self):
    color1 = Avogadro.Color(1.0, 0.0, 0.0, 1.0)
    self.assertNotEqual(color1, None)

    molecule = Avogadro.Molecule()
    atom = molecule.newAtom()
    atom.atomicNumber = 6
    color2 = Avogadro.Color(atom)
    self.assertNotEqual(color2, None)
    
    color3 = Avogadro.Color()
    self.assertNotEqual(color3, None)
  
  def test_typeName(self):
    for color in self.colors:
      self.assertEqual(color.type, Avogadro.PluginType.ColorType)

  def test_settingsWidget(self):
    for color in self.colors:
      widget = color.settingsWidget

  def test_set(self):
    molecule = Avogadro.Molecule()
    atom = molecule.newAtom()
    atom.atomicNumber = 6
 
    color = Avogadro.Color()
    color.set(0.0, 1.0, 0.0, 1.0)
    color.set(atom)
    color.alpha = 0.5
    
  def test_setToSelectionColor(self):
    color = Avogadro.Color()
    color.setToSelectionColor()
  
  def test_getters(self):
    color = Avogadro.Color(0.1, 0.2, 0.3, 0.4)
    self.assertAlmostEqual(color.red, 0.1)
    self.assertAlmostEqual(color.green, 0.2)
    self.assertAlmostEqual(color.blue, 0.3)
    self.assertAlmostEqual(color.alpha, 0.4)
   
  def _test_apply(self):
    color = Avogadro.Color()
    color.apply()
    color.applyAsMaterial()
    color.applyAsFlatMaterial()
 
  def test_name(self):
    color = Avogadro.Color()
    color.name = "testing"
    self.assertEqual(color.name, "testing")
 
 




if __name__ == "__main__":
  app = QApplication(sys.argv)
  unittest.main()
  sys.exit(app.exec_())
