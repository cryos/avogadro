// Last update: timvdm 12 May 2009

//#include <Python.h>
// http://www.boost.org/doc/libs/1_39_0/libs/python/doc/building.html#include-issues
#include <boost/python/detail/wrap_python.hpp>
#include <boost/python.hpp>

#include <avogadro/camera.h>
#include <avogadro/glwidget.h>

using namespace boost::python;
using namespace Avogadro;

void export_Camera()
{

  const Eigen::Transform3d& (Camera::*modelview_ptr)() const = &Camera::modelview;
  Eigen::Vector3d (Camera::*unProject_ptr1)(const Eigen::Vector3d&) const = &Camera::unProject;
  Eigen::Vector3d (Camera::*unProject_ptr2)(const QPoint&, const Eigen::Vector3d&) const = &Camera::unProject;
  Eigen::Vector3d (Camera::*unProject_ptr3)(const QPoint&) const = &Camera::unProject;
  
  class_<Avogadro::Camera, boost::noncopyable>("Camera")
    .def(init<const GLWidget*, double>())
    .def(init<const Camera*>())
    // read/write properties
    .add_property("angleOfViewY", &Camera::angleOfViewY, &Camera::setAngleOfViewY)
    .add_property("modelview", make_function(modelview_ptr, return_value_policy<return_by_value>()),
        &Camera::setModelview)
 
    // read-only properties
    .add_property("parent", make_function(&Camera::parent, return_value_policy<reference_existing_object>()))
    .add_property("backTransformedXAxis", &Camera::backTransformedXAxis)
    .add_property("backTransformedYAxis", &Camera::backTransformedYAxis)
    .add_property("backTransformedZAxis", &Camera::backTransformedZAxis)
    .add_property("transformedXAxis", &Camera::transformedXAxis)
    .add_property("transformedYAxis", &Camera::transformedYAxis)
    .add_property("transformedZAxis", &Camera::transformedZAxis)

    // real functions 
    .def("applyPerspective", &Camera::applyPerspective)
    .def("applyModelview", &Camera::applyModelview)
    .def("initializeViewPoint", &Camera::initializeViewPoint)
    .def("distance", &Camera::distance)
    .def("translate", &Camera::translate)
    .def("pretranslate", &Camera::pretranslate)
    .def("rotate", &Camera::rotate)
    .def("prerotate", &Camera::prerotate)
    .def("unProjectWithZ", unProject_ptr1)
    .def("unProject", unProject_ptr2)
    .def("unProject", unProject_ptr3)
    .def("project", &Camera::project)
    .def("normalize", &Camera::normalize)
    ;

}
