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
    ;

}
