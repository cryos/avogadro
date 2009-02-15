from PyQt4.QtCore import *
from PyQt4.QtGui import *
import Avogadro

class ClearCommand(QUndoCommand):
  def __init__(self, glwidget):
    QUndoCommand.__init__(self)

    # store the glwidget for use in undo/redo
    self.glwidget = glwidget
    # create a new molecule
    self.molCopy = Avogadro.molecules.addMolecule()
    # make a copy of the molecule for undo
    self.molCopy.copy(glwidget.molecule)
    # set the command's text
    self.setText("Clear Molecule")

  def redo(self):
    # clear the current molecule
    self.glwidget.molecule.clear()

  def undo(self):
    # Restore the molecule, we use the copy function again
    # because we might need to undo/redo multiple times.
    # If we just set the current molecule to molCopy, the next
    # redo would clear it and we wouldn't have a valid copy 
    # anymore.
    self.glwidget.molecule.copy(self.molCopy)

# always use 'Extension' for class name
class Extension(QObject):
  def __init__(self):
    QObject.__init__(self)

  def name(self):
    return "Example Python Extension"

  def description(self):
    return "This example clears the molecule"

  def actions(self):
    actions = []

    action = QAction(self)
    action.setText("Clear molecule")
    actions.append(action)

    return actions

  def menuPath(self, action):
    return "Python Examples"

  def performAction(self, action, glwidget):
    # return the undo command (ownership will be handled automatically)
    return ClearCommand(glwidget)

  def dockWidget(self):
    widget = QDockWidget("Python Example DockWidget")
    label = QLabel("This is a python QDockWidget")
    widget.setWidget(label)
    return widget

  def readSettings(self, settings):
    self.foo = settings.value("foo", QVariant(42))
  
  def writeSettings(self, settings):
    settings.setValue("foo", self.foo)
