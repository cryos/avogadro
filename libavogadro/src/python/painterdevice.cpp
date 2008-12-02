#include <Python.h>
#include <boost/python.hpp>

#include <avogadro/painterdevice.h>
#include <avogadro/painter.h>
#include <avogadro/color.h>
#include <avogadro/molecule.h>
#include <avogadro/camera.h>

using namespace boost::python;
using namespace Avogadro;

void export_PainterDevice()
{
  class_<Avogadro::PainterDevice, boost::noncopyable>("PainterDevice", no_init)
    // read/write properties
    .add_property("painter", make_function(&PainterDevice::painter, return_value_policy<reference_existing_object>()))
    .add_property("camera",  make_function(&PainterDevice::camera, return_value_policy<reference_existing_object>()))
    .add_property("molecule",  make_function(&PainterDevice::molecule, return_value_policy<reference_existing_object>()))
    .add_property("colorMap",  make_function(&PainterDevice::colorMap, return_value_policy<reference_existing_object>()))
    .add_property("width", &PainterDevice::width)
    .add_property("height", &PainterDevice::height)
    // read-only poperties 
    // real functions
    .def("isSelected", &PainterDevice::isSelected)
    .def("radius", &PainterDevice::radius)
    ;
   
}
