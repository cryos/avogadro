#include <Python.h>
#include <boost/python.hpp>

#include <avogadro/primitive.h>
#include <avogadro/cube.h>
#include <avogadro/molecule.h>

using namespace boost::python;
using namespace Avogadro;

void export_Cube()
{

  bool (Cube::*setLimits_ptr1)(const Eigen::Vector3d &, const Eigen::Vector3d &, double) = &Cube::setLimits;
  bool (Cube::*setLimits_ptr2)(const Eigen::Vector3d &, const Eigen::Vector3i &, double) = &Cube::setLimits;
  bool (Cube::*setLimits_ptr3)(const Molecule *, double, double) = &Cube::setLimits;
  double (Cube::*value_ptr1)(int, int, int) const = &Cube::value;
  double (Cube::*value_ptr2)(const Eigen::Vector3i &) const = &Cube::value;
  double (Cube::*value_ptr3)(const Eigen::Vector3d &) const = &Cube::value;

  class_<Avogadro::Cube, bases<Avogadro::Primitive>, boost::noncopyable>("Cube", no_init)
    .def("min", &Cube::min)
    .def("max", &Cube::max)
    .def("spacing", &Cube::spacing)
    .def("dimensions", &Cube::dimensions)
    .def("setLimits", setLimits_ptr1)
    .def("setLimits", setLimits_ptr2)
    .def("setLimits", setLimits_ptr3)
    .def("data", &Cube::data)
    .def("setData", &Cube::setData)
    .def("closestIndex", &Cube::closestIndex)
    .def("indexVector", &Cube::indexVector)
    .def("position", &Cube::position)
    .def("value", value_ptr1)
    .def("value", value_ptr2)
    .def("value", value_ptr3)
    .def("setValue", &Cube::setValue)
    .def("name", &Cube::name)
    .def("setName", &Cube::setName)
    ;

}
