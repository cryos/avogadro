from Numeric import *
import Avogadro

# test ConvertFromTypeCode for Eigen::Vector3d
atom = Avogadro.Avogadro.Atom()
if atom.pos() == array([0.0, 0.0, 0.0]):
  print "passed"
else:
  print "failed to convert Eigen::Vector3d -> array"

# test ConvertToTypeCode for Eigen::Vector3d
pos = array([1.0, 2.0, 3.0])
atom.setPos(pos)
if atom.pos() == pos:
  print "passed"
else:
  print "failed to convert array -> Eigen::Vector3d"

# test if code can correctly detect wrong size arrays
pos = array([1.0, 2.0, 3.0, 4.0])
try:
  atom.setPos(pos)
  print "failed to detect array -> Eigen::Vector3d conversion with 4 element"
except:
  print "passed"
 
pos = array([1.0])
try:
  atom.setPos(pos)
  print "failed to detect array -> Eigen::Vector3d conversion with only 1 element"
except:
  print "passed"
