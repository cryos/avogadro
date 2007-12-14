/**********************************************************************
  GLPainter - drawing spheres, cylinders and text in a GLWidget

  Copyright (C) 2007 Benoit Jacob
  Copyright (C) 2007 Donald Ephraim Curtis
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

#ifndef __GLPAINTER_H
#define __GLPAINTER_H

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
      void setQuality (int quality);

      /**
       * @return the current global quality setting.
       */
      int quality() const;

      /**
       * Uses the primitive to set the type and name if the Paint Device supports it.
       * @param primitive the primitive about to be drawn.
       */
      void setName (const Primitive *primitive);

      /**
       * Sets the primitive type and id.
       * @param type the primitive type about to be drawn.
       * @param id the primitive id.
       */
      void setName (Primitive::Type type, int id);

      /**
       * Set the color to paint the OpenGL primitives with.
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
       * Draws a sphere, leaving the Painter choose the appropriate detail level based on the
       * apparent radius (ratio of radius over distance) and the global quality setting.
       * @param center the position of the center of the sphere.
       * @param radius the radius of the sphere.
       */
      void drawSphere (const Eigen::Vector3d & center, double radius);

      /**
       * Draws a cylinder, leaving the Painter choose the appropriate detail level based on the
       * apparent radius (ratio of radius over distance) and the global quality setting.
       * @param end1 the position of the first end of the cylinder.
       * @param end2 the position of the second end of the cylinder.
       * @param radius the radius, i.e. half-width of the cylinder.
       */
      void drawCylinder (const Eigen::Vector3d &end1, const Eigen::Vector3d &end2,
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
      void drawMultiCylinder (const Eigen::Vector3d &end1, const Eigen::Vector3d &end2,
                               double radius, int order, double shift);

    /**
     * Draws a cone between the tip and the base with the base radius given.
     * @param base the position of the base of the cone.
     * @param tip the position of the tip of the cone.
     * @param radius the radius of the base of the cone.
     */
      void drawCone(const Eigen::Vector3d &base, const Eigen::Vector3d &tip,
                    double radius);

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
       * @sa begin(), drawText(const Eigen::Vector3d &, const QString &) const,
       *     drawText(const QPoint &, const QString &) const
       */
      int drawText (int x, int y, const QString &string) const;

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
       * @sa begin(), drawText(const Eigen::Vector3d &, const QString &) const,
       *     drawText(int, int, const QString &) const
       */
      int drawText (const QPoint& pos, const QString &string) const;

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
       * @sa begin(), drawText(const QPoint&, const QString &) const,
       *     drawText(int, int, const QString &) const
       */
      int drawText (const Eigen::Vector3d & pos, const QString &string) const;

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

    private:
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
  };
} // end namespace Avogadro

#endif
