#include <Python.h>
#include <boost/python.hpp>

#include <avogadro/primitive.h>
#include <avogadro/molecule.h>
#include <avogadro/atom.h>
#include <avogadro/bond.h>
#include <avogadro/residue.h>
#include <avogadro/cube.h>

using namespace boost::python;
using namespace Avogadro;

void export_Molecule()
{
 
  // define function pointers to handle overloading
  Atom* (Molecule::*newAtom_ptr1)() = &Molecule::newAtom;
  Atom* (Molecule::*newAtom_ptr2)(unsigned long int) = &Molecule::newAtom;
  void (Molecule::*deleteAtom_ptr1)(Atom*) = &Molecule::deleteAtom;
  void (Molecule::*deleteAtom_ptr2)(unsigned long int) = &Molecule::deleteAtom;
  Bond* (Molecule::*newBond_ptr1)() = &Molecule::newBond;
  Bond* (Molecule::*newBond_ptr2)(unsigned long int) = &Molecule::newBond;
  void (Molecule::*deleteBond_ptr1)(Bond*) = &Molecule::deleteBond;
  void (Molecule::*deleteBond_ptr2)(unsigned long int) = &Molecule::deleteBond;
  void (Molecule::*deleteCube_ptr1)(Cube*) = &Molecule::deleteCube;
  void (Molecule::*deleteCube_ptr2)(unsigned long int) = &Molecule::deleteCube;
  void (Molecule::*deleteResidue_ptr1)(Residue*) = &Molecule::deleteResidue;
  void (Molecule::*deleteResidue_ptr2)(unsigned long int) = &Molecule::deleteResidue;
  void (Molecule::*deleteRing_ptr1)(Fragment*) = &Molecule::deleteRing;
  void (Molecule::*deleteRing_ptr2)(unsigned long int) = &Molecule::deleteRing;
  Atom* (Molecule::*atom_ptr)(int) = &Molecule::atom;
  Bond* (Molecule::*bond_ptr1)(int) = &Molecule::bond;
  Bond* (Molecule::*bond_ptr2)(unsigned long, unsigned long) = &Molecule::bond;
  Bond* (Molecule::*bond_ptr3)(const Atom*, const Atom*) = &Molecule::bond;

  class_<Avogadro::Molecule, bases<Avogadro::Primitive>, boost::noncopyable>("Molecule")
    .def("setFileName", &Molecule::setFileName)
    .def("fileName", &Molecule::fileName)
    .def("newAtom", newAtom_ptr1, return_value_policy<reference_existing_object>())
    .def("newAtom", newAtom_ptr2, return_value_policy<reference_existing_object>())
    .def("setAtomPos", &Molecule::setAtomPos)
    .def("atomPos", &Molecule::atomPos, return_value_policy<return_by_value>())
    .def("deleteAtom", deleteAtom_ptr1)
    .def("deleteAtom", deleteAtom_ptr2)
    .def("newBond", newBond_ptr1, return_value_policy<reference_existing_object>())
    .def("newBond", newBond_ptr2, return_value_policy<reference_existing_object>())
    .def("deleteBond", deleteBond_ptr1)
    .def("deleteBond", deleteBond_ptr2)
    .def("addHydrogens", &Molecule::addHydrogens)
    .def("deleteHydrogens", &Molecule::deleteHydrogens)
    .def("calculatePartialCharges", &Molecule::calculatePartialCharges)
    .def("newCube", &Molecule::newCube, return_value_policy<reference_existing_object>())
    .def("deleteCube", deleteCube_ptr1)
    .def("deleteCube", deleteCube_ptr2)
    .def("newResidue", &Molecule::newResidue, return_value_policy<reference_existing_object>())
    .def("deleteResidue", deleteResidue_ptr1)
    .def("deleteResidue", deleteResidue_ptr2)
    .def("newRing", &Molecule::newRing, return_value_policy<reference_existing_object>())
    .def("deleteRing", deleteRing_ptr1)
    .def("deleteRing", deleteRing_ptr2)
    .def("numAtoms", &Molecule::numAtoms)
    .def("numBonds", &Molecule::numBonds)
    .def("numCubes", &Molecule::numCubes)
    .def("numResidues", &Molecule::numResidues)
    .def("atom", atom_ptr, return_value_policy<reference_existing_object>())
    .def("atomById", &Molecule::atomById, return_value_policy<reference_existing_object>())
    .def("bond", bond_ptr1, return_value_policy<reference_existing_object>())
    .def("bond", bond_ptr2, return_value_policy<reference_existing_object>())
    .def("bond", bond_ptr3, return_value_policy<reference_existing_object>())
    .def("bondById", &Molecule::bondById, return_value_policy<reference_existing_object>())
    .def("residue", &Molecule::residue, return_value_policy<reference_existing_object>())
    .def("residueById", &Molecule::residueById, return_value_policy<reference_existing_object>())
    .def("atoms", &Molecule::atoms)
    .def("bonds", &Molecule::bonds)
    .def("cubes", &Molecule::cubes)
    .def("residues", &Molecule::residues)
    .def("rings", &Molecule::rings)
    .def("clear", &Molecule::clear)
    .def("center", &Molecule::center, return_value_policy<return_by_value>())
    .def("normalVector", &Molecule::normalVector, return_value_policy<return_by_value>())
    .def("radius", &Molecule::radius)
    .def("farthestAtom", &Molecule::farthestAtom, return_value_policy<reference_existing_object>())
    .def("translate", &Molecule::translate)
    ;

} 
