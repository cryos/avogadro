// Last update: timvdm 18 June 2009
#include <boost/python.hpp>

#include <avogadro/primitive.h>
#include <avogadro/atom.h>
#include <avogadro/bond.h>
#include <avogadro/residue.h>
#include <avogadro/molecule.h>

using namespace boost::python;
using namespace Avogadro;

void export_Atom()
{
  // define function pointers to handle overloading
  void (Atom::*setPos_ptr)(const Eigen::Vector3d&) = &Atom::setPos;

  class_<Avogadro::Atom, bases<Avogadro::Primitive>, boost::noncopyable>("Atom", 
      "Atom class derived from Primitive", no_init)

    // 
    // read/write properties
    //
    .add_property("pos", 
        make_function(&Atom::pos, return_value_policy<return_by_value>()), 
        setPos_ptr,
        "The position iof the atom.")

    .add_property("atomicNumber", 
        &Atom::atomicNumber, 
        &Atom::setAtomicNumber,
        "The atomic number of the atom.")

    .add_property("partialCharge", 
        &Atom::partialCharge, 
        &Atom::setPartialCharge,
        "The partial charge of the atom. This is not calculated by the atom, instead call "
        "Molecule::calculatePartialCharges().")

    .add_property("forceVector", 
        &Atom::forceVector, 
        &Atom::setForceVector,
        "The force vector on the atom (e.g., used to display vibrations).")

    //
    // read-only properties
    //
    .add_property("residue", 
        make_function(&Atom::residue, return_value_policy<reference_existing_object>()),
        "The Residue that the Atom is a part of.")

    .add_property("residueId", 
        &Atom::residueId, 
        "The Id of the Residue that the Atom is a part of.")
    
    .add_property("bonds", 
        &Atom::bonds, 
        "List of bond ids to the atom.")
    
    .add_property("neighbors", 
        &Atom::neighbors, 
        "List of neighbor ids to the atom (atoms bonded to that atom).")
    
    .add_property("valence", 
        &Atom::valence, 
        "The valence of the atom.")
    
    .add_property("isHydrogen", 
        &Atom::isHydrogen, 
        "True if the atom is a hydrogen.")

    //
    // real functions
    //
    .def("bond", 
        make_function(&Atom::bond, return_value_policy<reference_existing_object>()), 
        "Get the bond between this Atom and another.")
    ;

}
