from Avogadro import *
from Numeric import *

mol = Molecule()

atom = mol.newAtom()
mol.newBond()
mol.newBond()

print atom.pos
print "atom.pos = array([1., 2., 3.])"
atom.pos = array([1., 2., 3.])
print atom.pos

atom.atomicNumber = 6
print "atom.atomicNumber =", atom.atomicNumber

print "Adding bonds to atom"
for bond in mol.bonds:
  atom.addBond(bond)

print "valence =", atom.valence
print "atom.bonds =", atom.bonds
