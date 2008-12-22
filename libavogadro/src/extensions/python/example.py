import Avogadro
from PyQt4.QtCore import *
from PyQt4.QtGui import *

class Extension():
  def __init__(self):
    # keep a reference to all actions to make sure garbage collection doesn't 
    # delete our actions
    self._actions = []

  def actions(self):
    action = QAction(None)
    action.setText("Add Atom")
    self._actions.append(action)
    
    action = QAction(None)
    action.setText("Clear molecule")
    self._actions.append(action)
    
    #action = QAction(None)
    #action.setText("Action 3")
    #self._actions.append(action)

    return self._actions

  def performAction(self, action, glwidget):
    if action.text() == "Add Atom":
      atom = glwidget.molecule.newAtom()
      atom.atomicNumber = 6
    elif action.text() == "Clear molecule":
      glwidget.molecule.clear()

    return None
  
  def menuPath(self, action):
    return "foo"
    

