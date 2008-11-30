#include <Python.h>
#include <boost/python.hpp>

#include <avogadro/camera.h>
#include <avogadro/glwidget.h>

using namespace boost::python;
using namespace Avogadro;

void export_Camera()
{

  //Eigen::Transform3d& (Camera::*modelview_ptr)() = &Camera::modelview;
  Eigen::Vector3d (Camera::*unProject_ptr1)(const Eigen::Vector3d&) const = &Camera::unProject;
  Eigen::Vector3d (Camera::*unProject_ptr2)(const QPoint&, const Eigen::Vector3d&) const = &Camera::unProject;
  Eigen::Vector3d (Camera::*unProject_ptr3)(const QPoint&) const = &Camera::unProject;
  
  class_<Avogadro::Camera, boost::noncopyable>("Camera")
    .def("parent", &Camera::parent, return_value_policy<reference_existing_object>())
    .def("setAngleOfViewY", &Camera::setAngleOfViewY)
    .def("angleOfViewY", &Camera::angleOfViewY)
    .def("setModelview", &Camera::setModelview)
    //.def("modelview", modelview_ptr)
    .def("applyPerspective", &Camera::applyPerspective)
    .def("applyModelview", &Camera::applyModelview)
    .def("initializeViewPoint", &Camera::initializeViewPoint)
    .def("distance", &Camera::distance)
    .def("translate", &Camera::translate)
    .def("pretranslate", &Camera::pretranslate)
    .def("rotate", &Camera::rotate)
    .def("prerotate", &Camera::prerotate)
    .def("unProject", unProject_ptr1)
    .def("unProject", unProject_ptr2)
    .def("unProject", unProject_ptr3)
    .def("project", &Camera::project)
    .def("backTransformedXAxis", &Camera::backTransformedXAxis)
    .def("backTransformedYAxis", &Camera::backTransformedYAxis)
    .def("backTransformed2Axis", &Camera::backTransformedZAxis)
    .def("transformedXAxis", &Camera::transformedXAxis)
    .def("transformedYAxis", &Camera::transformedYAxis)
    .def("transformedZAxis", &Camera::transformedZAxis)
    .def("normalize", &Camera::normalize)
    ;

}
