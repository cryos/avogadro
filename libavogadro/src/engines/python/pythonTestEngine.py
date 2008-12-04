import Avogadro
from numpy import * # array, ...

def name():
    return "PyEngine test"

def description():
    return "Delete all the Atoms"

def renderOpaque(pd):
    painter = pd.painter
    painter.drawText(10,10, "Testing python engine")

    painter.drawSphere(array([0.0, 0.0, 0.0]), 1.0)
