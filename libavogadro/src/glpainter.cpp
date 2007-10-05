/**********************************************************************
  Painter - drawing spheres, cylinders and text in a GLWidget

  Copyright (C) 2007 Benoit Jacob
  Copyright (C) 2007 Donald Ephraim Curtis

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

#include "glpainter.h"

#include <avogadro/glwidget.h>
#include <avogadro/primitive.h>
#include "textrenderer.h"
#include "cylinder.h"
#include "sphere.h"
#include "color.h"

namespace Avogadro
{

  const int      PAINTER_GLOBAL_QUALITY_SETTINGS       = 5;
  const int      DEFAULT_GLOBAL_QUALITY_SETTING        = PAINTER_GLOBAL_QUALITY_SETTINGS - 3;
  const int      PAINTER_DETAIL_LEVELS                 = 10;
  // Sphere detail level array. Each row is a detail level.
  // The first column is the sphere detail level at the furthest
  // point and the last column is the detail level at the closest
  // point.
  const int      PAINTER_SPHERES_LEVELS_ARRAY[5][10]
  =
    { {0, 0, 1, 1, 2, 2, 3, 3, 4, 4},
      {0, 1, 2, 3, 4, 4, 5, 5, 6, 6},
      {1, 2, 3, 4, 5, 6, 7, 8, 9, 9},
      {1, 2, 3, 4, 6, 7, 8, 9, 11, 12},
      {2, 3, 4, 5, 7, 9, 12, 15, 18, 22}
    };
  const double   PAINTER_SPHERES_LIMIT_MIN_LEVEL       = 0.005;
  const double   PAINTER_SPHERES_LIMIT_MAX_LEVEL       = 0.15;

  // Cylinder detail level array. Each row is a detail level.
  // The first column is the cylinder detail level at the furthest
  // point and the last column is the detail level at the closest
  // point.
  const int      PAINTER_CYLINDERS_LEVELS_ARRAY[5][10]
  =
    { {0, 3, 5, 5, 8, 8, 12, 12, 16, 16},
      {0, 4, 6, 9, 12, 12, 16, 16, 20, 20},
      {0, 4, 6, 10, 14, 18, 22, 26, 32, 40},
      {0, 4, 6, 12, 16, 20, 24, 28, 34, 42},
      {0, 5, 10, 15, 20, 25, 30, 35, 40, 45}
    };
  const double   PAINTER_CYLINDERS_LIMIT_MIN_LEVEL     = 0.001;
  const double   PAINTER_CYLINDERS_LIMIT_MAX_LEVEL     = 0.03;
  const int      PAINTER_MAX_DETAIL_LEVEL = PAINTER_DETAIL_LEVELS - 1;
  const double   PAINTER_SPHERES_SQRT_LIMIT_MIN_LEVEL
  = sqrt ( PAINTER_SPHERES_LIMIT_MIN_LEVEL );
  const double   PAINTER_SPHERES_SQRT_LIMIT_MAX_LEVEL
  = sqrt ( PAINTER_SPHERES_LIMIT_MAX_LEVEL );
  const double   PAINTER_SPHERES_DETAIL_COEFF
  = static_cast<double> ( PAINTER_MAX_DETAIL_LEVEL - 1 )
    / ( PAINTER_SPHERES_SQRT_LIMIT_MAX_LEVEL - PAINTER_SPHERES_SQRT_LIMIT_MIN_LEVEL );
  const double   PAINTER_CYLINDERS_SQRT_LIMIT_MIN_LEVEL
  = sqrt ( PAINTER_CYLINDERS_LIMIT_MIN_LEVEL );
  const double   PAINTER_CYLINDERS_SQRT_LIMIT_MAX_LEVEL
  = sqrt ( PAINTER_CYLINDERS_LIMIT_MAX_LEVEL );
  const double   PAINTER_CYLINDERS_DETAIL_COEFF
  = static_cast<double> ( PAINTER_MAX_DETAIL_LEVEL - 1 )
    / ( PAINTER_CYLINDERS_SQRT_LIMIT_MAX_LEVEL - PAINTER_CYLINDERS_SQRT_LIMIT_MIN_LEVEL );
  const double   PAINTER_FRUSTUM_CULL_TRESHOLD = -0.8;

  class GLPainterPrivate
  {
    public:
      GLPainterPrivate() : widget ( 0 ), newQuality(-1), quality ( 0 ), overflow(0),
          spheres ( 0 ), cylinders ( 0 ),
          textRenderer ( new TextRenderer ), initialized ( false ), sharing ( 0 ),
          type(Primitive::OtherType), id ( -1 ), color(0)  {};
      ~GLPainterPrivate()
      {
        deleteObjects();
        delete textRenderer;
      }

      GLWidget *widget;

      int newQuality;
      int quality;
      int overflow;

      /** array of pointers to Spheres. You might ask, why not have
       * a plain array of Spheres. The idea is that more than one global detail level
       * may use a given sphere detail level. It is therefore interesting to be able
       * to share that sphere, instead of having redundant spheres in memory.
       */
      Sphere **spheres;
      /** array of pointers to Cylinders. You might ask, why not have
       * a plain array of Cylinders. The idea is that more than one global detail level
       * may use a given cylinder detail level. It is therefore interesting to be able
       * to share that cylinder, instead of having redundant cylinder in memory.
       */
      Cylinder **cylinders;

      TextRenderer *textRenderer;

      bool initialized;

      void deleteObjects();
      void createObjects();

      inline bool isValid();

      /**
       * Painters can be shared, we must keep track of this.
       */
      int sharing;

      // The primitive type and id of the current object
      Primitive::Type type;
      int id;
      Color color;
  };

  inline bool GLPainterPrivate::isValid()
  {
    if(!widget)
    {
      qWarning("GLPainter not active.");
      return false;
    }

    if(newQuality != -1)
    {
      qDebug() << "updateObjects()";
      if(newQuality != quality)
      {
        deleteObjects();
        quality = newQuality;
        createObjects();
      }
      newQuality = -1;
    }
    else if(!initialized)
    {
      qDebug() << "createObjects()";
      createObjects();
      initialized = true;
    }
    return true;
  }

  void GLPainterPrivate::deleteObjects()
  {
    int level, lastLevel, n;
    // delete the spheres. One has to be wary that more than one sphere
    // pointer may have the same value. One wants to avoid deleting twice the same sphere.
    if ( spheres )
    {
      lastLevel = -1;
      for ( n = 0; n < PAINTER_DETAIL_LEVELS; n++ )
      {
        level = PAINTER_SPHERES_LEVELS_ARRAY[quality][n];
        if ( level != lastLevel )
        {
          lastLevel = level;
          if ( spheres[n] )
          {
            delete spheres[n];
            spheres[n] = 0;
          }
        }
      }
      delete[] spheres;
      spheres = 0;
    }

    // delete the cylinders. One has to be wary that more than one cylinder
    // pointer may have the same value. One wants to avoid deleting twice the same cylinder.
    if ( cylinders )
    {
      lastLevel = -1;
      for ( n = 0; n < PAINTER_DETAIL_LEVELS; n++ )
      {
        level = PAINTER_CYLINDERS_LEVELS_ARRAY[quality][n];
        if ( level != lastLevel )
        {
          lastLevel = level;
          if ( cylinders[n] )
          {
            delete cylinders[n];
            cylinders[n] = 0;
          }
        }
      }
      delete[] cylinders;
      cylinders = 0;
    }
  }

  void GLPainterPrivate::createObjects()
  {
    // create the spheres. More than one sphere detail level may have the same value.
    // in that case we want to reuse the corresponding sphere by just copying the pointer,
    // instead of creating redundant spheres.
    if ( spheres == 0 )
    {
      spheres = new Sphere*[PAINTER_DETAIL_LEVELS];
      int level, lastLevel;
      lastLevel = PAINTER_SPHERES_LEVELS_ARRAY[quality][0];
      spheres[0] = new Sphere ( lastLevel );
      for ( int n = 1; n < PAINTER_DETAIL_LEVELS; n++ )
      {
        level = PAINTER_SPHERES_LEVELS_ARRAY[quality][n];
        if ( level == lastLevel )
        {
          spheres[n] = spheres[n-1];
        }
        else
        {
          lastLevel = level;
          spheres[n] = new Sphere ( level );
        }
      }
    }

    // create the cylinders. More than one cylinder detail level may have the same value.
    // in that case we want to reuse the corresponding cylinder by just copying the pointer,
    // instead of creating redundant cylinders.
    if ( cylinders == 0 )
    {
      cylinders = new Cylinder*[PAINTER_DETAIL_LEVELS];
      int level, lastLevel;
      lastLevel = PAINTER_SPHERES_LEVELS_ARRAY[quality][0];
      cylinders[0] = new Cylinder ( lastLevel );
      for ( int n = 1; n < PAINTER_DETAIL_LEVELS; n++ )
      {
        level = PAINTER_CYLINDERS_LEVELS_ARRAY[quality][n];
        if ( level == lastLevel )
        {
          cylinders[n] = cylinders[n-1];
        }
        else
        {
          lastLevel = level;
          cylinders[n] = new Cylinder ( level );
        }
      }
    }
  }

  GLPainter::GLPainter( int quality ) : d ( new GLPainterPrivate )
  {
    if ( quality < 0 || quality >= PAINTER_MAX_DETAIL_LEVEL )
    {
      quality = DEFAULT_GLOBAL_QUALITY_SETTING;
    }
    d->quality = quality;
  }

  GLPainter::~GLPainter()
  {
    delete d;
  }

  void GLPainter::setQuality ( int quality )
  {
    assert ( quality >= 0 && quality < PAINTER_GLOBAL_QUALITY_SETTINGS );
    d->newQuality = quality;
  }

  int GLPainter::quality() const
  {
    return d->quality;
  }

  void GLPainter::setName ( const Primitive *primitive )
  {
    d->type = primitive->type();
    if (d->type == Primitive::AtomType)
    {
      d->id = static_cast<const Atom *>(primitive)->GetIdx();
    }
    else if (d->type == Primitive::BondType)
    {
      d->id = static_cast<const Bond *>(primitive)->GetIdx();
    }
  }

  void GLPainter::setName ( Primitive::Type type, int id )
  {
    d->type = type;
    d->id = id;
  }

  void GLPainter::setColor ( const Color *color )
  {
    d->color = *color;
  }

  void GLPainter::setColor ( float red, float green, float blue, float alpha )
  {
    d->color.set(red, green, blue, alpha);
  }

  void GLPainter::drawSphere ( const Eigen::Vector3d & center, double radius )
  {
    if(!d->isValid()) { return; }

    Eigen::Vector3d transformedCenter = d->widget->camera()->modelview() * center;
    double distance = transformedCenter.norm();

    // perform a rough form of frustum culling
    double dot = transformedCenter.z() / distance;
    if ( dot > PAINTER_FRUSTUM_CULL_TRESHOLD ) return;

    double apparentRadius = radius / distance;

    int detailLevel = 1 + static_cast<int> ( floor (
                        PAINTER_SPHERES_DETAIL_COEFF * ( sqrt ( apparentRadius ) - PAINTER_SPHERES_SQRT_LIMIT_MIN_LEVEL )
                      ) );
    if ( detailLevel < 0 )
    {
      detailLevel = 0;
    }
    if ( detailLevel > PAINTER_MAX_DETAIL_LEVEL )
    {
      detailLevel = PAINTER_MAX_DETAIL_LEVEL;
    }

    d->color.applyAsMaterials();
    pushName();
    d->spheres[detailLevel]->draw ( center, radius );
    popName();
  }

  void GLPainter::drawCylinder ( const Eigen::Vector3d &end1, const Eigen::Vector3d &end2,
                               double radius )
  {
    if(!d->isValid()) { return; }

    Eigen::Vector3d transformedEnd1 = d->widget->camera()->modelview() * end1;
    double distance = transformedEnd1.norm();

    // perform a rough form of frustum culling
    double dot = transformedEnd1.z() / distance;
    if ( dot > PAINTER_FRUSTUM_CULL_TRESHOLD ) return;

    double apparentRadius = radius / distance;
    int detailLevel = 1 + static_cast<int> ( floor (
                        PAINTER_CYLINDERS_DETAIL_COEFF
                        * ( sqrt ( apparentRadius ) - PAINTER_CYLINDERS_SQRT_LIMIT_MIN_LEVEL )
                      ) );
    if ( detailLevel < 0 )
    {
      detailLevel = 0;
    }
    if ( detailLevel > PAINTER_MAX_DETAIL_LEVEL )
    {
      detailLevel = PAINTER_MAX_DETAIL_LEVEL;
    }

    d->color.applyAsMaterials();
    pushName();
    d->cylinders[detailLevel]->draw ( end1, end2, radius );
    popName();
  }

  void GLPainter::drawMultiCylinder ( const Eigen::Vector3d &end1, const Eigen::Vector3d &end2,
                                    double radius, int order, double shift )
  {
    if(!d->isValid()) { return; }

    Eigen::Vector3d transformedEnd1 = d->widget->camera()->modelview() * end1;
    double distance = transformedEnd1.norm();

    // perform a rough form of frustum culling
    double dot = transformedEnd1.z() / distance;
    if ( dot > PAINTER_FRUSTUM_CULL_TRESHOLD ) return;

    double apparentRadius = radius / distance;
    int detailLevel = 1 + static_cast<int> ( floor (
                        PAINTER_CYLINDERS_DETAIL_COEFF
                        * ( sqrt ( apparentRadius ) - PAINTER_CYLINDERS_SQRT_LIMIT_MIN_LEVEL )
                      ) );
    if ( detailLevel < 0 )
    {
      detailLevel = 0;
    }
    if ( detailLevel > PAINTER_MAX_DETAIL_LEVEL )
    {
      detailLevel = PAINTER_MAX_DETAIL_LEVEL;
    }

    d->color.applyAsMaterials();
    pushName();
    d->cylinders[detailLevel]->drawMulti ( end1, end2, radius, order,
                                           shift, d->widget->normalVector() );
    popName();
  }

  void GLPainter::drawLine(const Eigen::Vector3d &start, const Eigen::Vector3d &end,
                           double lineWidth)
  {
    // Draw a line between two points of the specified thickness
    if(!d->isValid()) { return; }

    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glDisable(GL_LIGHTING);
    glDisable(GL_CULL_FACE);

    glLineWidth(lineWidth);
    glColor4f(d->color.red(), d->color.green(), d->color.blue(), d->color.alpha());

    // Draw the line
    glBegin(GL_LINE_STRIP);
    glVertex3d(start.x(), start.y(), start.z());
    glVertex3d(end.x(), end.y(), end.z());
    glEnd();

    glPopAttrib();
  }

  void GLPainter::drawSpline(const QList<Eigen::Vector3d>& pts, double lineWidth)
  {
    // Draw a spline between two points of the specified thickness
/*    if(!d->isValid()) { return; }

    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glDisable(GL_LIGHTING);
    glDisable(GL_CULL_FACE);
    glLineWidth(lineWidth);
    glColor4f(d->color.red(), d->color.green(), d->color.blue(), d->color.alpha());

    for (int i = 1; i < pts.size(); i++)
    {

      // Figure out our control points - right now it is a straight line...
      Eigen::Vector3d start = pts[i-1];
      Eigen::Vector3d end = pts[i];
      Eigen::Vector3d axis = end - start;
      Eigen::Vector3d startC = start + 0.25 * axis;
      Eigen::Vector3d endC = end - 0.25 * axis;
      Eigen::Vector3d p(0., 0., 0.);
      double a = 0., b = 0.;

      // Draw the spline
      glBegin(GL_LINE_STRIP);
      for (int i = 0; i <= 20; i++)
      {
        a = static_cast<double>(i) / 20.0;
        b = 1.0 - a;
        p = Eigen::Vector3d(start.x()*a*a*a + 3.0*startC.x()*a*a*b + 3.0*endC.x()*a*b*b + end.x()*b*b*b,
                          start.y()*a*a*a + 3.0*startC.y()*a*a*b + 3.0*endC.y()*a*b*b + end.y()*b*b*b,
                          start.z()*a*a*a + 3.0*startC.z()*a*a*b + 3.0*endC.z()*a*b*b + end.z()*b*b*b);
        glVertex3dv(p.array());
      }
      glEnd();

      glBegin(GL_POINTS);
      glVertex3dv(start.array());
      glVertex3dv(startC.array());
      glVertex3dv(endC.array());
      glVertex3dv(end.array());
      glEnd();
      glColor4f(0, 1, 0, 1);
      glBegin(GL_LINES);
      glVertex3dv(start.array());
      glVertex3dv(startC.array());
      glEnd();
      glColor4f(0, 0, 1, 1);
      glBegin(GL_LINES);
      glVertex3dv(end.array());
      glVertex3dv(endC.array());
      glEnd();
    }
    glPopAttrib(); */
  }

  void GLPainter::drawShadedSector(Eigen::Vector3d origin, Eigen::Vector3d direction1,
                                 Eigen::Vector3d direction2, double radius, bool alternateAngle)
  {
    assert( d->widget );

    // Get vectors representing the two lines out from the center of the circle.
    Eigen::Vector3d u = direction1 - origin;
    Eigen::Vector3d v = direction2 - origin;

    // Adjust the length of u and v to the radius given.
    u = (u / u.norm()) * radius;
    v = (v / v.norm()) * radius;

    // Angle between u and v.
    double uvAngle = acos(u.dot(v) / v.norm2()) * 180.0 / M_PI;

    // If angle is less than 1 (will be approximated to 0), attempting to draw
    // will crash, so return.
    if (abs((int)uvAngle) <= 1)
      return;

    // If alternateAngle is set, subtract this angle from 360 to get the alternate angle.
    if (alternateAngle) {
      uvAngle = 360.0 - (uvAngle > 0 ? uvAngle : -uvAngle);
    }

    // Vector perpindicular to both u and v.
    Eigen::Vector3d n = u.cross(v);

    Eigen::Vector3d x = Eigen::Vector3d(1, 0, 0);
    Eigen::Vector3d y = Eigen::Vector3d(0, 1, 0);

    if (n.norm() < 1e-16)
    {
      Eigen::Vector3d A = u.cross(x);
      Eigen::Vector3d B = u.cross(y);

      n = A.norm() >= B.norm() ? A : B;
    }

    n = n / n.norm();

    // Add the vectors to the origin vector to find the positions along the lines
    // of the two points the curve starts and ends at.
    direction1 = origin + u;
    direction2 = origin + v;

    // Calculate the points along the curve at each half-degree increment until we
    // reach the next line.
    Eigen::Vector3d points[720];
    for (int theta = 1; theta < (uvAngle * 2); theta++)
    {
      // Create a Matrix that represents a rotation about a vector perpindicular
      // to the plane.
      Eigen::Matrix3d rotMat;
      rotMat.loadRotation3((theta / 2 * (M_PI / 180.0)), n);

      // Apply the rotation Matrix to the vector to find the new point.
      if (alternateAngle) {
        rotMat.multiply(v, &points[theta-1]);
      } else {
        rotMat.multiply(u, &points[theta-1]);
      }
      points[theta-1] += origin;
      points[theta-1] = d->widget->camera()->modelview() * points[theta-1];
    }

    // Get vectors representing the points' positions in terms of the model view.
    origin = d->widget->camera()->modelview() * origin;
    direction1 = d->widget->camera()->modelview() * direction1;
    direction2 = d->widget->camera()->modelview() * direction2;

    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glPushMatrix();
    glLoadIdentity();
    glDisable(GL_LIGHTING);
    glDisable(GL_CULL_FACE);

    glColor4f(d->color.red(), d->color.green(), d->color.blue(), d->color.alpha());

    // Draw the transparent polygon that makes up the sector.
    glBegin(GL_TRIANGLE_FAN);
    glVertex3d(origin.x(), origin.y(), origin.z());
    if (alternateAngle)
    {
      glVertex3d(direction2.x(), direction2.y(), direction2.z());
      for (int i = 0; i < uvAngle*2 - 1; i++)
        glVertex3d(points[i].x(), points[i].y(), points[i].z());
      glVertex3d(direction1.x(), direction1.y(), direction1.z());
    }
    else
    {
      glVertex3d(direction1.x(), direction1.y(), direction1.z());
      for (int i = 0; i < uvAngle*2 - 1; i++)
        glVertex3d(points[i].x(), points[i].y(), points[i].z());
      glVertex3d(direction2.x(), direction2.y(), direction2.z());
    }
    glEnd();

    glPopMatrix();
    glPopAttrib();
  }

  void GLPainter::drawArc(Eigen::Vector3d origin, Eigen::Vector3d direction1,
                          Eigen::Vector3d direction2, double radius, double lineWidth,
                          bool alternateAngle)
  {
    assert( d->widget );

    // Get vectors representing the two lines out from the center of the circle.
    Eigen::Vector3d u = direction1 - origin;
    Eigen::Vector3d v = direction2 - origin;

    // Adjust the length of u and v to the radius given.
    u = (u / u.norm()) * radius;
    v = (v / v.norm()) * radius;

    // Angle between u and v.
    double uvAngle = acos(u.dot(v) / v.norm2()) * 180.0 / M_PI;

    // If angle is less than 1 (will be approximated to 0), attempting to draw
    // will crash, so return.
    if (abs((int)uvAngle) <= 1)
      return;

    // If alternateAngle is set, subtract this angle from 360 to get the alternate angle.
    if (alternateAngle) {
      uvAngle = 360.0 - (uvAngle > 0 ? uvAngle : -uvAngle);
    }

    // Vector perpindicular to both u and v.
    Eigen::Vector3d n = u.cross(v);

    Eigen::Vector3d x = Eigen::Vector3d(1, 0, 0);
    Eigen::Vector3d y = Eigen::Vector3d(0, 1, 0);

    if (n.norm() < 1e-16)
    {
      Eigen::Vector3d A = u.cross(x);
      Eigen::Vector3d B = u.cross(y);

      n = A.norm() >= B.norm() ? A : B;
    }

    n = n / n.norm();

    // Add the vectors to the origin vector to find the positions along the lines
    // of the two points the curve starts and ends at.
    direction1 = origin + u;
    direction2 = origin + v;

    // Calculate the points along the curve at each half-degree increment until we
    // reach the next line.
    Eigen::Vector3d points[720];
    for (int theta = 1; theta < (uvAngle * 2); theta++)
    {
      // Create a Matrix that represents a rotation about a vector perpindicular
      // to the plane.
      Eigen::Matrix3d rotMat;
      rotMat.loadRotation3((theta / 2 * (M_PI / 180.0)), n);

      // Apply the rotation Matrix to the vector to find the new point.
      if (alternateAngle) {
        rotMat.multiply(v, &points[theta-1]);
      } else {
        rotMat.multiply(u, &points[theta-1]);
      }
      points[theta-1] += origin;
      points[theta-1] = d->widget->camera()->modelview() * points[theta-1];
    }

    // Get vectors representing the points' positions in terms of the model view.
    origin = d->widget->camera()->modelview() * origin;
    direction1 = d->widget->camera()->modelview() * direction1;
    direction2 = d->widget->camera()->modelview() * direction2;

    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glPushMatrix();
    glLoadIdentity();
    glDisable(GL_LIGHTING);
    glDisable(GL_CULL_FACE);

    glLineWidth(lineWidth);
    glColor4f(d->color.red(), d->color.green(), d->color.blue(), d->color.alpha());

    // Draw the arc.
    glBegin(GL_LINE_STRIP);
    if (alternateAngle)
    {
      glVertex3d(direction2.x(), direction2.y(), direction2.z());
      for (int i = 0; i < uvAngle*2 - 1; i++)
        glVertex3d(points[i].x(), points[i].y(), points[i].z());
      glVertex3d(direction1.x(), direction1.y(), direction1.z());
    }
    else
    {
      glVertex3d(direction1.x(), direction1.y(), direction1.z());
      for (int i = 0; i < uvAngle*2 - 1; i++)
        glVertex3d(points[i].x(), points[i].y(), points[i].z());
      glVertex3d(direction2.x(), direction2.y(), direction2.z());
    }
    glEnd();

    glPopMatrix();
    glPopAttrib();
  }

  void GLPainter::drawShadedQuadrilateral(Eigen::Vector3d point1, Eigen::Vector3d point2,
                                        Eigen::Vector3d point3, Eigen::Vector3d point4)
  {
    assert( d->widget );

    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glPushMatrix();
    glLoadIdentity();
    glDisable(GL_LIGHTING);
    glDisable(GL_CULL_FACE);

    glColor4f(d->color.red(), d->color.green(), d->color.blue(), d->color.alpha());

    glBegin(GL_TRIANGLE_FAN);
    glVertex3d(point1.x(), point1.y(), point1.z());
    glVertex3d(point2.x(), point2.y(), point2.z());
    glVertex3d(point3.x(), point3.y(), point3.z());
    glVertex3d(point4.x(), point4.y(), point4.z());
    glEnd();

    glPopMatrix();
    glPopAttrib();
  }

  void GLPainter::drawQuadrilateral(Eigen::Vector3d point1, Eigen::Vector3d point2,
                                  Eigen::Vector3d point3, Eigen::Vector3d point4,
                                  double lineWidth)
  {
    assert( d->widget );

    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glPushMatrix();
    glLoadIdentity();
    glDisable(GL_LIGHTING);
    glDisable(GL_CULL_FACE);

    glLineWidth(lineWidth);
    glColor4f(d->color.red(), d->color.green(), d->color.blue(), d->color.alpha());

    glBegin(GL_LINE_LOOP);
    glVertex3d(point1.x(), point1.y(), point1.z());
    glVertex3d(point2.x(), point2.y(), point2.z());
    glVertex3d(point3.x(), point3.y(), point3.z());
    glVertex3d(point4.x(), point4.y(), point4.z());
    glEnd();

    glPopMatrix();
    glPopAttrib();
  }

  int GLPainter::drawText ( int x, int y, const QString &string ) const
  {
    if(!d->isValid()) { return 0; }
      d->textRenderer->begin ( d->widget );
    int val = d->textRenderer->draw ( x, y, string );

    d->textRenderer->end( );
    return val;
  }

  int GLPainter::drawText ( const QPoint& pos, const QString &string ) const
  {
    assert( d->widget );
    if(!d->isValid()) { return 0; }
    d->textRenderer->begin( d->widget );
    int val = d->textRenderer->draw ( pos.x(), pos.y(), string );
    d->textRenderer->end( );
    return 0;
  }

  int GLPainter::drawText ( const Eigen::Vector3d &pos, const QString &string ) const
  {
    if(!d->isValid()) { return 0; }
      d->textRenderer->begin ( d->widget );
    Eigen::Vector3d transformedPos = d->widget->camera()->modelview() * pos;

    // perform a rough form of frustum culling
    double dot = transformedPos.z() / transformedPos.norm();
    if ( dot > PAINTER_FRUSTUM_CULL_TRESHOLD ) return 0;

    int val = d->textRenderer->draw ( pos, string );

    d->textRenderer->end( );

    return val;
  }

  int GLPainter::defaultQuality()
  {
    return DEFAULT_GLOBAL_QUALITY_SETTING;
  }

  int GLPainter::maxQuality()
  {
    return PAINTER_GLOBAL_QUALITY_SETTINGS-1;
  }

  bool GLPainter::isShared()
  {
    return d->sharing-1;
  }

  bool GLPainter::isActive()
  {
    return (d->widget);
  }

  void GLPainter::incrementShare()
  {
    d->sharing++;
  }

  void GLPainter::decrementShare()
  {
    d->sharing--;
  }

  void GLPainter::begin(GLWidget *widget)
  {
    d->widget = widget;
    d->overflow++;
  }

  void GLPainter::end()
  {
    d->overflow--;
    if(!d->overflow)
    {
      d->widget = 0;
    }
  }

  void GLPainter::pushName()
  {
    // Push the type and id if they are set
    if (d->id != -1)
    {
      glPushName(d->type);
      glPushName(d->id);
    }
  }

  void GLPainter::popName()
  {
    // Pop the type and id if they are set, then reset them
    if (d->id != -1)
    {
      glPopName();
      glPopName();
      d->type = Primitive::OtherType;
      d->id = -1;
    }
  }

} // end namespace Avogadro
