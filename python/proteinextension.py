from PyQt4.QtCore import *
from PyQt4.QtGui import *
from numpy import *
import Avogadro

# always use 'Extension' for class name
class Extension(QObject):
  def __init__(self):
    QObject.__init__(self)

  def name(self):
    return "Protein Extension"

  def description(self):
    return "Extension providing some usefull protein selections."

  def actions(self):
    actions = []

    action = QAction(self)
    action.setText("Select Backbone")
    actions.append(action)

    action = QAction(self)
    action.setText("Select Side Chains")
    actions.append(action)

    action = QAction(self)
    action.setText("Select Binding Site")
    actions.append(action)

    action = QAction(self)
    action.setText("Cut Surface")
    actions.append(action)


    return actions

  def menuPath(self, action):
    return "Protein"

  def selectBackbone(self, glwidget):
    mol = glwidget.molecule
    # create a PrimitiveList to call GLWidget.setSelected(PrimitiveList, bool)
    primitiveList = Avogadro.PrimitiveList()
    # find the backbone atoms
    for atom in mol.atoms:
      # get the atomId and strip of white spaces
      atomId = atom.residue.atomId(atom.id).strip()
      print atomId
      # add the atom to the list if the atomId matches N, CA, C, O or H
      if atomId == "N":
        primitiveList.append(atom)
      elif atomId == "CA":
        primitiveList.append(atom)
      elif atomId == "C":
        primitiveList.append(atom)
      elif atomId == "O":
        primitiveList.append(atom)
      elif atomId == "H":
        primitiveList.append(atom)

    # find all bonds between backbone atoms
    for bond in mol.bonds:
      beginAtom = bond.beginAtom
      endAtom = bond.endAtom
      if primitiveList.contains(beginAtom) and primitiveList.contains(endAtom):
        primitiveList.append(bond)

    # clear the current selection
    glwidget.clearSelected()
    # select the backbone atom
    glwidget.setSelected(primitiveList, True)
    #Avogadro.toPyQt(glwidget).update()

  def selectSideChains(self, glwidget):
    mol = glwidget.molecule
    # create a PrimitiveList to call GLWidget.setSelected(PrimitiveList, bool)
    primitiveList = Avogadro.PrimitiveList()
    # find the backbone atoms
    for atom in mol.atoms:
      # get the atomId and strip of white spaces
      atomId = atom.residue.atomId(atom.id).strip()
      # add the atom to the list if the atomId matches N, CA, C, O or H
      if atomId != "N" and atomId != "CA" and atomId != "C" and atomId != "O" and atomId != "H":
        primitiveList.append(atom)

    # find all bonds between backbone atoms
    for bond in mol.bonds:
      beginAtom = bond.beginAtom
      endAtom = bond.endAtom
      if primitiveList.contains(beginAtom) and primitiveList.contains(endAtom):
        primitiveList.append(bond)

    # clear the current selection
    glwidget.clearSelected()
    # select the side chain atom
    glwidget.setSelected(primitiveList, True)

  def selectBindingSite(self, glwidget):
    # returns (double, bool ok) as tuple
    result = QInputDialog.getDouble(None, "Create Binding Site Around Selection", "radius", 5.0, 2.0, 20.0, 1)
    if not result[1]:
      return

    r = result[0]
    r2 = r * r
    selectedAtoms = glwidget.selectedPrimitives.subList(Avogadro.PrimitiveType.AtomType)

    newSelection = Avogadro.PrimitiveList()
    atomIds = []
    for residue in glwidget.molecule.residues:
      keepResidue = False
      # compute distance between residue atoms and selected atoms
      for atomId in residue.atoms:
        atom = glwidget.molecule.atomById(atomId)
        if not atom:
          continue
        
        for selectedAtom in selectedAtoms:
          ab = selectedAtom.pos - atom.pos
          dist2 = dot(ab,ab)
          if dist2 < r2:
            keepResidue = True
            break

        if keepResidue:
          break

      if keepResidue:
        for atomId in residue.atoms:
          atom = glwidget.molecule.atomById(atomId)
          newSelection.append(atom)
          atomIds.append(atom.id)

    # find all bonds between the atoms
    for bond in glwidget.molecule.bonds:
      beginAtom = bond.beginAtom
      endAtom = bond.endAtom
      if atomIds.count(beginAtom.id) and atomIds.count(endAtom.id):
        newSelection.append(bond)

    glwidget.setSelected(newSelection, True)

  def cutSurface(self, glwidget):
    # returns (double, bool ok) as tuple
    result = QInputDialog.getDouble(None, "Cut Surface", "radius", 5.0, 2.0, 20.0, 1)
    if not result[1]:
      return

    r = result[0]
    r2 = r * r
    selectedAtoms = glwidget.selectedPrimitives.subList(Avogadro.PrimitiveType.AtomType)
    
    newVertices = []
    newNormals = []
    for mesh in glwidget.molecule.meshes:
      verts = mesh.vertices
      normals = mesh.normals
      i = 0
      while i < len(verts):
        center = verts[i] + verts[i+1] + verts[i+2]
        center /= 3

        for selectedAtom in selectedAtoms:
          ab = selectedAtom.pos - verts[i]
          ab2 = dot(ab,ab)
          ac = selectedAtom.pos - verts[i+1]
          ac2 = dot(ac,ac)
          ad = selectedAtom.pos - verts[i+2]
          ad2 = dot(ad,ad)
          if ab2 < r2 or ac2 < r2 or ad2 < r2:
            newVertices.append(verts[i])
            newVertices.append(verts[i+1])
            newVertices.append(verts[i+2])

            newNormals.append(normals[i])
            newNormals.append(normals[i+1])
            newNormals.append(normals[i+2])
            break
        
        i += 3
    
    mesh.vertices = newVertices
    mesh.normals = newNormals


  def performAction(self, action, glwidget):
    if action.text() == "Select Backbone":
      self.selectBackbone(glwidget)
    elif action.text() == "Select Side Chains":
      self.selectSideChains(glwidget)
    elif action.text() == "Select Binding Site":
      self.selectBindingSite(glwidget)
    elif action.text() == "Cut Surface":
      self.cutSurface(glwidget)

