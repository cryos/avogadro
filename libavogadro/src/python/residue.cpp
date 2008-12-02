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
    // read/write properties
    .add_property("number", &Residue::number, &Residue::setNumber)
    .add_property("chainNumber", &Residue::chainNumber, &Residue::setChainNumber)
    .add_property("atomIds", make_function(&Residue::atomIds, return_value_policy<return_by_value>()), &Residue::setAtomIds)
    // real functions 
    .def("atomId", &Residue::atomId)
    .def("setAtomId", &Residue::setAtomId)
    ;

}
