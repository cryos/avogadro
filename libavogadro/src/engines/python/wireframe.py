from PyQt4.Qt import *
import Avogadro


class Engine(QObject):
  # declare the changed() signal
  __pyqtSignals__ = ("changed()",)

  # constructor
  def __init__(self):
    QObject.__init__(self)
    self.widget = None
    self.width = 1

  def name(self):
    return "Python Wireframe Engine"

  def flags(self):
    return Avogadro.EngineFlags.NoFlags

  # slot
  @pyqtSignature("int")
  def sliderChanged(self, value):
    self.width = value
    self.emit(SIGNAL("changed()"))
    
  def settingsWidget(self):
    self.widget = QWidget()
    slider = QSlider(Qt.Horizontal, self.widget)
    slider.setMinimum(1)
    slider.setMaximum(5)

    QObject.connect(slider, SIGNAL("valueChanged(int)"), self, SLOT("sliderChanged(int)"))

    return self.widget

  def renderOpaque(self, pd):
    # Painter 
    painter = pd.painter
    # Molecule
    molecule = pd.molecule

    # Color
    color = pd.colorMap

    for bond in molecule.bonds:
      beginAtom = molecule.atomById(bond.beginAtomId)
      endAtom = molecule.atomById(bond.endAtomId)

      delta = beginAtom.pos - endAtom.pos
      center = (beginAtom.pos + endAtom.pos) / 2

      color.set(beginAtom)
      painter.setColor(color)
      painter.drawLine(beginAtom.pos, center, self.width)

      color.set(endAtom)
      painter.setColor(color)
      painter.drawLine(endAtom.pos, center, self.width)

