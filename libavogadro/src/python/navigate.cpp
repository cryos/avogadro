// Last update: timvdm 12 May 2009

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
    // real functions
    .def("zoom", &Navigate::zoom)
    .staticmethod("zoom")
    .def("translate", translate_ptr1)
    .staticmethod("translate")
    .def("rotate", rotate_ptr1)
    .def("rotate", rotate_ptr2)
    .staticmethod("rotate")
    .def("tilt", &Navigate::tilt)
    .staticmethod("tilt")
    ;

}
