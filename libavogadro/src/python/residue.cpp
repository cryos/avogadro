#include <Python.h>
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
    //.def("number", &Residue::number)
    //.def("setNumber", &Residue::setNumber)
    .def("chainNumber", &Residue::chainNumber)
    .def("setChainNumber", &Residue::setChainNumber)
    //.def("atomId", &Residue::atomId)
    //.def("atomIds", &Residue::atomIds)
    //.def("setAtomId", &Residue::setAtomId)
    //.def("setAtomIds", &Residue::setAtomIds)
    ;

}
