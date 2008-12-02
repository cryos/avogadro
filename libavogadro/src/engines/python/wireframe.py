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

    # set color to white
    painter.setColor(1.0, 1.0, 1.0, 1.0)
    # draw some text
    painter.drawText(10,10, "Python wireframe engine")

    # set color to green
    painter.setColor(0.0, 1.0, 0.0, 1.0) 
    for bond in molecule.bonds:
      beginAtom = molecule.atomById(bond.beginAtomId)
      endAtom = molecule.atomById(bond.endAtomId)

      painter.drawLine(beginAtom.pos, endAtom.pos, 2.0)

