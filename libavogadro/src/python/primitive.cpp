#include <Python.h>
#include <boost/python.hpp>

#include <avogadro/primitive.h>

using namespace boost::python;
using namespace Avogadro;

void export_Primitive()
{

  class_<Avogadro::Primitive, boost::noncopyable>("Primitive")
    // read-only properties
    .add_property("id", &Primitive::id) // read-only, managed by Molecule
    .add_property("index", &Primitive::index) // read-only, managed by Molecule
    // real functions
    .def("update", &Primitive::update)
    ;
 
}
