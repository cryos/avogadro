/**********************************************************************
  POVPainter - drawing spheres, cylinders and text in a POVRay scene

  Copyright (C) 2007 Marcus D. Hanwell

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.sourceforge.net/>

  Avogadro is free software; you can redistribute it and/or modify 
  it under the terms of the GNU General Public License as published by 
  the Free Software Foundation; either version 2 of the License, or 
  (at your option) any later version.

  Avogadro is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.
 **********************************************************************/

#ifndef __POVPAINTER_H
#define __POVPAINTER_H

#include <avogadro/global.h>
#include <avogadro/glwidget.h>

class QFile;
class QTextStream;

using namespace Eigen;

namespace Avogadro
{
  // Forward declaration
  class Color;

  /**
   * @class POVPainter povpainter.h
   * @brief Implementation of the Painter class using POV-Ray.
   * @author Marcus D. Hanwell
   *
   * This class implements the base Painter class using POV-Ray. It is intended 
   * to be used with the POV-Ray to raytrace molecules and other constructs to
   * a POV-Ray scene.
   *
   * @sa Painter
   */
  class POVPainterPrivate;
  class A_EXPORT POVPainter : public Painter
  {
  public:
    /**
     * Constructor.
     * @param quality defaults to -1, valid range from 0 to 4.
     */
    POVPainter();

    /**
     * Destructor
     */
    ~POVPainter();

    /**
     * Uses the primitive to set the type and name. Not used by POV-Ray as it
     * is not an interactive display.
     */
    void setName (const Primitive *) { }

    /**
     * Sets the primitive type and id. Not used by POV-Ray as it is not an
     * interactive display.
     */
    void setName (Primitive::Type, int) { }

    /**
     * Set the color to paint the primitives with.
     * @param color the color to be used for painting.
     */
    void setColor (const Color *color);

    /**
     * Set the color to paint elements with where 0.0 is the minimum and 1.0
     * is the maximum.
     * @param red component of the color.
     * @param green component of the color.
     * @param blue component of the color.
     * @param alpha component of the color.
     */
    void setColor (float red, float green, float blue, float alpha = 1.0);

    /**
     * Sets the plane normal of the scene, this affects the way higher order
     * bonds are rendered.
     */
    void setPlaneNormal (Vector3d planeNormalVector);

    /**
     * Draws a sphere, leaving the Painter choose the appropriate detail level based on the
     * apparent radius (ratio of radius over distance) and the global quality setting.
     * @param center the position of the center of the sphere.
     * @param radius the radius of the sphere.
     */
    void drawSphere (const Vector3d & center, double radius);

    /**
     * Draws a cylinder, leaving the Painter choose the appropriate detail level based on the
     * apparent radius (ratio of radius over distance) and the global quality setting.
     * @param end1 the position of the first end of the cylinder.
     * @param end2 the position of the second end of the cylinder.
     * @param radius the radius, i.e. half-width of the cylinder.
     */
    void drawCylinder (const Vector3d &end1, const Vector3d &end2,
                        double radius);

    /**
     * Draws a multiple cylinder (see below), leaving the Painter choose the appropriate
     * detail level based on the apparent radius (ratio of radius over distance) and the
     * global quality setting.
     *
     * What is a "multiple cylinder"? Think bond of order two or more between two atoms.
     * This function is here to allow drawing multiple bonds in a single call.
     *
     * This function takes care of rendering multiple bonds in such a way that the individual
     * bonds avoid hiding each other, at least in the defaut viewpoint of a molecule.
     * To achieves that, it asks the GLWidget for the the normal vector of the
     * molecule's best-fitting plane.
     *
     * @param end1 the position of the first end of the bond.
     * @param end2 the position of the second end of the bond.
     * @param radius the radius, i.e. half-width of each cylinder.
     * @param order the multiplicity order of the bond, e.g. 2 for a double bond.
     *              When this parameter equals 1, this function is equivalent to
     *              drawCylinder().
     * @param shift how far away from the central axis the cylinders are shifted.
     *              In other words this influences the total width of multiple bonds.
     */
    void drawMultiCylinder (const Vector3d &end1, const Vector3d &end2,
                            double radius, int order, double shift);

    /**
     * Draws a line between the given points of the given width.
     * @param start the position of the start of the line.
     * @param end the position of the end of the line.
     * @param lineWidth the width of the line.
     */
    void drawLine(const Eigen::Vector3d &, const Eigen::Vector3d &,
                  double) { }

    /**
     * Draws a multiple line between the given points. This function is the
     * line equivalent to the drawMultiCylinder function and performs the 
     * same basic operations using simpler and quicker lines.
     * @param start the position of the start of the line.
     * @param end the position of the end of the line.
     * @param lineWidth the width of the line.
     * @param order the order of the bond, e.g. 2 for a double bond.
     * @param stipple The stipple parameter for the bond, can be used to
     * draw aromatic bonds etc.
     * sa drawMultiCylinder
     */
    void drawMultiLine(const Eigen::Vector3d &, const Eigen::Vector3d &,
                       double, int, short) { }

    /**
     * Draws a triangle with vertives on the three given points. This function
     * calculates the normal of the triangle and corrects the winding order to
     * ensure the front face is facing the camera.
     * @param p1 first triangle vertex.
     * @param p2 second triangle vertex.
     * @param p3 third triangle vertex.
     */
    void drawTriangle(const Eigen::Vector3d &, const Eigen::Vector3d &,
                      const Eigen::Vector3d &) { }

    /**
     * Draws a triangle with vertives on the three given points using the
     * given normal. This function corrects the triangle's winding order.
     * @param p1 first triangle vertex.
     * @param p2 second triangle vertex.
     * @param p3 third triangle vertex.
     * @param n the normal of the triangle.
     */
    void drawTriangle(const Eigen::Vector3d &, const Eigen::Vector3d &,
                      const Eigen::Vector3d &, const Eigen::Vector3d &) { }

    /**
     * Draw a cubic B-spline between the given points.
     * @param pts QVector containing the points to draw the cubic B-spline
     * along.
     * @param radius the radius of the cubic B-spline.
     */
    void drawSpline(const QVector<Eigen::Vector3d>&, double) { }

    /**
     * Draws a shaded sector of a circle.  The sector is defined by three vectors,
     * the center of the circle, and two vectors that define the lines going out
     * from the centre of the circle to the circumference of the circle.  The
     * actual points on the circumference are found using these two vectors and
     * the radius of the circle.
     *
     * @param origin the center of the circle this sector is a portion of.
     * @param direction1 a vector defining the line the first point will lie on.
     * @param direction2 a vector defining the line the second point will lie on.
     * @param radius the radius of the circle this sector is a portion of.
     * @param alternateAngle whether to draw the obtuse angle made by the two vectors
     *                       instead of the acute angle between them.
     */
    void drawShadedSector(Eigen::Vector3d origin, Eigen::Vector3d direction1,
                          Eigen::Vector3d direction2, double radius,
                          bool alternateAngle = false);

    /**
     * Draws an arc.  The arc is defined by three vectors, the center of the circle,
     * and two vectors that define the lines going out from the center of the
     * circle to the circumference of the circle.  The actual points on the
     * circumference are found using these two vectors and the radius of the circle.
     *
     * @param origin the center of the circle whose circumference this arc is a portion of.
     * @param direction1 a vector defining the line the start of the arc will lie on.
     * @param direction2 a vector defining the line the end of the arc will lie on.
     * @param radius the radius of the circle whose circumference this arc is a portion of.
     * @param lineWidth the thickness of the line the arc will be drawn with.
     * @param alternateAngle whether to draw the obtuse angle made by the two vectors
     *                       instead of the acute angle between them.
     */
    void drawArc(Eigen::Vector3d origin, Eigen::Vector3d direction1, Eigen::Vector3d direction2,
                 double radius, double lineWidth, bool alternateAngle = false);

    /**
     * Draws a solid two dimensional quadrilateral in three dimensional space.
     *
     * @param point1 the first of the four corners of the quadrilateral.
     * @param point2 the second of the four corners of the quadrilateral.
     * @param point3 the third of the four corners of the quadrilateral.
     * @param point4 the last of the four corners of the quadrilateral.
     */
    void drawShadedQuadrilateral(Eigen::Vector3d point1, Eigen::Vector3d point2,
                                 Eigen::Vector3d point3, Eigen::Vector3d point4);

    /**
     * Draws the outline of a two dimensional quadrilateral in three dimensional space.
     *
     * @param point1 the first of the four corners of the quadrilateral.
     * @param point2 the second of the four corners of the quadrilateral.
     * @param point3 the third of the four corners of the quadrilateral.
     * @param point4 the last of the four corners of the quadrilateral.
     * @param lineWidth the thickness of the line the quadrilateral will be drawn with.
     */
    void drawQuadrilateral(Eigen::Vector3d point1, Eigen::Vector3d point2,
                           Eigen::Vector3d point3, Eigen::Vector3d point4,
                           double lineWidth);

    int drawText (int x, int y, const QString &string) const;
    int drawText (const QPoint& pos, const QString &string) const;
    int drawText (const Vector3d & pos, const QString &string) const;

    void begin(QTextStream *output, Vector3d planeNormalVector);
    void end();

  private:
    POVPainterPrivate * const d;

  };

  class A_EXPORT POVPainterDevice : public PainterDevice
  {
  public:
    POVPainterDevice(const QString& filename, const GLWidget* glwidget);
    ~POVPainterDevice();

    void initializePOV();
    void render();

    Painter *painter() const { return m_painter; }
    Camera *camera() const { return m_glwidget->camera(); }
    bool isSelected(const Primitive *p) const { return m_glwidget->isSelected(p); }
    double radius(const Primitive *p) const { return m_glwidget->radius(p); }
    const Molecule *molecule() const { return m_glwidget->molecule(); }
    Color *colorMap() const { return m_glwidget->colorMap(); }

    int width() { return m_glwidget->width(); }
    int height() { return m_glwidget->height(); }

  private:
    const GLWidget *m_glwidget;
    QList<Engine *> m_engines;
    POVPainter *m_painter;
    QFile *m_file;
    QTextStream *m_output;
  };

} // End namespace Avogadro

#endif
