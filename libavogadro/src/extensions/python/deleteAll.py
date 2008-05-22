import sys
import Avogadro

def name():
    return "Delete All Atoms"

def description():
    return "Delete all the Atoms"

def extension():
    sys.stdout.write("Deleting all atoms")
    for x in range(Avogadro.molecule.NumAtoms()):
        Avogadro.molecule.DeleteAtom(Avogadro.molecule.GetAtom(1), 1)
        sys.stdout.write(".")
    sys.stdout.write("\n")

