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
    //
    // read/write properties
    //
    .add_property("name", 
        &Cube::name, 
        &Cube::setName)

    .add_property("data", 
        make_function(&Cube::data, return_value_policy<return_by_value>()), 
        &Cube::setData, 
        "List containing all the data in a one-dimensional array.")

    //
    // read-only properties
    //
    .add_property("min", 
        &Cube::min, 
        "The minimum point in the cube.")

    .add_property("max", 
        &Cube::max, 
        "The maximum point in the cube.")

    .add_property("spacing", 
        &Cube::spacing, 
        "The spacing of the grid.")

    .add_property("dimensions", 
        &Cube::dimensions, 
        "The x, y and z dimensions of the cube.")

    .add_property("minValue", 
        &Cube::minValue, 
        "The minimum  value at any point in the Cube.")

    .add_property("maxValue", 
        &Cube::maxValue, 
        "The mzximum  value at any point in the Cube.")

    //
    // real functions
    //
    .def("setLimits", 
        setLimits_ptr1, 
        "Set the limits of the cube.")
    .def("setLimits", 
        setLimits_ptr2, 
        "Set the limits of the cube.")
    .def("setLimits", 
        setLimits_ptr3, 
        "Set the limits of the cube.")
    .def("setLimits", 
        setLimits_ptr4, 
        "Set the limits of the cube.")
    .def("setLimits", 
        setLimits_ptr5, 
        "Set the limits of the cube.")

    .def("closestIndex", 
        &Cube::closestIndex, 
        "Index of the point closest to the position supplied.")

    .def("indexVector", 
        &Cube::indexVector, 
        "Index vector of the point closest to the position supplied, in the form of i, j, k.")

    .def("position", 
        &Cube::position, 
        "Position of the given index.")

    .def("value", 
        value_ptr1, 
        "This function is very quick as it just returns the value at the point.")
    .def("value", 
        value_ptr2, 
        "This function is very quick as it just returns the value at the point.")
    .def("value", 
        value_ptr3, 
        "This function uses trilinear interpolation to find the value at points "
        "between those specified in the cube.")

    .def("setValue", 
        setValue_ptr1, 
        "Sets the value at the specified point in the cube.")

    // provide setData, it returns bool...
    .def("setData", 
        &Cube::setData, 
        "Set the values in the cube.")

    .def("addData", 
        &Cube::addData, 
        "Add the values in the cube")
    ;

}
