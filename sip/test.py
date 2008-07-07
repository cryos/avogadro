from PyQt4.Qt import *
from AvoSIP import *
import sys

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

# load the tools
toolgroup = Avogadro.ToolGroup()
toolgroup.load()
glwidget.setToolGroup(toolgroup)

# print the names of the found tools
for tool in toolgroup.tools():
  print tool.name()
  if tool.name() == "Navigate":
    toolgroup.setActiveTool(tool)


# show the settings widget for the active tool (drawtool)
if (toolgroup.activeTool().settingsWidget()):
  toolgroup.activeTool().settingsWidget().show();

# create the molecule and add a nitrogen atom
mol = Avogadro.Molecule()
glwidget.setMolecule(mol);
atom = mol.newAtom()
atom.SetAtomicNum(7)


sys.exit(app.exec_())
