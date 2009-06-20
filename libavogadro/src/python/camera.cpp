// Last update: timvdm 18 June 2009
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
  
  class_<Avogadro::Camera, boost::noncopyable>("Camera", "Representation of the view")
    //
    // constructors
    //
    .def(init<const GLWidget*>())
    .def(init<const GLWidget*, double>())
    .def(init<const Camera*>())
    
    //
    // read/write properties
    //
    .add_property("angleOfViewY", 
        &Camera::angleOfViewY, 
        &Camera::setAngleOfViewY,
        "The vertical viewing angle in degrees.")

    .add_property("modelview", 
        make_function(modelview_ptr, return_value_policy<return_by_value>()),
        &Camera::setModelview, 
        "4x4 \"modelview\" matrix representing the camera orientation and position.")

    //
    // read-only properties
    //
    .add_property("parent", 
        make_function(&Camera::parent, return_value_policy<reference_existing_object>()),
        "The parent GLWidget.")

    .add_property("backTransformedXAxis", 
        &Camera::backTransformedXAxis,
        "Returns a unit vector pointing toward the right, expressed in the "
        "scene's coordinate system. This is simply the "
        "unit vector that is mapped to (1,0,0) by the camera rotation.")

    .add_property("backTransformedYAxis", 
        &Camera::backTransformedYAxis,
        "Returns a unit vector pointing upward, expressed in the "
        "scene's coordinate system. This is simply the "
        "unit vector that is mapped to (0,1,0) by the camera rotation.")

    .add_property("backTransformedZAxis", 
        &Camera::backTransformedZAxis,
        "Returns a unit vector pointing toward the camera, expressed in the "
        "scene's coordinate system. This is simply the "
        "unit vector that is mapped to (0,0,1) by the camera rotation.")

    .add_property("transformedXAxis", 
        &Camera::transformedXAxis,
        "Returns a unit vector pointing in the x direction, expressed in the "
        "space coordinate system.")

    .add_property("transformedYAxis", 
        &Camera::transformedYAxis,
        "Returns a unit vector pointing in the y direction, expressed in the "
        "space coordinate system.")

    .add_property("transformedZAxis", 
        &Camera::transformedZAxis,
        "Returns a unit vector pointing in the z direction, expressed in the "
        "space coordinate system.")

    // real functions 
    .def("applyPerspective", 
        &Camera::applyPerspective,
        "Calls gluPerspective() with parameters automatically chosen "
        "for rendering the GLWidget's molecule with this camera. Should be called "
        "only in GL_PROJECTION matrix mode.")

    .def("applyModelview", 
        &Camera::applyModelview,
        "Calls glMultMatrix() with the camera's \"modelview\" matrix. Should be called "
        "only in GL_MODELVIEW matrix mode.")

    .def("initializeViewPoint", 
        &Camera::initializeViewPoint,
        "Sets up the camera so that it gives a nice view of the molecule loaded in the "
        "parent GLWidget. Typically you would call this method right after loading a molecule.")

    .def("distance", 
        &Camera::distance, 
        "Returns the distance between @a point and the camera.")

    .def("translate", 
        &Camera::translate, 
        "Multiply the camera's \"modelview\" matrix on the right by the translation of the given "
        "vector. As the translation is applied on the right, the vector is understood in "
        "the molecule's coordinate system. Use this method if you want to give the impression "
        "that the molecule is moving while the camera remains fixed. This is the equivalent "
        "of the OpenGL function glTranslate().")

    .def("pretranslate", 
        &Camera::pretranslate,
        "Multiply the camera's \"modelview\" matrix on the left by the translation of given "
        "vector. Because the translation is applied on the left, the vector is understood in "
        "the coordinate system obtained by applying the camera's matrix to the molecule's "
        "coordinate system. Use this method if you want to give the impression that the camera "
        "is moving while the molecule remains fixed.")
    
    .def("rotate", 
        &Camera::rotate, 
        "Multiply the camera's \"modelview\" matrix on the right by the rotation of the given "
        "angle and axis. As the rotation is applied on the right, the axis vector is "
        "understood in the molecule's coordinate system. Use this method if you want to give "
        "the impression that the molecule is rotating while the camera remains fixed. This is the "
        "equivalent of the OpenGL function glRotate(), except that here the angle is expressed "
        "in radians, not in degrees. After the rotation is multiplied, a normalization is "
        "performed to ensure that the camera matrix remains sane.")

    .def("prerotate", 
        &Camera::prerotate,
        "Multiply the camera's \"modelview\" matrix on the left by the rotation of the given "
        "angle and axis. Because the rotation is applied on the left, the axis vector is "
        "understood in the the coordinate system obtained by applying the camera's matrix to "
        "the molecule's coordinate system. Use this method if you want to give "
        "the impression that the camera is rotating while the molecule remains fixed. "
        "After the rotation is multiplied, a normalization is performed to ensure that the "
        "camera matrix remains sane.")

    .def("unProjectWithZ", 
        unProject_ptr1, 
        "Performs an unprojection from window coordinates to space coordinates.")

    .def("unProject", 
        unProject_ptr2,
        "Performs an unprojection from window coordinates to space coordinates, "
        "into the plane passing through a given reference point and parallel to the screen. "
        "Thus the returned vector is a point of that plane. The rationale is that "
        "when unprojecting 2D window coords to 3D space coords, there are a priori "
        "infinitely many solutions, and one has to be choose. This is equivalent to "
        "choosing a plane parallel to the screen.")

    .def("unProject", 
        unProject_ptr3, 
        "Performs an unprojection from window coordinates to space coordinates, "
        "into the plane passing through the molecule's center and parallel to the screen. "
        "Thus the returned vector is a point belonging to that plane.")

    .def("project", 
        &Camera::project, 
        "Performs a projection from space coordinates to window coordinates.")

    .def("normalize", 
        &Camera::normalize,
        "The linear component (ie the 3x3 topleft block) of the camera matrix must "
        "always be a rotation. But after several hundreds of operations on it, "
        "it can drift farther and farther away from being a rotation. This method "
        "normalizes the camera matrix so that the linear component is guaranteed to be "
        "a rotation. Concretely, it performs a Gram-Schmidt orthonormalization to "
        "transform the linear component into a nearby rotation. "
        "The bottom row must always have entries 0, 0, 0, 1. This function overwrites "
        "the bottom row with these values.")
    ;

}
