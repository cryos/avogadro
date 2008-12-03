#include <boost/python.hpp>

#include <avogadro/color.h>
#include <avogadro/primitive.h>

using namespace boost::python;
using namespace Avogadro;

void export_Color()
{
  void (Color::*set_ptr1)(GLfloat, GLfloat, GLfloat, GLfloat) = &Color::set;
  void (Color::*set_ptr2)(const Primitive*) = &Color::set;

  class_<Avogadro::Color, bases<Avogadro::Plugin>, boost::noncopyable>("Color")
    // constructors
    .def(init<GLfloat, GLfloat, GLfloat, GLfloat>())
    .def(init<const Primitive*>())
    // read-only poperties 
    .add_property("red", &Color::red)
    .add_property("green", &Color::green)
    .add_property("blue", &Color::blue)
    // read/write properties
    .add_property("name", &Color::name, &Color::setName)
    .add_property("aplha", &Color::alpha, &Color::setAlpha)
    // real functions
    .def("set", set_ptr1)
    .def("set", set_ptr2)
    .def("setToSelectionColor", &Color::setToSelectionColor)
    .def("apply", &Color::apply)
    .def("applyAsMaterials", &Color::applyAsMaterials)
    .def("applyAsFlatMaterials", &Color::applyAsFlatMaterials)
    ;
   
}
