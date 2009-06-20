// Last update: timvdm 18 June 2009
#include <boost/python.hpp>

#include <avogadro/atom.h>
#include <avogadro/bond.h>
#include <avogadro/molecule.h>

using namespace boost::python;
using namespace Avogadro;

void export_Bond()
{

  class_<Avogadro::Bond, bases<Avogadro::Primitive>, boost::noncopyable>("Bond", 
      "Bond class derived from Primitive", no_init)
    
    // 
    // read/write properties
    //
    .add_property("order", 
        &Bond::order, 
        &Bond::setOrder, 
        "The order of the bond.")

    .add_property("isAromatic", 
        &Bond::isAromatic, 
        &Bond::setAromaticity, 
        "The aromaticity of the bond.")
    
    //
    // read-only poperties
    //
    .add_property("beginAtom", 
        make_function(&Bond::beginAtom, return_value_policy<reference_existing_object>()),
        "The first atom in the bond.")

    .add_property("beginAtomId", 
        &Bond::beginAtomId, 
        "The unique ID of the first atom in the bond.")

    .add_property("endAtom", 
        make_function(&Bond::endAtom, return_value_policy<reference_existing_object>()),
        "The second atom in the bond.")

    .add_property("endAtomId", 
        &Bond::endAtomId, 
        "The unique ID of the second atom in the bond.")

    .add_property("length", 
        &Bond::length, 
        "The length of the bond.")

    .add_property("beginPos", 
        make_function(&Bond::beginPos, return_value_policy<return_by_value>()),
        "The position of the begin of the Bond.")

    .add_property("endPos", 
        make_function(&Bond::endPos, return_value_policy<return_by_value>()),
        "The position of the end of the Bond.")

    .add_property("midPos", 
        make_function(&Bond::midPos, return_value_policy<return_by_value>()),
        "The position of the mid-point of the Bond.")

    //
    // real functions
    //
    .def("otherAtom", 
        &Bond::otherAtom, 
        "Get the unique ID of the other atom in the bond.")

    .def("setBegin", 
        &Bond::setBegin, 
        "Set the unique ID of the first atom in the bond.")

    .def("setEnd", 
        &Bond::setEnd, 
        "Set the unique ID of the second atom in the bond.")

    .def("setAtoms",
       &Bond::setAtoms, 
       "Set the unique ID of both atoms in the bond.")
    ;

}
