import sys
import Avogadro
from PyQt4 import QtGui

def name():
    return "Test Script"

def description():
    return "My Little Test Script"

def extension():
    box = QtGui.QMessageBox.information(None,"Hello World!", "PyQt & Python Scripting is Active!")
#    widget = QtGui.QMessageBox()
#    widget.setWindowTitle("Success")
#    widget.setText("PyQt Test was a Success!")
#    widget.show()
#    widget.resize(250, 150)
#    widget.setWindowTitle('Test')
#    widget.show()


print "test"
