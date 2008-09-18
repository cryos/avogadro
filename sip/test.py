from PyQt4.Qt import *
from AvoSIP import *
import sys
import pybel
import openbabel

# create a new application
# (must be done before creating a GLWidget)
app = QApplication(sys.argv)

# create the GLWidget and load the default engines
glwidget = Avogadro.GLWidget()
glwidget.loadDefaultEngines()
glwidget.resize(640, 480)
glwidget.show()

# set the undo stack
undostack = QUndoStack()
glwidget.setUndoStack(undostack)

pluginManager = Avogadro.PluginManager();

# load the tools
toolgroup = Avogadro.ToolGroup()
glwidget.setToolGroup(toolgroup)

# print the names of the found tools
for tool in pluginManager.tools():
  if tool.name() == "Navigate":
    toolgroup.setActiveTool(tool)

# show the settings widget for the active tool (drawtool)
if (toolgroup.activeTool().settingsWidget()):
  toolgroup.activeTool().settingsWidget().show();

# create the molecule and read a molecule from file
mol = Avogadro.Molecule()
readFile("test.sdf", mol)
glwidget.setMolecule(mol);


sys.exit(app.exec_())
