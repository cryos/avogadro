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
  Atom* (Molecule::*addAtom_ptr1)() = &Molecule::addAtom;
  Atom* (Molecule::*addAtom_ptr2)(unsigned long int) = &Molecule::addAtom;
  void (Molecule::*removeAtom_ptr1)(Atom*) = &Molecule::removeAtom;
  void (Molecule::*removeAtom_ptr2)(unsigned long int) = &Molecule::removeAtom;
  void (Molecule::*setAtomPos_ptr1)(unsigned long int, const Eigen::Vector3d &) = &Molecule::setAtomPos;
  Bond* (Molecule::*addBond_ptr1)() = &Molecule::addBond;
  Bond* (Molecule::*addBond_ptr2)(unsigned long int) = &Molecule::addBond;
  void (Molecule::*removeBond_ptr1)(Bond*) = &Molecule::removeBond;
  void (Molecule::*removeBond_ptr2)(unsigned long int) = &Molecule::removeBond;
  Cube* (Molecule::*addCube_ptr1)() = &Molecule::addCube;
  void (Molecule::*removeCube_ptr1)(Cube*) = &Molecule::removeCube;
  void (Molecule::*removeCube_ptr2)(unsigned long int) = &Molecule::removeCube;
  Residue* (Molecule::*addResidue_ptr1)() = &Molecule::addResidue;
  void (Molecule::*removeResidue_ptr1)(Residue*) = &Molecule::removeResidue;
  void (Molecule::*removeResidue_ptr2)(unsigned long int) = &Molecule::removeResidue;
  Fragment* (Molecule::*addRing_ptr1)() = &Molecule::addRing;
  void (Molecule::*removeRing_ptr1)(Fragment*) = &Molecule::removeRing;
  void (Molecule::*removeRing_ptr2)(unsigned long int) = &Molecule::removeRing;
  Atom* (Molecule::*atom_ptr)(int) = &Molecule::atom;
  Bond* (Molecule::*bond_ptr1)(int) = &Molecule::bond;
  Bond* (Molecule::*bond_ptr2)(unsigned long, unsigned long) = &Molecule::bond;
  Bond* (Molecule::*bond_ptr3)(const Atom*, const Atom*) = &Molecule::bond;
  Residue* (Molecule::*residue_ptr)(int) = &Molecule::residue;

  class_<Avogadro::Molecule, bases<Avogadro::Primitive>, boost::noncopyable>("Molecule")
    // constructor
    .def(init<const Molecule&>())

    // read/write properties
    .add_property("fileName", &Molecule::fileName, &Molecule::setFileName)
    // read-only poperties
    .add_property("numAtoms", &Molecule::numAtoms)
    .add_property("numBonds", &Molecule::numBonds)
    .add_property("numCubes", &Molecule::numCubes)
    .add_property("numResidues", &Molecule::numResidues)
    .add_property("numRings", &Molecule::numRings)
    .add_property("atoms", &Molecule::atoms)
    .add_property("bonds", &Molecule::bonds)
    .add_property("cubes", &Molecule::cubes)
    .add_property("residues", &Molecule::residues)
    .add_property("rings", &Molecule::rings)
    .add_property("center", make_function(&Molecule::center, return_value_policy<return_by_value>()))
    .add_property("normalVector", make_function(&Molecule::normalVector, return_value_policy<return_by_value>()))
    .add_property("radius", &Molecule::radius)
    .add_property("farthestAtom", make_function(&Molecule::farthestAtom, return_value_policy<reference_existing_object>()))

    //
    // real functions
    //

    .def("setAtomPos", setAtomPos_ptr1)
    .def("atomPos", &Molecule::atomPos, return_value_policy<return_by_value>())

    // atom functions
    .def("addAtom", addAtom_ptr1, return_value_policy<reference_existing_object>())
    .def("addAtom", addAtom_ptr2, return_value_policy<reference_existing_object>())
    .def("atom", atom_ptr, return_value_policy<reference_existing_object>())
    .def("atomById", &Molecule::atomById, return_value_policy<reference_existing_object>())
    .def("removeAtom", removeAtom_ptr1)
    .def("removeAtom", removeAtom_ptr2)
    // bond functions
    .def("addBond", addBond_ptr1, return_value_policy<reference_existing_object>())
    .def("addBond", addBond_ptr2, return_value_policy<reference_existing_object>())
    .def("bond", bond_ptr1, return_value_policy<reference_existing_object>())
    .def("bond", bond_ptr2, return_value_policy<reference_existing_object>())
    .def("bond", bond_ptr3, return_value_policy<reference_existing_object>())
    .def("bondById", &Molecule::bondById, return_value_policy<reference_existing_object>())
    .def("removeBond", removeBond_ptr1)
    .def("removeBond", removeBond_ptr2)
    // cube functions
    .def("addCube", addCube_ptr1, return_value_policy<reference_existing_object>())
    .def("removeCube", removeCube_ptr1)
    .def("removeCube", removeCube_ptr2)
    // residue functions
    .def("addResidue", addResidue_ptr1, return_value_policy<reference_existing_object>())
    .def("residue", residue_ptr, return_value_policy<reference_existing_object>())
    .def("residueById", &Molecule::residueById, return_value_policy<reference_existing_object>())
    .def("removeResidue", removeResidue_ptr1)
    .def("removeResidue", removeResidue_ptr2)
    // ring functions
    .def("addRing", addRing_ptr1, return_value_policy<reference_existing_object>())
    .def("removeRing", removeRing_ptr1)
    .def("removeRing", removeRing_ptr2)
    // general functions
    .def("addHydrogens", &Molecule::addHydrogens)
    .def("removeHydrogens", &Molecule::removeHydrogens)
    .def("calculatePartialCharges", &Molecule::calculatePartialCharges)
    .def("clear", &Molecule::clear)
    .def("translate", &Molecule::translate)
    ;

}
