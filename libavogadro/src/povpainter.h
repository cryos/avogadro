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
#include <avogadro/painter.h>
#include <eigen/vector.h>
#include <QObject>
#include <QPoint>
#include <QList>

#include "glwidget.h"
#include "camera.h"

class QFile;
class QTextStream;

using namespace Eigen;

namespace Avogadro
{
  class POVPainterPrivate;
  class A_EXPORT POVPainter : public Painter
  {
  public:
    POVPainter ();
    ~POVPainter();

    void setName (const Primitive *primitive);
    void setName (Primitive::Type type, int id);
    void setColor (const Color *color);
    void setColor (float red, float green, float blue, float alpha = 1.0);
    void setPlaneNormal (Vector3d planeNormalVector);

    void drawSphere (const Vector3d & center, double radius);

    void drawCylinder (const Vector3d &end1, const Vector3d &end2,
                        double radius);

    void drawMultiCylinder (const Vector3d &end1, const Vector3d &end2,
                            double radius, int order, double shift);

    void drawLine(const Eigen::Vector3d &, const Eigen::Vector3d &,
                  double) { }

    void drawSpline(const QList<Eigen::Vector3d>&, double) { }

    /** Draws a shaded sector of a circle.  The sector is defined by three vectors,
     * the center of the circle, and two vectors that define the lines going out
     * from the centre of the circle to the circumference of the circle.  The
     * actual points on the circumference are found using these two vectors and
     * the radius of the circle.
     *
     * @param origin The center of the circle this sector is a portion of.
     * @param direction1 A vector defining the line the first point will lie on.
     * @param direction2 A vector defining the line the second point will lie on.
     * @param radius The radius of the circle this sector is a portion of.
     */
    void drawShadedSector(Eigen::Vector3d origin, Eigen::Vector3d direction1,
                          Eigen::Vector3d direction2, double radius);

    /** Draws an arc.  The arc is defined by three vectors, the center of the circle,
     * and two vectors that define the lines going out from the center of the
     * circle to the circumference of the circle.  The actual points on the
     * circumference are found using these two vectors and the radius of the circle.
     *
     * @param origin The center of the circle whose circumference this arc is a portion of.
     * @param direction1 A vector defining the line the start of the arc will lie on.
     * @param direction2 A vector defining the line the end of the arc will lie on.
     * @param radius The radius of the circle whose circumference this arc is a portion of.
     * @param lineWidth The thickness of the line the arc will be drawn with.
     */
    void drawArc(Eigen::Vector3d origin, Eigen::Vector3d direction1, Eigen::Vector3d direction2,
                 double radius, double lineWidth);

    /** Draws a solid two dimensional quadrilateral in three dimensional space.
     *
     * @param point1 The first of the four corners of the quadrilateral.
     * @param point2 The second of the four corners of the quadrilateral.
     * @param point3 The third of the four corners of the quadrilateral.
     * @param point4 The last of the four corners of the quadrilateral.
     */
    void drawShadedQuadrilateral(Eigen::Vector3d point1, Eigen::Vector3d point2,
                                 Eigen::Vector3d point3, Eigen::Vector3d point4);

    /** Draws the outline of a two dimensional quadrilateral in three dimensional space.
     *
     * @param point1 The first of the four corners of the quadrilateral.
     * @param point2 The second of the four corners of the quadrilateral.
     * @param point3 The third of the four corners of the quadrilateral.
     * @param point4 The last of the four corners of the quadrilateral.
     * @param lineWidth The thickness of the line the quadrilateral will be drawn with.
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
