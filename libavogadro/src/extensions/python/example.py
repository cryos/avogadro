import Avogadro
from PyQt4.QtCore import *
from PyQt4.QtGui import *

class AddAtomCommand(QUndoCommand):
  def __init__(self, molecule, atomicNumber):
    super(AddAtomCommand, self).__init__(QString("Add Atom - Python"))
    self.molecule = molecule
    self.atomicNumber = atomicNumber
    self.atomId = None

  def redo(self):
    atom = self.molecule.newAtom()
    atom.atomicNumber = self.atomicNumber
    # save the id for undo method
    self.atomId = atom.id

  def undo(self):
    self.molecule.deleteAtom(self.atomId)
    self.atomId = None


class Extension(QObject):
  def __init__(self):
    QObject.__init__(self)
    # keep a reference to all commands to make sure garbage collection doesn't 
    # delete them
    self.commands = []

  def actions(self):
    actions = []

    action = QAction(self)
    action.setText("Add Atom")
    actions.append(action)
    
    action = QAction(self)
    action.setText("Clear molecule")
    actions.append(action)
    
    #action = QAction(None)
    #action.setText("Action 3")
    #self._actions.append(action)

    return actions

  def performAction(self, action, glwidget):
    if action.text() == "Add Atom":
      # AddAtomCommand(parent, molecule, atomicNumber)
      command = AddAtomCommand(glwidget.molecule, 6)
      self.commands.append(command) # prevent deletion
      return command
    elif action.text() == "Clear molecule":
      glwidget.molecule.clear()
      return None 

    return None
  
  def menuPath(self, action):
    return "foo"
    

