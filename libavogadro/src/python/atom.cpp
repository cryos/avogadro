#include <Python.h>
#include <boost/python.hpp>

#include <avogadro/primitive.h>
#include <avogadro/atom.h>
#include <avogadro/molecule.h>

using namespace boost::python;
using namespace Avogadro;

void export_Atom()
{
  // define function pointers to handle overloading
  void (Atom::*setPos_ptr)(const Eigen::Vector3d&) = &Atom::setPos;
  void (Atom::*addBond_ptr)(unsigned long int) = &Atom::addBond;
  void (Atom::*deleteBond_ptr)(unsigned long int) = &Atom::deleteBond;

  class_<Avogadro::Atom, bases<Avogadro::Primitive>, boost::noncopyable>("Atom", no_init)
    .def("pos", &Atom::pos, return_value_policy<return_by_value>())
    .def("setPos", setPos_ptr)
    
    //.def("atomicNumber", &Atom::atomicNumber)
    //.def("setAtomicNumber", &Atom::setAtomicNumber)
    .add_property("atomicNumber", &Atom::atomicNumber, &Atom::setAtomicNumber)
    
    .def("addBond", addBond_ptr)
    .def("deleteBond", deleteBond_ptr)
    .def("bonds", &Atom::bonds)
    
    .def("neighbors", &Atom::neighbors)
    .def("valence", &Atom::valence)
    .def("isHydrogen", &Atom::isHydrogen)
    
    //.def("partialCharge", &Atom::partialCharge)
    //.def("setPartialCharge", &Atom::setPartialCharge)
    .add_property("partialCharge", &Atom::partialCharge, &Atom::setPartialCharge)
    ;

}
