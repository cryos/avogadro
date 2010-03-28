from PyQt4.QtCore import *
from PyQt4.QtGui import *
from numpy import *
import Avogadro

# always use 'Extension' for class name
class Extension(QObject):
  def __init__(self):
    QObject.__init__(self)

  def name(self):
    return "My Extension"

  def description(self):
    return "Extension for ..."

  def actions(self):
    actions = []

    action = QAction(self)
    action.setText("Some action")
    actions.append(action)

    return actions

  def menuPath(self, action):
    return "Extensions"

  def performAction(self, action, glwidget):
    if action.text() == "Some action":
      # do something...
      return None
