// Last update: timvdm 19 June 2009
#include <boost/python.hpp>

#include <avogadro/navigate.h>
#include <avogadro/glwidget.h>

using namespace boost::python;
using namespace Avogadro;

void export_Navigate()
{ 
  void (*translate_ptr1)(GLWidget*, const Eigen::Vector3d&, double, double) = &Navigate::translate;
  void (*rotate_ptr1)(GLWidget*, const Eigen::Vector3d&, double, double) = &Navigate::rotate;
  void (*rotate_ptr2)(GLWidget*, const Eigen::Vector3d&, double, double, double) = &Navigate::rotate;
 
  class_<Avogadro::Navigate, boost::noncopyable>("Navigate", no_init)
    //
    // real functions
    //
    .def("zoom", 
        &Navigate::zoom,
        "Zooms toward a given point by the given amount.")
        .staticmethod("zoom")

    .def("translate", 
        translate_ptr1,
        "Translate between the from and to positions relative to what.")
        .staticmethod("translate")
    
    .def("rotate", 
        rotate_ptr1,
        "Rotate about center by the amounts deltaX and deltaY in tha x and y axes.")
    .def("rotate", 
        rotate_ptr2,
        "Rotate about center by deltaX, deltaY, and deltaZ in the x, y and z axes "
        "A generalization of the rotate() and tilt() methods.")
        .staticmethod("rotate")

    .def("tilt", 
        &Navigate::tilt,
        "Tilt about center by the amount delta z axis.")
        .staticmethod("tilt")
    ;

}
