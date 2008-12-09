import Avogadro
from numpy import *

class Engine():
  def name(self):
    return "Cube Point Engine"

  def renderOpaque(self, pd):
    # Painter 
    painter = pd.painter
    # Molecule
    molecule = pd.molecule

    # Color
    color = pd.colorMap

    # set color to white
    painter.setColor(1.0, 1.0, 1.0, 1.0)
    # draw some text
    painter.drawText(10,10, "Cube engine")

    # set color to green
    for cube in molecule.cubes:
      # array with 3 doubles
      min = cube.min
      max = cube.max
      
      x_range = max[0] - min[0]
      y_range = max[1] - min[1]
      z_range = max[2] - min[2]

      dim = 10.0
      for x in range(dim):
        for y in range(dim):
          for z in range(dim):
            pos = min + array([ x / dim * x_range, y / dim * y_range, z / dim * z_range ])
            if cube.value(pos) < 0:
              painter.setColor(1.0, 0.0, 0.0, 1.0)
            else:
              painter.setColor(0.0, 0.0, 1.0, 1.0)
            painter.drawSphere(pos, 0.2)

