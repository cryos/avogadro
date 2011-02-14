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
    layout = QVBoxLayout(self.widget)
    self.widget.setLayout(layout)
    label = QLabel("Wireframe width:", self.widget)

    slider = QSlider(Qt.Horizontal, self.widget)
    slider.setMinimum(1)
    slider.setMaximum(5)
    slider.setValue(self.width)

    layout.addWidget(label)
    layout.addWidget(slider)
    layout.addStretch()

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

      color.setFromPrimitive(beginAtom)
      painter.setColor(color)
      painter.drawLine(beginAtom.pos, center, self.width)

      color.setFromPrimitive(endAtom)
      painter.setColor(color)
      painter.drawLine(endAtom.pos, center, self.width)

  def readSettings(self, settings):
    # As opposed to C++, in PyQt4 toInt() returns a tuple,
    # converted value is the first element
    self.width = settings.value("width", 1).toInt()[0]

  def writeSettings(self, settings):
    settings.setValue("width", self.width)
