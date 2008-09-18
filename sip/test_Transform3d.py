from PyQt4.Qt import *
from Avogadro import *
from Numeric import *
import sys

# create a new application
# (must be done before creating a GLWidget)
app = QApplication(sys.argv)

# create the GLWidget and load the default engines
glwidget = Avogadro.GLWidget()

modelView = glwidget.camera().modelview();
print modelView

modelView = array(((1., 2., 3., 0.), (4., 5., 6., 0.), (7., 8., 9., 0.), (1., 2., 3., 4.)))
glwidget.camera().setModelview(modelView);
if modelView == glwidget.camera().modelview():
  print "passed"
else:
  print "failed to convert array -> Eigen::Transform3d"


glwidget.show()
sys.exit(app.exec_())
