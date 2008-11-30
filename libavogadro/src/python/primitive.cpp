#include <Python.h>
#include <boost/python.hpp>

#include <avogadro/primitive.h>

using namespace boost::python;
using namespace Avogadro;

void export_Primitive()
{

  class_<Avogadro::Primitive, boost::noncopyable>("Primitive")
    .def("id", &Primitive::id)
    .def("index", &Primitive::index)
    .def("update", &Primitive::update)
    ;
 
}
