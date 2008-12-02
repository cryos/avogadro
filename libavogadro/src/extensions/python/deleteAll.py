import sys
import Avogadro

def name():
    return "Delete All Atoms"

def description():
    return "Delete all the Atoms"

def extension():
    sys.stdout.write("Deleting all atoms")
    for atom in Avogadro.molecule.atoms:
      Avogadro.molecule.deleteAtom(atom)
      sys.stdout.write(".")
    sys.stdout.write("\n")

