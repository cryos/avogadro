// Last update: timvdm 12 May 2009

#include <boost/python.hpp>

#include <avogadro/primitive.h>
#include <avogadro/cube.h>
#include <avogadro/molecule.h>

using namespace boost::python;
using namespace Avogadro;

void export_Cube()
{

  bool (Cube::*setLimits_ptr1)(const Eigen::Vector3d &, const Eigen::Vector3d &,
      const Eigen::Vector3i&) = &Cube::setLimits;
  bool (Cube::*setLimits_ptr2)(const Eigen::Vector3d &, const Eigen::Vector3d &, double) = &Cube::setLimits;
  bool (Cube::*setLimits_ptr3)(const Eigen::Vector3d &, const Eigen::Vector3i &, double) = &Cube::setLimits;
  bool (Cube::*setLimits_ptr4)(const Molecule *, double, double) = &Cube::setLimits;
  bool (Cube::*setLimits_ptr5)(const Cube &) = &Cube::setLimits;
  double (Cube::*value_ptr1)(int, int, int) const = &Cube::value;
  double (Cube::*value_ptr2)(const Eigen::Vector3i &) const = &Cube::value;
  double (Cube::*value_ptr3)(const Eigen::Vector3d &) const = &Cube::value;
  bool (Cube::*setValue_ptr1)(int, int, int, double) = &Cube::setValue;

  class_<Avogadro::Cube, bases<Avogadro::Primitive>, boost::noncopyable>("Cube", no_init)
    // read/write properties
    .add_property("name", &Cube::name, &Cube::setName)
    .add_property("data", make_function(&Cube::data, return_value_policy<return_by_value>()), &Cube::setData)
    // read-only properties
    .add_property("min", &Cube::min)
    .add_property("max", &Cube::max)
    .add_property("spacing", &Cube::spacing)
    .add_property("dimensions", &Cube::dimensions)
    .add_property("minValue", &Cube::minValue)
    .add_property("maxValue", &Cube::maxValue)
    // real functions
    .def("setLimits", setLimits_ptr1)
    .def("setLimits", setLimits_ptr2)
    .def("setLimits", setLimits_ptr3)
    .def("setLimits", setLimits_ptr4)
    .def("setLimits", setLimits_ptr5)
    .def("closestIndex", &Cube::closestIndex)
    .def("indexVector", &Cube::indexVector)
    .def("position", &Cube::position)
    .def("value", value_ptr1)
    .def("value", value_ptr2)
    .def("value", value_ptr3)
    .def("setValue", setValue_ptr1)
    // provide setData, it returns bool...
    .def("setData", &Cube::setData)
    .def("addData", &Cube::addData)
    ;

}
