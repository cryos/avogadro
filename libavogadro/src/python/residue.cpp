// Last update: timvdm 19 June 2009
#include <boost/python.hpp>

#include <avogadro/primitive.h>
#include <avogadro/fragment.h>
#include <avogadro/residue.h>
#include <avogadro/molecule.h>

using namespace boost::python;
using namespace Avogadro;

void export_Residue()
{

  class_<Avogadro::Residue, bases<Avogadro::Fragment>, boost::noncopyable>("Residue", no_init)
    //
    // read/write properties
    //
    .add_property("number", 
        &Residue::number, 
        &Residue::setNumber,
        "The \"number\" of the residue, e.g. 5A, 69, etc.")

    .add_property("chainNumber", 
        &Residue::chainNumber, 
        &Residue::setChainNumber,
        "The chain number that the residue belongs to.")

    .add_property("chainID", 
        &Residue::chainID, 
        &Residue::setChainID,
        "The chain ID (' ', 'A', 'B', ...) that the residue belongs to.")

    .add_property("atomIds", 
        make_function(&Residue::atomIds, return_value_policy<return_by_value>()), 
        &Residue::setAtomIds,
        "List of all atom text ids in the Residue.")

    //
    // real functions 
    //
    .def("addAtom", 
        &Residue::addAtom, 
        "Add an Atom to the Residue.")

    .def("removeAtom", 
        &Residue::addAtom, 
        "Remove an Atom to the Residue.")

    .def("atomId", 
        &Residue::atomId,
        "Returns the atom text id, as in the Residue.")

    .def("setAtomId", 
        &Residue::setAtomId,
        "Set the text id of the Atom.")

    // returns bool...
    .def("setAtomIds", 
        &Residue::setAtomIds,
        "Set the text id of all the Atom objects.")
    ;

}
