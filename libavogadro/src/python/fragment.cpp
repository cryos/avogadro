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
    .def("name", &Fragment::name)
    .def("setName", &Fragment::setName)
    .def("addAtom", &Fragment::addAtom)
    .def("removeAtom", &Fragment::removeAtom)
    //.def("atoms", &Fragment::atoms)
    .def("addBond", &Fragment::addBond)
    .def("removeBond", &Fragment::removeBond)
    //.def("bonds", &Fragment::bonds)
    ;

}
