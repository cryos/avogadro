// Last update: timvdm 12 May 2009

//#include <Python.h>
// http://www.boost.org/doc/libs/1_39_0/libs/python/doc/building.html#include-issues
#include <boost/python/detail/wrap_python.hpp>
#include <boost/python.hpp>

#include <avogadro/primitive.h>
#include <avogadro/atom.h>
#include <avogadro/bond.h>
#include <avogadro/residue.h>
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
    .add_property("forceVector", &Atom::forceVector, &Atom::setForceVector)
    // read-only properties
    .add_property("residue", make_function(&Atom::residue, return_value_policy<reference_existing_object>()))
    .add_property("residueId", &Atom::residueId)
    .add_property("bonds", &Atom::bonds)
    .add_property("neighbors", &Atom::neighbors)
    .add_property("valence", &Atom::valence)
    .add_property("isHydrogen", &Atom::isHydrogen)
    // real functions
    .def("bond", make_function(&Atom::bond, return_value_policy<reference_existing_object>()))
    ;

}
