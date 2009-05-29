import openbabel
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from PyQt4 import uic
import Avogadro
import sys


class MainWindow(QMainWindow):
  def __init__(self):
    QMainWindow.__init__(self)

    uiFile = file('mainwindow.ui', 'r')
    self.ui = uic.loadUi(uiFile, self)
    
    self.connect(self.ui.actionOpen, SIGNAL('triggered()'), self, SLOT('open()'))
    self.connect(self.ui.listWidget, SIGNAL('currentRowChanged(int)'), self, SLOT('load(int)'))

    self.objects = []

  def newGLWidget(self):
    glWidget = Avogadro.GLWidget()
    glWidget.loadDefaultEngines()
    glWidget.quality = 4
    toolGroup = Avogadro.ToolGroup()
    tool = Avogadro.PluginManager.instance.tool('Navigate', None)
    toolGroup.append(tool)
    glWidget.toolGroup = toolGroup
    self.objects.append(glWidget)
    self.objects.append(toolGroup)
    self.objects.append(tool)
    return glWidget

  @pyqtSignature('load(int)')
  def load(self, row):
    mol = self.molFile.molecule(row)
    self.objects.append(mol)
    glWidget = self.newGLWidget()
    glWidget.molecule = mol
    mdiWindow = self.ui.mdiArea.addSubWindow(Avogadro.toPyQt(glWidget))
    mdiWindow.setWindowTitle(self.molFile.titles[row])
    mdiWindow.show()

  @pyqtSignature('readThreadFinnished()')
  def readThreadFinnished(self):
    self.ui.listWidget.clear()
    self.ui.listWidget.addItems(self.molFile.titles)
    self.ui.statusbar.showMessage('Done.', 5)

  @pyqtSignature('open()')
  def open(self):
    filename = str(QFileDialog.getOpenFileName(self, 'Open', QDir.currentPath(), 
          'Chemistry files (*.cml *.sdf *.mol *.xyz)'))
    self.molFile = Avogadro.OpenbabelWrapper.readFile(filename, '', '', False)
    self.connect(Avogadro.toPyQt(self.molFile), SIGNAL('ready()'), self, SLOT('readThreadFinnished()'))
    self.ui.statusbar.showMessage('Reading file...')
   

if __name__ == "__main__":
  app = QApplication(sys.argv)

  window = MainWindow()
  window.show()

  sys.exit(app.exec_())
