// Last update: timvdm 18 June 2009
#include <boost/python.hpp>

#include <avogadro/primitive.h>
#include <avogadro/fragment.h>
#include <avogadro/molecule.h>

using namespace boost::python;
using namespace Avogadro;

void export_Fragment()
{ 
 
  class_<Avogadro::Fragment, bases<Avogadro::Primitive>, boost::noncopyable>("Fragment", no_init)
    //
    // read/write properties
    //
    .add_property("name", 
        &Fragment::name, 
        &Fragment::setName, 
        "The name of the fragment.")

    //
    // read-only properties
    //
    .add_property("atoms", 
        &Fragment::atoms, 
        "List of the unique ids of the atoms in this Fragment.")

    .add_property("bonds", 
        &Fragment::bonds, 
        "List of the unique ids of the bonds in this Fragment.")

    // 
    // real functions
    //
    .def("addAtom", 
        &Fragment::addAtom, 
        "Add an Atom to the Fragment.")

    .def("removeAtom", 
        &Fragment::removeAtom, 
        "Remove the Atom from the Fragment.")

    .def("addBond", 
        &Fragment::addBond, 
        "Add a Bond to the Fragment.")

    .def("removeBond", 
        &Fragment::removeBond, 
        "Remove the Bond from the Fragment.")
    ;

}
