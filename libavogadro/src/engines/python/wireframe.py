import sys
import Avogadro

def name():
    return "Python Wireframe Engine"

def description():
    return "A simple wireframe engine in python"

def renderOpaque(pd):
    # Painter 
    painter = pd.painter
    # Molecule
    molecule = pd.molecule

    # Color
    color = pd.colorMap

    # set color to white
    painter.setColor(1.0, 1.0, 1.0, 1.0)
    # draw some text
    painter.drawText(10,10, "Python wireframe engine")

    # set color to green
    for bond in molecule.bonds:
      beginAtom = molecule.atomById(bond.beginAtomId)
      endAtom = molecule.atomById(bond.endAtomId)

      delta = beginAtom.pos - endAtom.pos
      #center = (beginAtom.pos + endAtom.pos + delta) / 2
      center = (beginAtom.pos + endAtom.pos) / 2

      color.set(beginAtom)
      painter.setColor(color)
      painter.drawLine(beginAtom.pos, center, 2.0)

      color.set(endAtom)
      painter.setColor(color)
      painter.drawLine(endAtom.pos, center, 2.0)

