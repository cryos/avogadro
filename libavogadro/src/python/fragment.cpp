#include <Python.h>
#include <boost/python.hpp>

#include <avogadro/primitive.h>
#include <avogadro/fragment.h>
#include <avogadro/molecule.h>

using namespace boost::python;
using namespace Avogadro;

void export_Fragment()
{ 
 
  class_<Avogadro::Fragment, bases<Avogadro::Primitive>, boost::noncopyable>("Fragment", no_init)
    // read/write properties
    .add_property("name", &Fragment::name, &Fragment::setName)
    // read-only properties
    .def("atoms", &Fragment::atoms)
    .def("bonds", &Fragment::bonds)
    // real functions
    .def("addAtom", &Fragment::addAtom)
    .def("removeAtom", &Fragment::removeAtom)
    .def("addBond", &Fragment::addBond)
    .def("removeBond", &Fragment::removeBond)
    ;

}
