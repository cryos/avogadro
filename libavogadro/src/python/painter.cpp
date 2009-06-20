// Last update: timvdm 19 June 2009
#include <boost/python.hpp>

#include <avogadro/painter.h>
#include <avogadro/mesh.h>
#include <avogadro/color.h>

#include <QVector>

using namespace boost::python;
using namespace Avogadro;

void setColor(Painter &self, float r, float g, float b)
{
  self.setColor(r, g, b);
}

void drawCone(Painter &self, const Eigen::Vector3d &base, const Eigen::Vector3d &cap, double baseRadius)
{
  self.drawCone(base, cap, baseRadius);
}

void drawShadedSector(Painter &self, const Eigen::Vector3d &origin, const Eigen::Vector3d &dir1,
    const Eigen::Vector3d &dir2, double radius)
{
  self.drawShadedSector(origin, dir1, dir2, radius);
}

void drawArc(Painter &self, const Eigen::Vector3d & origin, const Eigen::Vector3d & direction1, 
    const Eigen::Vector3d & direction2, double radius, double lineWidth)
{
  self.drawArc(origin, direction1, direction2, radius, lineWidth);
}
    
void drawMesh(Painter &self, const Mesh & mesh)
{
  self.drawMesh(mesh);
}
void drawColorMesh(Painter &self, const Mesh & mesh)
{
  self.drawColorMesh(mesh);
}

void export_Painter()
{
  void (Painter::*setName_ptr1)(const Primitive *) = &Painter::setName;
  void (Painter::*setName_ptr2)(Primitive::Type, int) = &Painter::setName;
  void (Painter::*setColor_ptr1)(const Color *) = &Painter::setColor;
  void (Painter::*setColor_ptr2)(float, float, float, float) = &Painter::setColor;
  //void (Painter::*setColor (const QColor *color) = 0;
  void (Painter::*drawSphere_ptr1)(const Eigen::Vector3d &, double) = &Painter::drawSphere;
  void (Painter::*drawTriangle_ptr1)(const Eigen::Vector3d &, const Eigen::Vector3d &, 
      const Eigen::Vector3d &) = &Painter::drawTriangle;
  void (Painter::*drawTriangle_ptr2)(const Eigen::Vector3d &, const Eigen::Vector3d &,
      const Eigen::Vector3d &, const Eigen::Vector3d &) = &Painter::drawTriangle;
  int (Painter::*drawText_ptr1)(int, int, const QString &) = &Painter::drawText;
  int (Painter::*drawText_ptr2)(const Eigen::Vector3d &, const QString &) = &Painter::drawText;
  
  class_<Avogadro::Painter, boost::noncopyable>("Painter", no_init)
    // 
    // read-only poperties 
    //
    .add_property("quality", 
        &Painter::quality,
        "The current global quality setting.")

    // real functions
    .def("setName", 
        setName_ptr1,
        "Uses the primitive to set the type and name if the Paint Device supports it.")
    .def("setName", 
        setName_ptr2,
        "Sets the primitive type and id.")

    .def("setColor", 
        setColor_ptr1, 
        "Set the color to paint the primitive elements with.")
    .def("setColor", 
        setColor_ptr2, 
        "Set the color to paint elements with where 0.0 is the minimum and 1.0 "
        "is the maximum.")
    .def("setColor", 
        setColor, 
        "Set the color to paint elements with where 0.0 is the minimum and 1.0 "
        "is the maximum.")


    .def("drawSphere", 
        drawSphere_ptr1,
        "Draws a sphere, leaving the Painter choose the appropriate detail level based on the "
        "apparent radius (ratio of radius over distance) and the global quality setting.")
    
    .def("drawCylinder", 
        &Painter::drawCylinder, 
        "Draws a cylinder, leaving the Painter choose the appropriate detail level based on the "
        "apparent radius (ratio of radius over distance) and the global quality setting.")

    .def("drawMultiCylinder", 
        &Painter::drawMultiCylinder, 
        "Draws a multiple cylinder (see below), leaving the Painter choose the appropriate "
        " detail level based on the apparent radius (ratio of radius over distance) and the "
        "global quality setting.")

    .def("drawCone", 
        &Painter::drawCone, 
        "Draws a cone between the tip and the base with the base radius given.")
    .def("drawCone", 
        drawCone, 
        "Draws a cone between the tip and the base with the base radius given.")

    .def("drawLine", 
        &Painter::drawLine, 
        "Draws a line between the given points of the given width.")

    .def("drawMultiLine", 
        &Painter::drawMultiLine, 
        "Draws a multiple line between the given points. This function is the "
        "line equivalent to the drawMultiCylinder function and performs the "
        "same basic operations using simpler and quicker lines.")

    .def("drawTriangle", 
        drawTriangle_ptr1,
        "Draws a triangle with vertives on the three given points. This function "
        "calculates the normal of the triangle and corrects the winding order to "
        "ensure the front face is facing the camera.")
    .def("drawTriangle", 
        drawTriangle_ptr2,
        "Draws a triangle with vertives on the three given points using the "
        "given normal. This function corrects the triangle's winding order.")
 
    .def("drawSpline", 
        &Painter::drawSpline, 
        "Draw a cubic B-spline between the given points.")

    .def("drawShadedSector", 
        &Painter::drawShadedSector,
        "Draws a shaded sector of a circle.  The sector is defined by three vectors, "
        "the center of the circle, and two vectors that define the lines going out "
        "from the centre of the circle to the circumference of the circle.  The "
        "actual points on the circumference are found using these two vectors and "
        "the radius of the circle.")
    .def("drawShadedSector", 
        drawShadedSector,
        "Draws a shaded sector of a circle.  The sector is defined by three vectors, "
        "the center of the circle, and two vectors that define the lines going out "
        "from the centre of the circle to the circumference of the circle.  The "
        "actual points on the circumference are found using these two vectors and "
        "the radius of the circle.")

    .def("drawArc", 
        &Painter::drawArc,
        "Draws an arc.  The arc is defined by three vectors, the center of the circle, "
        "and two vectors that define the lines going out from the center of the "
        "circle to the circumference of the circle.  The actual points on the "
        "circumference are found using these two vectors and the radius of the circle.")

    .def("drawShadedQuadrilateral", 
        &Painter::drawShadedQuadrilateral,
        "Draws a solid two dimensional quadrilateral in three dimensional space.")

    .def("drawQuadrilateral", 
        &Painter::drawQuadrilateral,
        "Draws the outline of a two dimensional quadrilateral in three dimensional space.")

    .def("drawMesh", 
        &Painter::drawMesh,
        "Draws a continuous mesh of triangles.")
    .def("drawMesh", 
        drawMesh,
        "Draws a continuous mesh of triangles.")

    .def("drawColorMesh", 
        &Painter::drawColorMesh,
        "Draws a continuous mesh of triangles and respects the colors stored.")
    .def("drawColorMesh", 
        drawColorMesh,
        "Draws a continuous mesh of triangles and respects the colors stored.")

    .def("drawText", 
        drawText_ptr1,
        "Draws text at a given window position, on top of the scene.")
    .def("drawText", 
        drawText_ptr2,
        "Draws text at a given scene position, inside the scene.")
    ;
   
}
