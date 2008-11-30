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
    .def("setBegin", &Bond::setBegin)
    .def("beginAtomId", &Bond::beginAtomId)
    .def("setEnd", &Bond::setEnd)
    .def("endAtomId", &Bond::endAtomId)
    .def("setAtoms", &Bond::setAtoms)
    .def("otherAtom", &Bond::otherAtom)
    .def("order", &Bond::order)
    .def("setOrder", &Bond::setOrder)
    .def("length", &Bond::length)
    ;
   
}
