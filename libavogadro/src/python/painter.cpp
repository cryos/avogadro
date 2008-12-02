#include <Python.h>
#include <boost/python.hpp>

#include <avogadro/painter.h>
#include <avogadro/mesh.h>
#include <avogadro/color.h>

#include <QVector>

using namespace boost::python;
using namespace Avogadro;

void export_Painter()
{
  void (Painter::*setName_ptr1)(const Primitive *) = &Painter::setName;
  void (Painter::*setName_ptr2)(Primitive::Type, int) = &Painter::setName;
  void (Painter::*setColor_ptr1)(const Color *) = &Painter::setColor;
  void (Painter::*setColor_ptr2)(float, float, float, float) = &Painter::setColor;
  //void (Painter::*setColor (const QColor *color) = 0;
  void (Painter::*drawTriangle_ptr1)(const Eigen::Vector3d &, const Eigen::Vector3d &, 
      const Eigen::Vector3d &) = &Painter::drawTriangle;
  void (Painter::*drawTriangle_ptr2)(const Eigen::Vector3d &, const Eigen::Vector3d &,
      const Eigen::Vector3d &, const Eigen::Vector3d &) = &Painter::drawTriangle;
  int (Painter::*drawText_ptr1)(int, int, const QString &) const = &Painter::drawText;
  int (Painter::*drawText_ptr2)(const Eigen::Vector3d &, const QString &) const = &Painter::drawText;
  
  class_<Avogadro::Painter, boost::noncopyable>("Painter", no_init)
    // read-only poperties 
    .add_property("quality", &Painter::quality)
    // real functions
    .def("setName", setName_ptr1)
    .def("setName", setName_ptr2)
    .def("setColor", setColor_ptr1)
    .def("setColor", setColor_ptr2)
    .def("drawTriangle", drawTriangle_ptr1)
    .def("drawTriangle", drawTriangle_ptr2)
    .def("drawSphere", &Painter::drawSphere)
    .def("drawCylinder", &Painter::drawCylinder)
    .def("drawMultiCylinder", &Painter::drawMultiCylinder)
    .def("drawCone", &Painter::drawCone)
    .def("drawLine", &Painter::drawLine)
    .def("drawMultiLine", &Painter::drawMultiLine)
    .def("drawSpline", &Painter::drawSpline)
    .def("drawShadedSector", &Painter::drawShadedSector)
    .def("drawArc", &Painter::drawArc)
    .def("drawShadedQuadrilateral", &Painter::drawShadedQuadrilateral)
    .def("drawQuadrilateral", &Painter::drawQuadrilateral)
    .def("drawMesh", &Painter::drawMesh)
    .def("drawColorMesh", &Painter::drawColorMesh)
    .def("drawText", drawText_ptr1)
    .def("drawText", drawText_ptr2)
    ;
   
}
