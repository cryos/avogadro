// Last update: timvdm 12 May 2009

#include <boost/python.hpp>

#include <avogadro/color.h>
#include <avogadro/primitive.h>

using namespace boost::python;
using namespace Avogadro;

// handle default arguments
void setFromRgba(Color &self, GLfloat r, GLfloat g, GLfloat b)
{
  self.setFromRgba(r, g, b, 1.0);
}

void export_Color()
{
  class_<Avogadro::Color, bases<Avogadro::Plugin>, boost::noncopyable>("Color")
    //
    // constructors
    //
    .def(init<GLfloat, GLfloat, GLfloat>())
    .def(init<GLfloat, GLfloat, GLfloat, GLfloat>())
    .def(init<const Primitive*>())

    //
    // read-only poperties 
    //
    .add_property("red", 
        &Color::red, 
        "The red component of the color.")

    .add_property("green", 
        &Color::green, 
        "The green component of the color.")

    .add_property("blue", 
        &Color::blue, 
        "The blue component of the color.")

    //
    // read/write properties
    //
    .add_property("name", 
        &Color::name, 
        &Color::setName, 
        "The translated name of this instance of the class.")

    .add_property("alpha", 
        &Color::alpha, 
        &Color::setAlpha,
        "the alpha component of the color, 0.0 means totally transparent and"
        "1.0 means totally opaque.")

    //
    // real functions
    //
    .def("setFromPrimitive", 
        &Color::setFromPrimitive, 
        "Set the color based on the supplied Primitive. If NULL is passed do nothing.")

    .def("setFromIndex", 
        &Color::setFromIndex,
        "Set the color based on the supplied index (e.g., in an indexed color table).")

    .def("setFromGradient", 
        &Color::setFromGradient,
        "Set the color based on the supplied floating point value (e.g., a gradient).")

    .def("setFromQColor", 
        &Color::setFromQColor,
        "Set the color explicitly based on a QColor, copying RGB and Alpha levels.")

    .def("setFromRgba", 
        &Color::setFromRgba,
        "Set the four components of the color individually. Each one ranges from 0.0 "
        "(lowest intensity) to 1.0 (highest intensity). For the alpha component, 0.0 "
        "means totally transparent and 1.0 (the default) means totally opaque.")

    .def("setFromRgba", 
        &setFromRgba,
        "Set the four components of the color individually. Each one ranges from 0.0 "
        "(lowest intensity) to 1.0 (highest intensity). For the alpha component, 0.0 "
        "means totally transparent and 1.0 (the default) means totally opaque.")

    .def("setToSelectionColor", 
        &Color::setToSelectionColor,
        "Set the color to the selection color. By default, the selection color is (0.3, 0.6, 1.0, 0.7)")

    .def("apply", 
        &Color::apply, 
        "Sets this color to be the one used by OpenGL for rendering when lighting is disabled.")

    .def("applyAsMaterials", 
        &Color::applyAsMaterials,
        "Applies nice OpenGL materials using this color as the "
        "diffuse color while using different shades for the ambient and "
        "specular colors. This is only useful if lighting is enabled.")

    .def("applyAsFlatMaterials", 
        &Color::applyAsFlatMaterials, 
        "Applies an OpenGL material more appropriate for flat surfaces.")
    ;
   
}
