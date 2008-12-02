import sys
import Avogadro

def name():
    return "PyEngine test"

def description():
    return "Delete all the Atoms"

def renderOpaque(pd):
    print "engine.py: renderOpaque(pd)"

    painter = pd.painter
    painter.drawText(10,10, "Testing python engine")

