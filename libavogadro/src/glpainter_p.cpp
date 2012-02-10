/**********************************************************************
  Painter - drawing spheres, cylinders and text in a GLWidget

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

#include "glpainter_p.h"
#include "glwidget.h"
#include "camera.h"
#include "sphere_p.h"
#include "cylinder_p.h"
#include "textrenderer_p.h"

#include <avogadro/atom.h>
#include <avogadro/bond.h>
#include <avogadro/molecule.h>
#include <avogadro/mesh.h>
#include <avogadro/color.h>
#include <avogadro/color3f.h>

#include <QDebug>
#include <QColor>
#include <QVarLengthArray>
#include <Eigen/Geometry>

#ifdef Q_WS_MAC
# include <OpenGL/glu.h>
#else
# include <GL/glu.h>
#endif

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
//  const double   PAINTER_FRUSTUM_CULL_TRESHOLD = -0.8;

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

    if(!initialized)
    {
      if(newQuality != -1) {
        quality = newQuality;
      }
      qDebug() << "createObjects()";
      createObjects();
      initialized = true;
    }
    else if(newQuality != -1)
    {
      if(newQuality != quality)
      {
        qDebug() << "updateObjects()";
        deleteObjects();
        quality = newQuality;
        createObjects();
      }
      newQuality = -1;
    }
    return true;
  }

  void GLPainterPrivate::deleteObjects()
  {
    int level, lastLevel, n;
    // delete the spheres. One has to be wary that more than one sphere
    // pointer may have the same value. One wants to avoid deleting twice the same sphere.
    if (spheres) {
      lastLevel = -1;
      for (n = 0; n < PAINTER_DETAIL_LEVELS; ++n) {
        level = PAINTER_SPHERES_LEVELS_ARRAY[quality][n];
        if (level != lastLevel) {
          lastLevel = level;
          if (spheres[n]) {
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
    if (cylinders)
    {
      lastLevel = -1;
      for (n = 0; n < PAINTER_DETAIL_LEVELS; ++n) {
        level = PAINTER_CYLINDERS_LEVELS_ARRAY[quality][n];
        if (level != lastLevel) {
          lastLevel = level;
          if (cylinders[n]) {
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
      for (int n = 1; n < PAINTER_DETAIL_LEVELS; ++n) {
        level = PAINTER_SPHERES_LEVELS_ARRAY[quality][n];
        if (level == lastLevel) {
          spheres[n] = spheres[n-1];
        }
        else {
          lastLevel = level;
          spheres[n] = new Sphere ( level );
        }
      }
    }

    // create the cylinders. More than one cylinder detail level may have the same value.
    // in that case we want to reuse the corresponding cylinder by just copying the pointer,
    // instead of creating redundant cylinders.
    if (cylinders == 0) {
      cylinders = new Cylinder*[PAINTER_DETAIL_LEVELS];
      int level, lastLevel;
      lastLevel = PAINTER_SPHERES_LEVELS_ARRAY[quality][0];
      cylinders[0] = new Cylinder ( lastLevel );
      for (int n = 1; n < PAINTER_DETAIL_LEVELS; ++n) {
        level = PAINTER_CYLINDERS_LEVELS_ARRAY[quality][n];
        if (level == lastLevel) {
          cylinders[n] = cylinders[n-1];
        }
        else {
          lastLevel = level;
          cylinders[n] = new Cylinder ( level );
        }
      }
    }
  }

  GLPainter::GLPainter(int quality) : d(new GLPainterPrivate),
    m_dynamicScaling(true)
  {
    if (quality < 0 || quality >= PAINTER_MAX_DETAIL_LEVEL)
      quality = DEFAULT_GLOBAL_QUALITY_SETTING;
    else
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
      d->id = static_cast<const Atom *>(primitive)->index();
    else if (d->type == Primitive::BondType)
      d->id = static_cast<const Bond *>(primitive)->index();
  }

  void GLPainter::setName ( Primitive::Type type, int id )
  {
    d->type = type;
    d->id = id;
  }

  void GLPainter::setColor (const Color *color)
  {
    d->color.setFromRgba(color->red(), color->green(), color->blue(),
                         color->alpha());
  }

  void GLPainter::setColor (const QColor *color)
  {
    d->color.setFromRgba(color->redF(), color->greenF(), color->blueF(),
                         color->alphaF());
  }

  void GLPainter::setColor ( float red, float green, float blue, float alpha )
  {
    d->color.setFromRgba(red, green, blue, alpha);
  }

  void GLPainter::setColor(QString name)
  {
    d->color.setFromQColor(QColor(name));
  }

  void GLPainter::drawSphere (const Eigen::Vector3d &center, double radius)
  {
    if(!d->isValid())
      return;

    // Default to the minimum detail level for this quality
    int detailLevel = PAINTER_MAX_DETAIL_LEVEL / 3;

    if (d->widget->projection() != GLWidget::Orthographic &&
        m_dynamicScaling) {
      double apparentRadius = radius / d->widget->camera()->distance(center);
      detailLevel = 1 + static_cast<int>(floor (PAINTER_SPHERES_DETAIL_COEFF
                        * (sqrt(apparentRadius) - PAINTER_SPHERES_SQRT_LIMIT_MIN_LEVEL)));
      if (detailLevel < 0)
        detailLevel = 0;
      if (detailLevel > PAINTER_MAX_DETAIL_LEVEL)
        detailLevel = PAINTER_MAX_DETAIL_LEVEL;
    }

    d->color.applyAsMaterials();
    pushName();
    d->spheres[detailLevel]->draw (center, radius);
    popName();
  }

  void GLPainter::drawCylinder ( const Eigen::Vector3d &end1, const Eigen::Vector3d &end2,
                                 double radius )
  {
    if(!d->isValid()) { return; }

    // Default to the minimum detail level for this quality
    int detailLevel = PAINTER_MAX_DETAIL_LEVEL / 3;

    if (d->widget->projection() != GLWidget::Orthographic &&
        m_dynamicScaling) {
      double apparentRadius = radius / d->widget->camera()->distance(end1);
      detailLevel = 1 + static_cast<int> ( floor (
                                                    PAINTER_CYLINDERS_DETAIL_COEFF
                                                    * ( sqrt ( apparentRadius ) - PAINTER_CYLINDERS_SQRT_LIMIT_MIN_LEVEL )
                                                    ) );
      if (detailLevel < 0)
        detailLevel = 0;
      if (detailLevel > PAINTER_MAX_DETAIL_LEVEL)
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

    // Default to the minimum detail level for this quality
    int detailLevel = PAINTER_MAX_DETAIL_LEVEL / 3;

    if (d->widget->projection() != GLWidget::Orthographic &&
        m_dynamicScaling) {
      double apparentRadius = radius / d->widget->camera()->distance(end1);
      detailLevel = 1 + static_cast<int> ( floor (
                                                    PAINTER_CYLINDERS_DETAIL_COEFF
                                                    * ( sqrt ( apparentRadius ) - PAINTER_CYLINDERS_SQRT_LIMIT_MIN_LEVEL )
                                                    ) );
      if (detailLevel < 0)
        detailLevel = 0;
      if (detailLevel > PAINTER_MAX_DETAIL_LEVEL)
        detailLevel = PAINTER_MAX_DETAIL_LEVEL;
    }

    d->color.applyAsMaterials();
    pushName();
    d->cylinders[detailLevel]->drawMulti ( end1, end2, radius, order,
                                           shift, d->widget->normalVector() );
    popName();
  }

  void GLPainter::drawCone(const Eigen::Vector3d &base,
                           const Eigen::Vector3d &cap,
                           double baseRadius,
                           double)
  {
    const int CONE_TESS_LEVEL = 30;
    // This draws a cone which will be most useful for drawing arrows etc.
    Eigen::Vector3d axis = cap - base;
    Eigen::Vector3d axisNormalized = axis.normalized();
    Eigen::Vector3d ortho1, ortho2;
    ortho1 = axisNormalized.unitOrthogonal();
    ortho1 *= baseRadius;
    ortho2 = axisNormalized.cross(ortho1);

    d->color.applyAsMaterials();

    // Draw the cone
    // unfortunately we can't use a GL_TRIANGLE_FAN because this would force
    // having a common normal vector at the tip.
    for (int j = 0; j < CONE_TESS_LEVEL; j++) {
      const double alphaStep = 2.0 * M_PI / CONE_TESS_LEVEL;
      double alpha = j * alphaStep;
      double alphaNext = alpha + alphaStep;
      double alphaPrec = alpha - alphaStep;
      Eigen::Vector3d v = sin(alpha) * ortho1 + cos(alpha) * ortho2 + base;
      Eigen::Vector3d vNext = sin(alphaNext) * ortho1 + cos(alphaNext) * ortho2 + base;
      Eigen::Vector3d vPrec = sin(alphaPrec) * ortho1 + cos(alphaPrec) * ortho2 + base;
      Eigen::Vector3d n = (cap - v).cross(v - vPrec).normalized();
      Eigen::Vector3d nNext = (cap - vNext).cross(vNext - v).normalized();
      glBegin(GL_TRIANGLES);
      glNormal3dv((n+nNext).normalized().data());
      glVertex3dv(cap.data());
      glNormal3dv(nNext.data());
      glVertex3dv(vNext.data());
      glNormal3dv(n.data());
      glVertex3dv(v.data());
      glEnd();
    }

    // Now to draw the base
    glBegin(GL_TRIANGLE_FAN);
    glNormal3dv((-axisNormalized).eval().data());
    glVertex3dv(base.data());
    for (int j = 0; j <= CONE_TESS_LEVEL; j++) {
      double alpha = -j * M_PI / (CONE_TESS_LEVEL/2.0);
      Eigen::Vector3d v = cos(alpha) * ortho1 + sin(alpha) * ortho2 + base;
      glVertex3dv(v.data());
    }
    glEnd();
  }

  void GLPainter::drawLine(const Eigen::Vector3d &start, const Eigen::Vector3d &end,
                           double lineWidth)
  {
    // Draw a line between two points of the specified thickness
    if(!d->isValid()) { return; }

    glPushAttrib(GL_LIGHTING_BIT);
    glDisable(GL_LIGHTING);

    glLineWidth(lineWidth);
    d->color.apply();

    // Draw the line
    glBegin(GL_LINE_STRIP);
    glVertex3dv(start.data());
    glVertex3dv(end.data());
    glEnd();

    glPopAttrib();
  }

  void GLPainter::drawMultiLine(const Eigen::Vector3d &end1,
                                const Eigen::Vector3d &end2,
                                double lineWidth, int order, short stipple)
  {
    // Draw multiple lines between two points of the specified thickness
    if(!d->isValid()) { return; }

    // construct the 4D transformation matrix
    Eigen::Matrix4d matrix;
    matrix.row(3) << 0,0,0,1;
    matrix.block<3,1>(0,3) = end1;
    matrix.block<3,1>(0,2) = end2 - end1; // the "axis vector" of the line
    // Now we want to construct an orthonormal basis whose third
    // vector is axis.normalized(). The first vector in this
    // basis, which we call ortho1, should be approximately lying in the
    // z=0 plane if possible. This is to ensure double bonds don't look
    // like single bonds from the default point of view.
    Eigen::Vector3d axisNormalized = matrix.block<3,1>(0,2).normalized();
    Eigen::Block<Eigen::Matrix4d, 3, 1> ortho1(matrix, 0, 0);
    ortho1 = axisNormalized.cross(d->widget->normalVector());
    double ortho1Norm = ortho1.norm();
    if( ortho1Norm > 0.001 ) ortho1 = ortho1.normalized() * lineWidth;
    else ortho1 = axisNormalized.unitOrthogonal() * lineWidth;
    matrix.block<3,1>(0,1) = axisNormalized.cross(ortho1);

    // now the matrix is entirely filled, so we can do the actual drawing !
    glPushMatrix();
    glMultMatrixd( matrix.data() );

    glDisable(GL_LIGHTING);

    glLineWidth(lineWidth);
    d->color.apply();

    glEnable(GL_LINE_STIPPLE);
    glLineStipple(1, stipple);

    // Draw the line
    if (order == 1 || order == -1) { // single or aromatic
      glBegin(GL_LINE_STRIP);
      glVertex3f(0.0, 0.0, 0.0);
      glVertex3f(0.0, 0.0, 1.0);
      glEnd();
    }
    else {
      double angleOffset = 0.0;
      if( order >= 3 ) {
        if( order == 3 ) angleOffset = 90.0;
        else angleOffset = 22.5;
      }

      // these may need further refinement
      double displacementFactor = 0.0004 * lineWidth + 0.018;
      for( int i = 0; i < order; i++) {
        glPushMatrix();
        glRotated( angleOffset + 360.0 * i / order,
                   0.0, 0.0, 1.0 );
        glTranslated( displacementFactor, 0.0, 0.0 );

        glBegin(GL_LINE_STRIP);
        glVertex3f(0.0, 0.0, 0.0);
        glVertex3f(0.0, 0.0, 1.0);
        glEnd();

        glPopMatrix();
      }
    }
    glDisable(GL_LINE_STIPPLE);
    glPopMatrix();

    glEnable(GL_LIGHTING);
  }

  void GLPainter::drawTriangle(const Eigen::Vector3d &p1, const Eigen::Vector3d &p2,
                               const Eigen::Vector3d &p3)
  {
    if(!d->isValid()) { return; }

    // Sort out the winding order by assigning in the correct order
    Eigen::Vector3d tp2, tp3;

    // Don't want planes to be too shiny.
    d->color.applyAsFlatMaterials();

    // The plane normal vector of the view
    const Eigen::Vector3d planeNormalVector = d->widget->normalVector();

    // Calculate the normal for the triangle as GL_AUTO_NORMAL doesn't seem to work
    Eigen::Vector3d v1, v2, n;
    v1 = p2 - p1;
    v2 = p3 - p2;
    n = v1.cross(v2);
    n.normalize();

    // Dot product is 1 or -1 - want normals facing the same direction
    if (n.dot(p1 - d->widget->camera()->backTransformedZAxis()) < 0) {
      n *= -1;
      tp2 = p3;
      tp3 = p2;
    }
    else {
      tp2 = p2;
      tp3 = p3;
    }

    glBegin(GL_TRIANGLES);
    glNormal3dv(n.data());
    glVertex3dv(p1.data());
    glVertex3dv(tp2.data());
    glVertex3dv(tp3.data());
    glEnd();
  }

  void GLPainter::drawTriangle(const Eigen::Vector3d &p1, const Eigen::Vector3d &p2,
                               const Eigen::Vector3d &p3, const Eigen::Vector3d &n)
  {
    if(!d->isValid()) { return; }

    // Don't want planes to be too shiny.
    d->color.applyAsFlatMaterials();
    d->color.apply();

    glBegin(GL_TRIANGLES);
    glNormal3dv(n.data());
    glVertex3dv(p1.data());
    glVertex3dv(p2.data());
    glVertex3dv(p3.data());
    glEnd();
  }

  void GLPainter::drawSpline(const QVector<Eigen::Vector3d>& pts, double radius)
  {
    // Draw a spline between two points of the specified thickness
    if(!d->isValid()) { return; }

    // The first value is repeated three times as is the last in order to complete the curve
    QVector<Eigen::Vector3d> points = pts;

    //    glColor4f(d->color.red(), d->color.green(), d->color.blue(), d->color.alpha());

    /*    QVector<Eigen::Vector3d> p, a;
          a.resize(4);
          p.resize(4);

          // Define the number of interpolated points between control points
          int numPts = 40;
          double step = 1. / double(numPts);

          Eigen::Vector3d last, cur;

          for (int i = 2; i < pts.size()+1; i++) {
          p[0] = points.at(i-1);
          p[1] = points.at(i);
          p[2] = points.at(i+1);
          p[3] = points.at(i+2);

          // Now calculate the basis
          a[0] = (-p[0] + 3.*p[1] - 3.*p[2] + p[3]) / 6.;
          a[1] = (3.*p[0] - 6.*p[1] + 3.*p[2]) / 6.;
          a[2] = (-3.*p[0] + 3.*p[2]) / 6.;
          a[3] = (p[0] + 4.*p[1] + p[2]) / 6.;

          // Now interpolate some points and draw them...
          last = a[3];
          for (int j = 0; j < numPts; j++) {
          double t = step * j;
          cur = a[3] + t*(a[2] + t*(a[1] + t*a[0]));
          //   drawCylinder(last, cur, radius/4.);
          last = cur;
          }
          }
    */
    glEnable(GL_AUTO_NORMAL);
    GLUnurbsObj *nurb = gluNewNurbsRenderer();
    // These settings were inspired by the code supplied by Thomas Margraf
    // and tweaked a little more by me - performance seems good.
    // FIXME Should still be linked to our global quality level.
    gluNurbsProperty(nurb, GLU_V_STEP, 4);
    gluNurbsProperty(nurb, GLU_U_STEP, 10);
    gluNurbsProperty(nurb, GLU_CULLING, GL_TRUE);
    gluNurbsProperty(nurb, GLU_SAMPLING_METHOD, GLU_DOMAIN_DISTANCE);

    // This seems reasonable but should be linked to the detail level
    int TUBE_TESS = 6;

    QVarLengthArray<GLfloat> ctrlpts(points.size()*TUBE_TESS*3);
    QVarLengthArray<GLfloat> uknots(points.size() + 4);

    // The first one is a special case
    Eigen::Vector3d axis = points[1] - points[0];
    Eigen::Vector3d axisNormalized = axis.normalized();
    Eigen::Vector3d ortho1, ortho2;
    ortho1 = axisNormalized.unitOrthogonal() * radius;
    ortho2 = axisNormalized.cross(ortho1);
    for (int j = 0; j < TUBE_TESS; j++) {
      double alpha = j * M_PI / 1.5f;
      Eigen::Vector3d v = cosf(alpha) * ortho1 + sinf(alpha) * ortho2;
      ctrlpts[3*j+0] = v.x() + points[0].x();
      ctrlpts[3*j+1] = v.y() + points[0].y();
      ctrlpts[3*j+2] = v.z() + points[0].z();
    }
    uknots[2] = 0.0;

    for (int i = 1; i < points.size(); i++) {
      axis = Eigen::Vector3d(points[i-1].x() - points[i].x(),
                             points[i-1].y() - points[i].y(),
                             points[i-1].z() - points[i].z());
      axisNormalized = axis.normalized();
      ortho1 = axisNormalized.unitOrthogonal();
      ortho1 *= radius;
      ortho2 = axisNormalized.cross(ortho1);
      for (int j = 0; j < TUBE_TESS; j++) {
        double alpha = j * M_PI / 1.5f;
        Eigen::Vector3d v = cosf(alpha) * ortho1 + sinf(alpha) * ortho2;
        ctrlpts[(i*TUBE_TESS + j)*3 + 0] = v.x() + points[i].x();
        ctrlpts[(i*TUBE_TESS + j)*3 + 1] = v.y() + points[i].y();
        ctrlpts[(i*TUBE_TESS + j)*3 + 2] = v.z() + points[i].z();
      }
      uknots[i+2] = i - 1.0;
    }
    uknots[0] = 0.0;
    uknots[1] = 0.0;
    uknots[points.size()] = points.size() - 1.0;
    uknots[points.size()+1] = points.size() - 1.0;
    uknots[points.size()+2] = points.size() - 1.0;
    uknots[points.size()+3] = points.size() - 1.0;

    // Hard coded right now - will generalise for arbitrary TUBE_TESS values
    GLfloat vknots[10] = {0., 0., 1., 2., 3., 4., 5., 6., 7., 7.};

    d->color.applyAsMaterials();

    // Actually draw the tube as a nurb
    gluBeginSurface(nurb);

    gluNurbsSurface(nurb,
                    points.size() + 4, uknots.data(),
                    TUBE_TESS + 4, vknots,
                    TUBE_TESS*3,
                    3,
                    ctrlpts.data(),
                    4, 4,
                    GL_MAP2_VERTEX_3);

    gluEndSurface(nurb);

    gluDeleteNurbsRenderer(nurb);

    glDisable(GL_AUTO_NORMAL);
  }

  void GLPainter::drawShadedSector(const Eigen::Vector3d & origin, const Eigen::Vector3d & direction1,
                                   const Eigen::Vector3d & direction2, double radius, bool alternateAngle)
  {
    assert( d->widget );

    // Get vectors representing the two lines out from the center of the circle.
    Eigen::Vector3d u = direction1 - origin;
    Eigen::Vector3d v = direction2 - origin;

    // Adjust the length of u and v to the radius given.
    u = u.normalized() * radius;
    v = v.normalized() * radius;

    // Angle between u and v.
    double uvAngle = acos(u.dot(v) / v.squaredNorm()) * 180.0 / M_PI;

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

    if (n.norm() < 1e-3)
      {
        Eigen::Vector3d A = u.cross(Eigen::Vector3d::UnitX());
        Eigen::Vector3d B = u.cross(Eigen::Vector3d::UnitY());

        n = A.norm() >= B.norm() ? A : B;
      }

    n.normalize();

    // Calculate the points along the curve at each half-degree increment until we
    // reach the next line.
    Eigen::Vector3d points[720];
    for (int theta = 1; theta < (uvAngle * 2); theta++)
      {
        // Apply a rotation about a vector perpindicular
        // to the plane to the vector to find the new point.
        if (alternateAngle) {
          points[theta-1] = Eigen::AngleAxisd(theta * (M_PI / 180.0) / 2, n) * v;
        } else {
          points[theta-1] = Eigen::AngleAxisd(theta * (M_PI / 180.0) / 2, n) * u;
        }
        points[theta-1] = d->widget->camera()->modelview() * (origin + points[theta-1]);
      }

    // Get vectors representing the points' positions in terms of the model view.
    Eigen::Vector3d _origin = d->widget->camera()->modelview() * origin;
    Eigen::Vector3d _direction1 = d->widget->camera()->modelview() * (origin+u);
    Eigen::Vector3d _direction2 = d->widget->camera()->modelview() * (origin+v);

    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glPushMatrix();
    glLoadIdentity();
    glDisable(GL_LIGHTING);
    glDisable(GL_CULL_FACE);

    d->color.apply();

    // Draw the transparent polygon that makes up the sector.
    glBegin(GL_TRIANGLE_FAN);
    glVertex3dv(_origin.data());
    if (alternateAngle)
      {
        glVertex3dv(_direction2.data());
        for (int i = 0; i < uvAngle*2 - 1; i++)
          glVertex3dv(points[i].data());
        glVertex3dv(_direction1.data());
      }
    else
      {
        glVertex3dv(_direction1.data());
        for (int i = 0; i < uvAngle*2 - 1; i++)
          glVertex3dv(points[i].data());
        glVertex3dv(_direction2.data());
      }
    glEnd();

    glPopMatrix();
    glPopAttrib();
  }

  void GLPainter::drawArc(const Eigen::Vector3d & origin, const Eigen::Vector3d & direction1,
                          const Eigen::Vector3d & direction2, double radius, double lineWidth,
                          bool alternateAngle)
  {
    assert( d->widget );

    // Get vectors representing the two lines out from the center of the circle.
    Eigen::Vector3d u = direction1 - origin;
    Eigen::Vector3d v = direction2 - origin;

    // Adjust the length of u and v to the radius given.
    u = u.normalized() * radius;
    v = v.normalized() * radius;

    // Angle between u and v.
    double uvAngle = acos(u.dot(v) / v.squaredNorm()) * 180.0 / M_PI;

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

    if (n.norm() < 1e-3)
      {
        Eigen::Vector3d A = u.cross(Eigen::Vector3d::UnitX());
        Eigen::Vector3d B = u.cross(Eigen::Vector3d::UnitY());

        n = A.norm() >= B.norm() ? A : B;
      }

    n.normalize();

    // Calculate the points along the curve at each half-degree increment until we
    // reach the next line.
    Eigen::Vector3d points[720];
    for (int theta = 1; theta < (uvAngle * 2); theta++)
      {
        // Apply a rotation about a vector perpindicular
        // to the plane to the vector to find the new point.
        if (alternateAngle) {
          points[theta-1] = Eigen::AngleAxisd(theta * (M_PI / 180.0) / 2, n) * v;
        } else {
          points[theta-1] = Eigen::AngleAxisd(theta * (M_PI / 180.0) / 2, n) * u;
        }
        points[theta-1] = d->widget->camera()->modelview() * (origin + points[theta-1]);
      }

    // Get vectors representing the points' positions in terms of the model view.
    Eigen::Vector3d _direction1 = d->widget->camera()->modelview() * (origin + u);
    Eigen::Vector3d _direction2 = d->widget->camera()->modelview() * (origin + v);

    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glPushMatrix();
    glLoadIdentity();
    glDisable(GL_LIGHTING);
    glDisable(GL_CULL_FACE);

    glLineWidth(lineWidth);
    d->color.apply();

    // Draw the arc.
    glBegin(GL_LINE_STRIP);
    if (alternateAngle)
      {
        glVertex3dv(_direction2.data());
        for (int i = 0; i < uvAngle*2 - 1; i++)
          glVertex3dv(points[i].data());
        glVertex3dv(_direction1.data());
      }
    else
      {
        glVertex3dv(_direction1.data());
        for (int i = 0; i < uvAngle*2 - 1; i++)
          glVertex3dv(points[i].data());
        glVertex3dv(_direction2.data());
      }
    glEnd();

    glPopMatrix();
    glPopAttrib();
  }

  void GLPainter::drawShadedQuadrilateral(const Eigen::Vector3d & point1, const Eigen::Vector3d & point2,
                                          const Eigen::Vector3d & point3, const Eigen::Vector3d & point4)
  {
    assert( d->widget );

    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glPushMatrix();
    glLoadIdentity();
    glDisable(GL_LIGHTING);
    glDisable(GL_CULL_FACE);

    d->color.apply();

    glBegin(GL_TRIANGLE_FAN);
    glVertex3dv(point1.data());
    glVertex3dv(point2.data());
    glVertex3dv(point3.data());
    glVertex3dv(point4.data());
    glEnd();

    glPopMatrix();
    glPopAttrib();
  }

  void GLPainter::drawQuadrilateral(const Eigen::Vector3d & point1, const Eigen::Vector3d & point2,
                                    const Eigen::Vector3d & point3, const Eigen::Vector3d & point4,
                                    double lineWidth)
  {
    assert( d->widget );

    glPushAttrib(GL_LIGHTING_BIT);
    glDisable(GL_LIGHTING);

    glLineWidth(lineWidth);
    d->color.apply();

    glBegin(GL_LINE_LOOP);
    glVertex3dv(point1.data());
    glVertex3dv(point2.data());
    glVertex3dv(point3.data());
    glVertex3dv(point4.data());
    glEnd();

    glPopAttrib();
  }

  void GLPainter::drawLineLoop(const QList<Eigen::Vector3d> & points,
                               const double lineWidth)
  {
    assert( d->widget );

    glPushAttrib(GL_LIGHTING_BIT);
    glDisable(GL_LIGHTING);

    glLineWidth(lineWidth);
    d->color.apply();

    glBegin(GL_LINE_LOOP);

    for (QList<Eigen::Vector3d>::const_iterator
           it = points.constBegin(),
           it_end = points.constEnd();
         it != it_end; ++it) {
      glVertex3dv(it->data());
    }

    glEnd();

    glPopAttrib();
  }

  void GLPainter::drawMesh(const Mesh & mesh, int mode)
  {
    // Now we draw the given mesh to the OpenGL widget
    switch (mode)
    {
      case 0:
        glPolygonMode(GL_FRONT, GL_FILL);
        glEnable(GL_LIGHTING);
        break;
      case 1:
        glPolygonMode(GL_FRONT, GL_LINE);
        glDisable(GL_LIGHTING);
        break;
      case 2:
        glPolygonMode(GL_FRONT, GL_POINT);
        glDisable(GL_LIGHTING);
        break;
    }

    d->color.apply();
    d->color.applyAsMaterials();

    // Render the triangles of the mesh
    std::vector<Eigen::Vector3f> v = mesh.vertices();
    std::vector<Eigen::Vector3f> n = mesh.normals();

    if (v.size() != n.size()) {
      qDebug() << "Vertices size does not equal normals size:" << v.size()
               << n.size();
      return;
    }

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, &(v[0]));
    glNormalPointer(GL_FLOAT, 0, &(n[0]));
    glDrawArrays(GL_TRIANGLES, 0, v.size());
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);

    glPolygonMode(GL_FRONT, GL_FILL);
    glEnable(GL_LIGHTING);
  }

  void GLPainter::drawColorMesh(const Mesh & mesh, int mode)
  {
    // Now we draw the given mesh to the OpenGL widget
    switch (mode)
    {
      case 0:
        glPolygonMode(GL_FRONT, GL_FILL);
        glEnable(GL_LIGHTING);
        break;
      case 1:
        glPolygonMode(GL_FRONT, GL_LINE);
        glDisable(GL_LIGHTING);
        break;
      case 2:
        glPolygonMode(GL_FRONT, GL_POINT);
        glDisable(GL_LIGHTING);
        break;
    }

    // Render the triangles of the mesh
    std::vector<Eigen::Vector3f> v = mesh.vertices();
    std::vector<Eigen::Vector3f> n = mesh.normals();
    std::vector<Color3f> c = mesh.colors();

    if (v.size() != n.size() || v.size() != c.size()) {
      qDebug() << "Vertices size does not equal normals size or color size:"
               << v.size() << n.size() << c.size();
      return;
    }

    d->color.applyAsMaterials();

    /* Need to work out issues with material properties, lighting etc...
    glEnable(GL_COLOR_MATERIAL);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, &(v[0]));
    glNormalPointer(GL_FLOAT, 0, &(n[0]));
    glColorPointer(3, GL_FLOAT, 0, &(c[0]));
    glDrawArrays(GL_TRIANGLES, 0, v.size());
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    glDisable(GL_COLOR_MATERIAL);
*/

    float alpha = d->color.alpha();

    glBegin(GL_TRIANGLES);
    for(unsigned int i = 0; i < v.size(); ++i) {
      applyAsMaterials(c[i], alpha);
      glNormal3fv(n[i].data());
      glVertex3fv(v[i].data());
    }
    glEnd();

    glPolygonMode(GL_FRONT, GL_FILL);
    glEnable(GL_LIGHTING);
  }

  int GLPainter::drawText ( int x, int y, const QString &string )
  {
    if(!d->isValid()) { return 0; }
    d->textRenderer->begin ( d->widget );
    int val = d->textRenderer->draw ( x, y, string );

    d->textRenderer->end( );
    return val;
  }

  int GLPainter::drawText ( const QPoint& pos, const QString &string )
  {
    assert( d->widget );
    if(!d->isValid()) { return 0; }
    d->textRenderer->begin( d->widget );
    d->textRenderer->draw ( pos.x(), pos.y(), string );
    d->textRenderer->end( );
    return 0;
  }

  int GLPainter::drawText ( const Eigen::Vector3d &pos, const QString &string )
  {
    if(!d->isValid()) { return 0; }
    d->textRenderer->begin ( d->widget );
    int val = d->textRenderer->draw ( pos, string );
    d->textRenderer->end( );
    return val;
  }

  int GLPainter::drawText(const Eigen::Vector3d &pos, const QString &string, const QFont &font)
  {
    if(!d->isValid()) { return 0; }
    d->widget->renderText(pos.x(), pos.y(), pos.z(), string, font);
    return 0;
  }

  void GLPainter::drawBox(const Eigen::Vector3d &,
                          const Eigen::Vector3d &)
  {
  }

  void GLPainter::drawBoxEdges(const Eigen::Vector3d &offset,
                             const Eigen::Vector3d &v1,
                             const Eigen::Vector3d &v2,
                             const Eigen::Vector3d &v3,
                             const double linewidth)
  {
    //       6------8  c1 = origin
    //      /:     /|  c2 = origin + v1
    //     / :    / |  c3 = origin + v2
    //    /  4---/--7  c4 = origin + v3
    //   /  /   /  /   c5 = origin + v1 + v2
    //  3------5  /    c6 = origin + v2 + v3
    //  | /    | /     c7 = origin + v1 + v3
    //  |/     |/      c8 = origin + v1 + v2 + v3
    //  1------2
    const Eigen::Vector3d &c1 (offset);
    const Eigen::Vector3d  c2 (c1 + v1);
    const Eigen::Vector3d  c3 (c1 + v2);
    const Eigen::Vector3d  c4 (c1 + v3);
    const Eigen::Vector3d  c5 (c2 + v2);
    const Eigen::Vector3d  c6 (c3 + v3);
    const Eigen::Vector3d  c7 (c2 + v3);
    const Eigen::Vector3d  c8 (c5 + v3);
    this->drawBoxEdges(c1, c2, c3, c4, c5, c6, c7, c8, linewidth);
  }

  void GLPainter::drawBoxEdges(const Eigen::Vector3d &c1,
                               const Eigen::Vector3d &c2,
                               const Eigen::Vector3d &c3,
                               const Eigen::Vector3d &c4,
                               const Eigen::Vector3d &c5,
                               const Eigen::Vector3d &c6,
                               const Eigen::Vector3d &c7,
                               const Eigen::Vector3d &c8,
                               const double lineWidth)
  {
    if (!d->isValid()) {
      return;
    }

    glPushAttrib(GL_LIGHTING_BIT);
    glDisable(GL_LIGHTING);

    glLineWidth(lineWidth);
    d->color.apply();

    // Box is:
    //      6------8
    //     /:     /|
    //    / :    / |
    //   /  4---/--7
    //  /  /   /  /
    // 3------5  /
    // | /    | /
    // |/     |/
    // 1------2

    // Near "plane"
    glBegin(GL_LINE_LOOP);
    glVertex3dv(c1.data());
    glVertex3dv(c2.data());
    glVertex3dv(c5.data());
    glVertex3dv(c3.data());
    glEnd();

    // Far "plane"
    glBegin(GL_LINE_LOOP);
    glVertex3dv(c4.data());
    glVertex3dv(c7.data());
    glVertex3dv(c8.data());
    glVertex3dv(c6.data());
    glEnd();

    // Connect
    glBegin(GL_LINES);
    glVertex3dv(c1.data()); glVertex3dv(c4.data());
    glVertex3dv(c2.data()); glVertex3dv(c7.data());
    glVertex3dv(c5.data()); glVertex3dv(c8.data());
    glVertex3dv(c3.data()); glVertex3dv(c6.data());
    glEnd();

    glPopAttrib();
  }

  void GLPainter::drawTorus(const Eigen::Vector3d &,
                            double, double)
  {
  }

  void GLPainter::drawEllipsoid(const Eigen::Vector3d &,
                                const Eigen::Matrix3d &)
  {
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
    // Ensure that the painter is properly initialised
    d->isValid();
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

  void GLPainter::resetName()
  {
    d->type = Primitive::OtherType;
    d->id = -1;
  }

  void GLPainter::popName()
  {
    // Pop the type and id if they are set, then reset them
    if (d->id != -1)
      {
        glPopName();
        glPopName();
        resetName();
      }
  }

  void GLPainter::apply(const Color3f &color)
  {
    glColor3fv(color.data());
  }

  void GLPainter::applyAsMaterials(const Color3f &c, float alpha)
  {
    float color[] = {c.red(), c.green(), c.blue(), alpha};
    float ambientColor [] = {color[0] / 3.0f,
                             color[1] / 3.0f,
                             color[2] / 3.0f,
                             alpha};

    float s = (0.5f + fabs( color[0] - color[1])
              + fabs(color[2] - color[1])
              + fabs(color[2] - color[0])) / 4.0f;

    float t = 1.0 - s;

    float specularColor [] = {s + t * color[0],
                              s + t * color[1],
                              s + t * color[2],
                              alpha};

    glMaterialfv(GL_FRONT, GL_AMBIENT, ambientColor);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, color);
    glMaterialfv(GL_FRONT, GL_SPECULAR, specularColor);
    glMaterialf(GL_FRONT, GL_SHININESS, 50.0);
  }

  void GLPainter::setDynamicScaling(bool scaling)
  {
    m_dynamicScaling = scaling;
  }

} // end namespace Avogadro
