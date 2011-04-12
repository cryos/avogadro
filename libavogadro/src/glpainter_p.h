/**********************************************************************
  GLPainter - drawing spheres, cylinders and text in a GLWidget

  Copyright (C) 2007 Benoit Jacob
  Copyright (C) 2007 Donald Ephraim Curtis
  Copyright (C) 2007-2009 Marcus D. Hanwell
  Copyright (C) 2010 Konstantin Tokarev
  Copyright (C) 2011 David C. Lonie

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.openmolecules.net/>

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

#ifndef GLPAINTER_H
#define GLPAINTER_H

#include <avogadro/global.h>
#include <avogadro/painter.h>

namespace Avogadro
{

  /**
   * @class GLPainter glpainter.h
   * @brief Implementation of the Painter class using OpenGL.
   * @author Marcus D. Hanwell
   *
   * This class implements the base Painter class using OpenGL. It is intended
   * to be used with the GLWidget to render molecules and other constructs to
   * an OpenGL context.
   *
   * @sa Painter
   */
  class GLWidget;
  class Color3f;
  class GLPainterPrivate;
  class GLPainter : public Painter
  {
    friend class GLWidget;
  public:

    /**
     * Constructor.
     * @param quality defaults to -1, valid range from 0 to 4.
     */
    GLPainter (int quality=-1);

    /**
     * Destructor.
     */
    ~GLPainter();

    /**
     * Sets the global quality setting. This influences the detail level of the
     * geometric objects (spheres and cylinders). Values range from 0 to
     * PAINTER_GLOBAL_QUALITY_SETTINGS-1.
     */
    void setQuality(int quality);

    /**
     * @return the current global quality setting.
     */
    int quality() const;

    /**
     * Uses the primitive to set the type and name if the Paint Device supports it.
     * @param primitive the primitive about to be drawn.
     */
    void setName(const Primitive *primitive);

    /**
     * Sets the primitive type and id.
     * @param type the primitive type about to be drawn.
     * @param id the primitive id.
     */
    void setName(Primitive::Type type, int id);

    /**
     * Set the color to paint the OpenGL primitives with.
     * @param color the color to be used for painting.
     */
    void setColor(const Color *color);

    /**
     * Set the color to paint the OpenGL primitives with.
     * @param color the color to be used for painting.
     */
    void setColor(const QColor *color);

    /**
     * Set the color to paint elements with where 0.0 is the minimum and 1.0
     * is the maximum.
     * @param red component of the color.
     * @param green component of the color.
     * @param blue component of the color.
     * @param alpha component of the color.
     */
    void setColor(float red, float green, float blue, float alpha = 1.0);

    /**
     * Set the color to paint elements by its name 
     * @param name name of the color to be used
     */    
    void setColor(QString name);

    /**
     * Draws a sphere, leaving the Painter choose the appropriate detail level based on the
     * apparent radius (ratio of radius over distance) and the global quality setting.
     * @param center the position of the center of the sphere.
     * @param radius the radius of the sphere.
     */
    void drawSphere(const Eigen::Vector3d &center, double radius);

    /**
     * Draws a cylinder, leaving the Painter choose the appropriate detail level based on the
     * apparent radius (ratio of radius over distance) and the global quality setting.
     * @param end1 the position of the first end of the cylinder.
     * @param end2 the position of the second end of the cylinder.
     * @param radius the radius, i.e. half-width of the cylinder.
     */
    void drawCylinder(const Eigen::Vector3d &end1, const Eigen::Vector3d &end2,
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
    void drawMultiCylinder(const Eigen::Vector3d &end1, const Eigen::Vector3d &end2,
                           double radius, int order, double shift);

    /**
     * Draws a cone between the tip and the base with the base radius given.
     * @param base the position of the base of the cone.
     * @param baseRadius the radius of the base of the cone.
     * @param cap the position of the tip of the cone.
     * @param capRadius the radius of the base of the cone.
     * @note The capRadius is currently ignored
     */
    void drawCone(const Eigen::Vector3d &base, const Eigen::Vector3d &cap,
                  double baseRadius, double capRadius = 0.0);

    /**
     * Draws a GL line between the given points of the given width.
     * @param start the position of the start of the line.
     * @param end the position of the end of the line.
     * @param lineWidth the width of the GL line.
     */
    void drawLine(const Eigen::Vector3d &start, const Eigen::Vector3d &end,
                  double lineWidth);

    /**
     * Draws a multiple GL line between the given points. This function is the
     * GL line equivalent to the drawMultiCylinder function and performs the
     * same basic operations using simpler and quicker GL lines.
     * @param start the position of the start of the line.
     * @param end the position of the end of the line.
     * @param lineWidth the width of the GL line.
     * @param order the order of the bond, e.g. 2 for a double bond.
     * @param stipple The GL stipple parameter for the bond, can be used to
     * draw aromatic bonds etc.
     * sa drawMultiCylinder
     */
    void drawMultiLine(const Eigen::Vector3d &start, const Eigen::Vector3d &end,
                       double lineWidth, int order, short stipple);

    /**
     * Draws a triangle with vertives on the three given points. This function
     * calculates the normal of the triangle and corrects the winding order to
     * ensure the front face is facing the camera.
     * @param p1 first triangle vertex.
     * @param p2 second triangle vertex.
     * @param p3 third triangle vertex.
     */
    void drawTriangle(const Eigen::Vector3d &p1, const Eigen::Vector3d &p2,
                      const Eigen::Vector3d &p3);

    /**
     * Draws a triangle with vertives on the three given points using the
     * given normal. This function corrects the triangle's winding order.
     * @param p1 first triangle vertex.
     * @param p2 second triangle vertex.
     * @param p3 third triangle vertex.
     * @param n the normal of the triangle.
     */
    void drawTriangle(const Eigen::Vector3d &p1, const Eigen::Vector3d &p2,
                      const Eigen::Vector3d &p3, const Eigen::Vector3d &n);

    /**
     * Draw a cubic B-spline between the given points using GL NURBS.
     * @param pts QVector containing the points to draw the cubic B-spline
     * along.
     * @param radius the radius of the cubic B-spline.
     */
    void drawSpline(const QVector<Eigen::Vector3d>& pts, double radius);

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
    void drawShadedSector(const Eigen::Vector3d & origin,
                          const Eigen::Vector3d & direction1,
                          const Eigen::Vector3d & direction2, double radius,
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
    void drawArc(const Eigen::Vector3d & origin, const Eigen::Vector3d &direction1,
                 const Eigen::Vector3d & direction2,
                 double radius, double lineWidth, bool alternateAngle = false);

    /**
     * Draws a solid two dimensional quadrilateral in three dimensional space.
     *
     * @param point1 the first of the four corners of the quadrilateral.
     * @param point2 the second of the four corners of the quadrilateral.
     * @param point3 the third of the four corners of the quadrilateral.
     * @param point4 the last of the four corners of the quadrilateral.
     */
    void drawShadedQuadrilateral(const Eigen::Vector3d & point1,
                                 const Eigen::Vector3d & point2,
                                 const Eigen::Vector3d & point3,
                                 const Eigen::Vector3d & point4);

    /**
     * Draws the outline of a two dimensional quadrilateral in three dimensional space.
     *
     * @param point1 the first of the four corners of the quadrilateral.
     * @param point2 the second of the four corners of the quadrilateral.
     * @param point3 the third of the four corners of the quadrilateral.
     * @param point4 the last of the four corners of the quadrilateral.
     * @param lineWidth the thickness of the line the quadrilateral will be drawn with.
     */
    void drawQuadrilateral(const Eigen::Vector3d & point1,
                           const Eigen::Vector3d & point2,
                           const Eigen::Vector3d & point3,
                           const Eigen::Vector3d & point4,
                           double lineWidth);

    /**
     * Draws the outline of a pentagon in three dimensional space.
     *
     * @param point1 the first of the five corners of the pentagon.
     * @param point2 the second of the five corners of the pentagon.
     * @param point3 the third of the five corners of the pentagon.
     * @param point4 the fourth of the five corners of the pentagon.
     * @param point5 the last of the five corners of the pentagon.
     * @param lineWidth the thickness of the line the pentagon will be drawn with.
     */
    void drawPentagon(const Eigen::Vector3d & point1,
                      const Eigen::Vector3d & point2,
                      const Eigen::Vector3d & point3,
                      const Eigen::Vector3d & point4,
                      const Eigen::Vector3d & point5,
                      const double lineWidth);

    /**
     * Draws the outline of a hexagon in three dimensional space.
     *
     * @param point1 the first of the six corners of the hexagon.
     * @param point2 the second of the six corners of the hexagon.
     * @param point3 the third of the six corners of the hexagon.
     * @param point4 the fourth of the six corners of the hexagon.
     * @param point4 the fifth of the six corners of the hexagon.
     * @param point5 the last of the six corners of the hexagon.
     * @param lineWidth the thickness of the line the hexagon will be drawn with.
     */
    void drawHexagon(const Eigen::Vector3d & point1,
                     const Eigen::Vector3d & point2,
                     const Eigen::Vector3d & point3,
                     const Eigen::Vector3d & point4,
                     const Eigen::Vector3d & point5,
                     const Eigen::Vector3d & point6,
                     const double lineWidth);

    /**
     * Draws a continuous mesh of triangles.
     * @param mesh the mesh to be drawn.
     * @param mode the mode to use. 0 = filled, 1 = lines and 2 = points.
     */
    void drawMesh(const Mesh & mesh, int mode = 0);

    /**
     * Draws a continuous mesh of triangles and respects the colors stored.
     * @param mesh the mesh to be drawn.
     * @param mode the mode to use. 0 = filled, 1 = lines and 2 = points.
     */
    void drawColorMesh(const Mesh & mesh, int mode = 0);

    /**
     * Draws text at a given window position, on top of the scene.
     * @note Calls to drawText methods must be enclosed between begin() and end().
     * @note Text is rendered as a transparent object, and should therefore be rendered after
     *       the opaque objects.
     * @param x,y the window coordinates of the top-left corner of the text to render,
     *        (0, 0) is the top-left corner of the window.
     * @param string the string to render. All character encodings are allowed but
     *               superposed characters are not supported yet. For accented letters,
     *               use a character giving the whole accented letter, not a separate
     *               character for the accent.
     * @sa begin(), drawText(const Eigen::Vector3d &, const QString &),
     *     drawText(const QPoint &, const QString &)
     */
    int drawText (int x, int y, const QString &string);

    /**
     * Draws text at a given window position, on top of the scene.
     * @note Calls to drawText methods must be enclosed between begin() and endText().
     * @note Text is rendered as a transparent object, and should therefore be rendered after
     *       the opaque objects.
     * @param pos the window coordinates of the top-left corner of the text to render,
     *        (0, 0) is the top-left corner of the window.
     * @param string the string to render. All character encodings are allowed but
     *               superposed characters are not supported yet. For accented letters,
     *               use a character giving the whole accented letter, not a separate
     *               character for the accent.
     * @sa begin(), drawText(const Eigen::Vector3d &, const QString &),
     *     drawText(int, int, const QString &) const
     */
    int drawText(const QPoint& pos, const QString &string);

    /**
     * Draws text at a given scene position, inside the scene.
     * @note Calls to drawText methods must be enclosed between begin() and endText().
     * @note Text is rendered as a transparent object, and should therefore be rendered after
     *       the opaque objects.
     * @param pos the scene coordinates of the top-left corner of the text to render.
     * @param string The string to render. All character encodings are allowed but
     *               superposed characters are not supported yet. For accented letters,
     *               use a character giving the whole accented letter, not a separate
     *               character for the accent.
     * @sa begin(), drawText(const QPoint&, const QString &),
     *     drawText(int, int, const QString &)
     */
    int drawText(const Eigen::Vector3d & pos, const QString &string);

    int drawText(const Eigen::Vector3d &pos, const QString &string, const QFont &font);

    /**
     * Placeholder to draw a box.
     * @param corner1 First corner of the box.
     * @param corner2 Second corner of the box.
     * @todo Implement this primitive.
     */
    void drawBox(const Eigen::Vector3d &corner1,
                 const Eigen::Vector3d &corner2);

    /**
     * @overload
     *
     * Draws the outline of a parallelpiped at \a offset with three
     * vectors \a v1, \a v2, and \a v3 defining the edges.
     *
     * @verbatim
       6------8  c1 = origin
      /:     /|  c2 = origin + v1
     / :    / |  c3 = origin + v2
    /  4---/--7  c4 = origin + v3
   /  /   /  /   c5 = origin + v1 + v2
  3------5  /    c6 = origin + v2 + v3
  | /    | /     c7 = origin + v1 + v3
  |/     |/      c8 = origin + v1 + v2 + v3
  1------2
@endverbatim
     *
     * @param offset Corner of the box.
     * @param v1 Edge of box, pointing relative to \a offset.
     * @param v2 Edge of box, pointing relative to \a offset.
     * @param v3 Edge of box, pointing relative to \a offset.
     * @param linewidth The width of the line.
     */
    void drawBoxEdges(const Eigen::Vector3d &offset,
                      const Eigen::Vector3d &v1,
                      const Eigen::Vector3d &v2,
                      const Eigen::Vector3d &v3,
                      const double linewidth);

    /**
     * Draws the outline of a box with the given corners.
     * @verbatim
       6------8
      /:     /|
     / :    / |
    /  4---/--7
   /  /   /  /
  3------5  /
  | /    | /
  |/     |/
  1------2
@endverbatim
     *
     * @warning The default implementaion of this function simply
     * calls drawLine repeatedly to draw the specified shape. This may
     * be very inefficent on certain paint devices and should be
     * reimplemented in such cases.
     *
     * @param c1 Corner
     * @param c2 Corner
     * @param c3 Corner
     * @param c4 Corner
     * @param c5 Corner
     * @param c6 Corner
     * @param c7 Corner
     * @param c8 Corner
     * @param linewidth The width of the line.
     */
    void drawBoxEdges(const Eigen::Vector3d &c1,
                      const Eigen::Vector3d &c2,
                      const Eigen::Vector3d &c3,
                      const Eigen::Vector3d &c4,
                      const Eigen::Vector3d &c5,
                      const Eigen::Vector3d &c6,
                      const Eigen::Vector3d &c7,
                      const Eigen::Vector3d &c8,
                      const double linewidth);

    /**
     * Placeholder to draw a torus.
     * @param pos Position of the center of the torus.
     * @param majorRadius Major radius of the torus.
     * @param minorRadius Minor radius of the torus.
     * @todo Implement this primitive.
     */
    void drawTorus(const Eigen::Vector3d &pos,
                   double majorRadius, double minorRadius);

    /**
     * Placeholder to draw an ellipsoid.
     * @param pos Position of the center of the ellipsoid.
     * @param matrix Linear transformation matrix for scaling and rotation.
     * @todo Implement this primitive.
     */
    void drawEllipsoid(const Eigen::Vector3d &position,
                               const Eigen::Matrix3d &matrix);

    /**
     * Set the Painter up for painting onto a GLWidget, should be called
     * before any painting.
     */
    void begin(GLWidget *widget);

    /**
     * End painting, should be called when all painting is complete.
     */
    void end();

    /**
     * @return true if the Painter is active.
     */
    bool isActive();

    /**
     * @return true if the Painter is being shared between multiple GLWidgets.
     */
    bool isShared();

    /**
     * @return the default quality level of the Painter.
     */
    static int defaultQuality();

    /**
     * @return the maximum quality level of the Painter.
     */
    static int maxQuality();

    /**
     * Set to true to turn dynamic object scaling on, false for off.
     */
    void setDynamicScaling(bool scaling);

  protected:
    GLPainterPrivate * const d;

    bool m_dynamicScaling;

    /**
     * Increment the number of widgets the Painter is being shared by.
     */
    void incrementShare();

    /**
     * Decrement the number of widgets the Painter is being shared by.
     */
    void decrementShare();

    /**
     * Push the GL name and type, called internally before GL objects are
     * painted on the GLWidget.
     */
    void pushName();

    /**
     * Pop the GL name and type, called internally after GL objects are
     * painted on the GLWidget.
     */
    void popName();

    /**
     * Reset the GL name and type, called internally in popName() and also
     * whenever frustum culling determines that a GL object must not be
     * painted.
     */
    void resetName();

    /**
     * Sets this color to be the one used by OpenGL for rendering
     * when lighting is disabled.
     */
    inline void apply(const Color3f &color);

    /**
     * Applies nice OpenGL materials using this color as the
     * diffuse color while using different shades for the ambient and
     * specular colors. This is only useful if lighting is enabled.
     */
    inline void applyAsMaterials(const Color3f &color, float alpha = 1.0);
  };
} // end namespace Avogadro

#endif
