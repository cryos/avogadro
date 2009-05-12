// Last update: timvdm 12 May 2009

//#include <Python.h>
// http://www.boost.org/doc/libs/1_39_0/libs/python/doc/building.html#include-issues
#include <boost/python/detail/wrap_python.hpp>
#include <boost/python.hpp>

#include <avogadro/atom.h>
#include <avogadro/bond.h>
#include <avogadro/molecule.h>

using namespace boost::python;
using namespace Avogadro;

void export_Bond()
{

  class_<Avogadro::Bond, bases<Avogadro::Primitive>, boost::noncopyable>("Bond", no_init)
    // read/write properties
    .add_property("order", &Bond::order, &Bond::setOrder)
    .add_property("isAromatic", &Bond::isAromatic, &Bond::setAromaticity)
    // read-only poperties
    .add_property("beginAtom", make_function(&Bond::beginAtom, return_value_policy<reference_existing_object>()))
    .add_property("beginAtomId", &Bond::beginAtomId)
    .add_property("endAtom", make_function(&Bond::endAtom, return_value_policy<reference_existing_object>()))
    .add_property("endAtomId", &Bond::endAtomId)
    .add_property("length", &Bond::length)
    .add_property("beginPos", make_function(&Bond::beginPos, return_value_policy<return_by_value>()))
    .add_property("endPos", make_function(&Bond::endPos, return_value_policy<return_by_value>()))
    .add_property("midPos", make_function(&Bond::midPos, return_value_policy<return_by_value>()))
    // real functions
    .def("otherAtom", &Bond::otherAtom)
    .def("setBegin", &Bond::setBegin)
    .def("setEnd", &Bond::setEnd)
    .def("setAtoms", &Bond::setAtoms)
    ;

}
