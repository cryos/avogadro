#include <Python.h>
#include <boost/python.hpp>

#include <avogadro/primitive.h>
#include <avogadro/bond.h>
#include <avogadro/molecule.h>

using namespace boost::python;
using namespace Avogadro;

void export_Bond()
{
      
  class_<Avogadro::Bond, bases<Avogadro::Primitive>, boost::noncopyable>("Bond", no_init)
    // read/write properties
    .add_property("order", &Bond::order, &Bond::setOrder)
    // read-only poperties 
    .add_property("beginAtomId", &Bond::beginAtomId)
    .add_property("endAtomId", &Bond::endAtomId)
    //.add_property("otherAtom", &Bond::otherAtom)
    .add_property("otherAtom", &Bond::otherAtom)
    .add_property("length", &Bond::length)
    // real functions
    .def("setBegin", &Bond::setBegin)
    .def("setEnd", &Bond::setEnd)
    .def("setAtoms", &Bond::setAtoms)
    ;
   
}
