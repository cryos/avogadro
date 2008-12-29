#include <Python.h>
#include <boost/python.hpp>

#include <avogadro/primitive.h>
#include <avogadro/atom.h>
#include <avogadro/bond.h>
#include <avogadro/molecule.h>

using namespace boost::python;
using namespace Avogadro;

void export_Atom()
{
  // define function pointers to handle overloading
  void (Atom::*setPos_ptr)(const Eigen::Vector3d&) = &Atom::setPos;
  void (Atom::*addBond_ptr1)(Bond*) = &Atom::addBond;
  void (Atom::*addBond_ptr2)(unsigned long int) = &Atom::addBond;
  void (Atom::*removeBond_ptr1)(Bond*) = &Atom::removeBond;
  void (Atom::*removeBond_ptr2)(unsigned long int) = &Atom::removeBond;

  class_<Avogadro::Atom, bases<Avogadro::Primitive>, boost::noncopyable>("Atom", no_init)
    // read/write properties
    .add_property("pos", make_function(&Atom::pos, return_value_policy<return_by_value>()), setPos_ptr)
    .add_property("atomicNumber", &Atom::atomicNumber, &Atom::setAtomicNumber)
    .add_property("partialCharge", &Atom::partialCharge, &Atom::setPartialCharge)
    // read-only properties
    .add_property("bonds", &Atom::bonds)
    .add_property("neighbors", &Atom::neighbors)
    .add_property("valence", &Atom::valence)
    .add_property("isHydrogen", &Atom::isHydrogen)
    // real functions
    .def("addBond", addBond_ptr1)
    .def("addBond", addBond_ptr2)
    .def("removeBond", removeBond_ptr1)
    .def("removeBond", removeBond_ptr2)
    ;

}
