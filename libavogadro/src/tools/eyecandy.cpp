/**********************************************************************
  Eyecandy - Draw arrows etc. inside the Avogadro scene

  Copyright (C) 2007,2008 by Marcus D. Hanwell
  Copyright (C) 2006,2007 by Benoit Jacob

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

#include "eyecandy.h"

#define TESS_LEVEL 32
#define RIBBON_WIDTH 0.05
#define RIBBON_LENGTH 0.6
#define RIBBON_ARROW_WIDTH 0.15
#define RIBBON_ARROW_LENGTH 0.25
#define RIBBON_APERTURE 0.07
#define MINIMUM_APPARENT_SIZE 0.04
#define MAXIMUM_APPARENT_SIZE 0.25
#define SIZE_FACTOR_WHEN_NOTHING_CLICKED 0.25
#define ZOOM_SIZE_FACTOR 0.3
#define ATOM_SIZE_FACTOR 1.1

using namespace Eigen;

namespace Avogadro {

  void Eyecandy::drawRotationHorizRibbon()
  {
    glBegin(GL_QUAD_STRIP);
    for(int i = 0; i <= TESS_LEVEL; i++) {
      double alpha = m_xAngleStart + (static_cast<double>(i) / TESS_LEVEL)
        * (m_xAngleEnd - m_xAngleStart);
      Vector3d v = cos(alpha) * m_xAxis + sin(alpha) * m_zAxis;
      Vector3d v1 = v - RIBBON_WIDTH * m_yAxis;
      Vector3d v2 = v + RIBBON_WIDTH * m_yAxis;
      glNormal3dv(v.data());
      glVertex3dv((m_center + m_radius * v1).eval().data());
      glVertex3dv((m_center + m_radius * v2).eval().data());
    }
    glEnd();
  }

  void Eyecandy::drawRotationVertRibbon()
  {
    glBegin(GL_QUAD_STRIP);
    for(int i = 0; i <= TESS_LEVEL; i++) {
      double alpha = m_yAngleStart + (static_cast<double>(i) / TESS_LEVEL)
        * (m_yAngleEnd - m_yAngleStart);
      Vector3d v = cos(alpha) * m_yAxis + sin(alpha) * m_zAxis;
      Vector3d v1 = v - RIBBON_WIDTH * m_xAxis;
      Vector3d v2 = v + RIBBON_WIDTH * m_xAxis;
      glNormal3dv(v.data());
      glVertex3dv((m_center + m_radius * v2).eval().data());
      glVertex3dv((m_center + m_radius * v1).eval().data());
    }
    glEnd();
  }

  void Eyecandy::drawRotationLeftArrow()
  {
    Vector3d v = cos(m_xAngleEnd) * m_xAxis + sin(m_xAngleEnd) * m_zAxis;
    Vector3d v1 = v + RIBBON_ARROW_WIDTH * m_yAxis;
    Vector3d v2 = v - RIBBON_ARROW_WIDTH * m_yAxis;
    Vector3d v3 = v + RIBBON_ARROW_LENGTH * v.cross(m_yAxis);
    glBegin(GL_TRIANGLES);
    glNormal3dv(v.data());
    glVertex3dv((m_center + m_radius * v1).eval().data());
    glVertex3dv((m_center + m_radius * v3).eval().data());
    glVertex3dv((m_center + m_radius * v2).eval().data());
    glEnd();
  }

  void Eyecandy::drawRotationRightArrow()
  {
    Vector3d v = cos(m_xAngleStart) * m_xAxis + sin(m_xAngleStart) * m_zAxis;
    Vector3d v1 = v - RIBBON_ARROW_WIDTH * m_yAxis;
    Vector3d v2 = v + RIBBON_ARROW_WIDTH * m_yAxis;
    Vector3d v3 = v - RIBBON_ARROW_LENGTH * v.cross(m_yAxis);
    glBegin(GL_TRIANGLES);
    glNormal3dv(v.data());
    glVertex3dv((m_center + m_radius * v1).eval().data());
    glVertex3dv((m_center + m_radius * v3).eval().data());
    glVertex3dv((m_center + m_radius * v2).eval().data());
    glEnd();
  }

  void Eyecandy::drawRotationUpArrow()
  {
    Vector3d v = cos(m_yAngleStart) * m_yAxis + sin(m_yAngleStart) * m_zAxis;
    Vector3d v1 = v - RIBBON_ARROW_WIDTH * m_xAxis;
    Vector3d v2 = v + RIBBON_ARROW_WIDTH * m_xAxis;
    Vector3d v3 = v + RIBBON_ARROW_LENGTH * v.cross(m_xAxis);
    glBegin(GL_TRIANGLES);
    glNormal3dv(v.data());
    glVertex3dv((m_center + m_radius * v1).eval().data());
    glVertex3dv((m_center + m_radius * v2).eval().data());
    glVertex3dv((m_center + m_radius * v3).eval().data());
    glEnd();
  }

  void Eyecandy::drawRotationDownArrow()
  {
    Vector3d v = cos(m_yAngleEnd) * m_yAxis + sin(m_yAngleEnd) * m_zAxis;
    Vector3d v1 = v + RIBBON_ARROW_WIDTH * m_xAxis;
    Vector3d v2 = v - RIBBON_ARROW_WIDTH * m_xAxis;
    Vector3d v3 = v - RIBBON_ARROW_LENGTH * v.cross(m_xAxis);
    glBegin(GL_TRIANGLES);
    glNormal3dv(v.data());
    glVertex3dv((m_center + m_radius * v1).eval().data());
    glVertex3dv((m_center + m_radius * v2).eval().data());
    glVertex3dv((m_center + m_radius * v3).eval().data());
    glEnd();
  }

  void Eyecandy::drawRotation(GLWidget *widget, Atom *clickedAtom, double xAngle, double yAngle, const Eigen::Vector3d &center)
  {
    if(clickedAtom)
    {
      drawRotation(widget, clickedAtom->pos(),
          qMax(widget->radius(clickedAtom) * ATOM_SIZE_FACTOR,
            MINIMUM_APPARENT_SIZE * widget->camera()->distance(center)),
          xAngle, yAngle);
    }
    else
    {
      drawRotation(widget, center,
          qMin(
            qMax(
              qMax(widget->radius() * SIZE_FACTOR_WHEN_NOTHING_CLICKED, CAMERA_NEAR_DISTANCE),
              MINIMUM_APPARENT_SIZE * widget->camera()->distance(center)),
            MAXIMUM_APPARENT_SIZE * widget->camera()->distance(center)),
          xAngle, yAngle);
    }
  }

  void Eyecandy::drawRotation(GLWidget *widget, const Eigen::Vector3d& center, double radius, double xAngle, double yAngle)
  {
    m_center = center;
    m_radius = radius;
    m_xAngleStart = 2.0 * M_PI * (0.25 + RIBBON_APERTURE) - xAngle;
    m_xAngleEnd = 2.0 * M_PI * (1.25 - RIBBON_APERTURE) - xAngle;
    m_yAngleStart = 2.0 * M_PI * (0.25 + RIBBON_APERTURE) + yAngle;
    m_yAngleEnd = 2.0 * M_PI * (1.25 - RIBBON_APERTURE) + yAngle;
    m_xAxis = widget->camera()->backTransformedXAxis();
    m_yAxis = widget->camera()->backTransformedYAxis();
    m_zAxis = widget->camera()->backTransformedZAxis();

//    glEnable(GL_BLEND);
//    glDepthMask(GL_FALSE);
    m_color.applyAsMaterials();

    //draw back faces
    glCullFace(GL_FRONT);
    drawRotationHorizRibbon();
    drawRotationVertRibbon();
    drawRotationRightArrow();
    drawRotationLeftArrow();
    drawRotationUpArrow();
    drawRotationDownArrow();

    //draw front faces
    glCullFace(GL_BACK); // this restores the default culling behaviour
    drawRotationHorizRibbon();
    drawRotationVertRibbon();
    drawRotationRightArrow();
    drawRotationLeftArrow();
    drawRotationUpArrow();
    drawRotationDownArrow();

//    glDisable(GL_BLEND);
//    glDepthMask(GL_TRUE);
  }

  void Eyecandy::drawTranslation(GLWidget *widget, Atom *clickedAtom, const Eigen::Vector3d &center)
  {
    if(clickedAtom)
    {
      drawTranslation(widget, center,
        qMax(widget->radius(clickedAtom) * ATOM_SIZE_FACTOR,
             MINIMUM_APPARENT_SIZE * widget->camera()->distance(center)),
          widget->radius(clickedAtom));
    }
    else
    {
      drawTranslation(widget, center, qMin(
        qMax(
          qMax(widget->radius() * SIZE_FACTOR_WHEN_NOTHING_CLICKED, CAMERA_NEAR_DISTANCE),
              MINIMUM_APPARENT_SIZE * widget->camera()->distance(center)),
            MAXIMUM_APPARENT_SIZE * widget->camera()->distance(center)),
          0.);
    }
  }
  void Eyecandy::drawTranslation(GLWidget *widget, const Eigen::Vector3d& center, double size, double shift)
  {
//    glEnable(GL_BLEND);
    glDisable(GL_LIGHTING);
//    glDepthMask(GL_FALSE);
    m_color.apply();

    // Set up the axes and some vectors to work with
    Vector3d xAxis = widget->camera()->backTransformedXAxis();
    Vector3d yAxis = widget->camera()->backTransformedYAxis();
    Vector3d zAxis = widget->camera()->backTransformedZAxis();
    Vector3d v;

    // Horizontal arrow, pointing left
    v = center + shift * zAxis;
    glBegin(GL_QUAD_STRIP);
    glVertex3dv((v + RIBBON_WIDTH*size*yAxis).eval().data());
    glVertex3dv((v - RIBBON_WIDTH*size*yAxis).eval().data());
    v += RIBBON_LENGTH * size * xAxis;
    glVertex3dv((v + RIBBON_WIDTH*size*yAxis).eval().data());
    glVertex3dv((v - RIBBON_WIDTH*size*yAxis).eval().data());
    glEnd();
    glBegin(GL_TRIANGLES);
    glVertex3dv((v + RIBBON_ARROW_WIDTH*size*yAxis).eval().data());
    glVertex3dv((v - RIBBON_ARROW_WIDTH*size*yAxis).eval().data());
    glVertex3dv((v + RIBBON_ARROW_LENGTH*size*xAxis).eval().data());
    glEnd();
    // Horizontal arrow, pointing right
    v = center + shift*zAxis;
    glBegin(GL_QUAD_STRIP);
    glVertex3dv((v - RIBBON_WIDTH*size*yAxis).eval().data());
    glVertex3dv((v + RIBBON_WIDTH*size*yAxis).eval().data());
    v -= RIBBON_LENGTH*size * xAxis;
    glVertex3dv((v - RIBBON_WIDTH*size*yAxis).eval().data());
    glVertex3dv((v + RIBBON_WIDTH*size*yAxis).eval().data());
    glEnd();
    glBegin(GL_TRIANGLES);
    glVertex3dv((v - RIBBON_ARROW_WIDTH*size*yAxis).eval().data());
    glVertex3dv((v + RIBBON_ARROW_WIDTH*size*yAxis).eval().data());
    glVertex3dv((v - RIBBON_ARROW_LENGTH*size*xAxis).eval().data());
    glEnd();
    // Vertical arrow, pointing up
    v = center + shift*zAxis;
    glBegin(GL_QUAD_STRIP);
    glVertex3dv((v - RIBBON_WIDTH*size*xAxis).eval().data());
    glVertex3dv((v + RIBBON_WIDTH*size*xAxis).eval().data());
    v += RIBBON_LENGTH*size * yAxis;
    glVertex3dv((v - RIBBON_WIDTH*size*xAxis).eval().data());
    glVertex3dv((v + RIBBON_WIDTH*size*xAxis).eval().data());
    glEnd();
    glBegin(GL_TRIANGLES);
    glVertex3dv((v - RIBBON_ARROW_WIDTH*size*xAxis).eval().data());
    glVertex3dv((v + RIBBON_ARROW_WIDTH*size*xAxis).eval().data());
    glVertex3dv((v + RIBBON_ARROW_LENGTH*size*yAxis).eval().data());
    glEnd();
    // Vertical arrow, pointing down
    v = center + shift*zAxis;
    glBegin(GL_QUAD_STRIP);
    glVertex3dv((v + RIBBON_WIDTH*size*xAxis).eval().data());
    glVertex3dv((v - RIBBON_WIDTH*size*xAxis).eval().data());
    v -= RIBBON_LENGTH*size * yAxis;
    glVertex3dv((v + RIBBON_WIDTH*size*xAxis).eval().data());
    glVertex3dv((v - RIBBON_WIDTH*size*xAxis).eval().data());
    glEnd();
    glBegin(GL_TRIANGLES);
    glVertex3dv((v + RIBBON_ARROW_WIDTH*size*xAxis).eval().data());
    glVertex3dv((v - RIBBON_ARROW_WIDTH*size*xAxis).eval().data());
    glVertex3dv((v - RIBBON_ARROW_LENGTH*size*yAxis).eval().data());
    glEnd();

//    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
//    glDepthMask(GL_TRUE);
  }

  void Eyecandy::drawZoom(GLWidget *widget, Atom *clickedAtom, const Eigen::Vector3d &center)
  {
    if(clickedAtom) {
      drawZoom(widget, center,
          widget->radius(clickedAtom) *  2);
    }
    else
    {
      // zoom with respect to molecule's center: let's not draw any eyecandy
      // as I can't think of any that would be useful.
      drawZoom(widget, center, widget->radius());
      //     qMin(
      //         qMax(
      //           qMax(widget->radius() * SIZE_FACTOR_WHEN_NOTHING_CLICKED, CAMERA_NEAR_DISTANCE),
      //           MINIMUM_APPARENT_SIZE * widget->camera()->distance(center)),
      //         MAXIMUM_APPARENT_SIZE * widget->camera()->distance(center)));
    }
  }

  void Eyecandy::drawZoom(GLWidget *widget, const Eigen::Vector3d& center, double size)
  {
    widget->painter()->setColor(&m_color);
    //   glEnable( GL_BLEND );
    //   widget->painter()->drawSphere(center, radius);
    //   glDisable( GL_BLEND );
//    glEnable(GL_BLEND);
    glDisable(GL_LIGHTING);
//    glDepthMask(GL_FALSE);
    //draw back faces
    glDisable(GL_CULL_FACE);
    m_color.apply();

    // Set up the axes and some vectors to work with
    Vector3d xAxis = widget->camera()->backTransformedXAxis();
    Vector3d yAxis = widget->camera()->backTransformedYAxis();
    Vector3d zAxis = widget->camera()->backTransformedZAxis();
    Vector3d v;

    // Horizontal arrow, pointing left
    v = center; // * zAxis;
    glBegin(GL_QUAD_STRIP);
    glVertex3dv((v + RIBBON_WIDTH*size*yAxis).eval().data());
    glVertex3dv((v - RIBBON_WIDTH*size*yAxis).eval().data());
    v += RIBBON_LENGTH * size * zAxis;
    glVertex3dv((v + RIBBON_WIDTH*size*yAxis).eval().data());
    glVertex3dv((v - RIBBON_WIDTH*size*yAxis).eval().data());
    glEnd();
    glBegin(GL_TRIANGLES);
    glVertex3dv((v + RIBBON_ARROW_WIDTH*size*yAxis).eval().data());
    glVertex3dv((v - RIBBON_ARROW_WIDTH*size*yAxis).eval().data());
    glVertex3dv((v + RIBBON_ARROW_LENGTH*size*zAxis).eval().data());
    glEnd();
    // Horizontal arrow, pointing right
    v = center; // + shift*zAxis;
    glBegin(GL_QUAD_STRIP);
    glVertex3dv((v - RIBBON_WIDTH*size*yAxis).eval().data());
    glVertex3dv((v + RIBBON_WIDTH*size*yAxis).eval().data());
    v -= RIBBON_LENGTH*size * zAxis;
    glVertex3dv((v - RIBBON_WIDTH*size*yAxis).eval().data());
    glVertex3dv((v + RIBBON_WIDTH*size*yAxis).eval().data());
    glEnd();
    glBegin(GL_TRIANGLES);
    glVertex3dv((v - RIBBON_ARROW_WIDTH*size*yAxis).eval().data());
    glVertex3dv((v + RIBBON_ARROW_WIDTH*size*yAxis).eval().data());
    glVertex3dv((v - RIBBON_ARROW_LENGTH*size*zAxis).eval().data());
    glEnd();
    // Vertical arrow, pointing up
    v = center; // + shift*zAxis;
    glBegin(GL_QUAD_STRIP);
    glVertex3dv((v - RIBBON_WIDTH*size*xAxis).eval().data());
    glVertex3dv((v + RIBBON_WIDTH*size*xAxis).eval().data());
    v += RIBBON_LENGTH*size * zAxis;
    glVertex3dv((v - RIBBON_WIDTH*size*xAxis).eval().data());
    glVertex3dv((v + RIBBON_WIDTH*size*xAxis).eval().data());
    glEnd();
    glBegin(GL_TRIANGLES);
    glVertex3dv((v - RIBBON_ARROW_WIDTH*size*xAxis).eval().data());
    glVertex3dv((v + RIBBON_ARROW_WIDTH*size*xAxis).eval().data());
    glVertex3dv((v + RIBBON_ARROW_LENGTH*size*zAxis).eval().data());
    glEnd();
    // Vertical arrow, pointing down
    v = center; // + shift*zAxis;
    glBegin(GL_QUAD_STRIP);
    glVertex3dv((v + RIBBON_WIDTH*size*xAxis).eval().data());
    glVertex3dv((v - RIBBON_WIDTH*size*xAxis).eval().data());
    v -= RIBBON_LENGTH*size * zAxis;
    glVertex3dv((v + RIBBON_WIDTH*size*xAxis).eval().data());
    glVertex3dv((v - RIBBON_WIDTH*size*xAxis).eval().data());
    glEnd();
    glBegin(GL_TRIANGLES);
    glVertex3dv((v + RIBBON_ARROW_WIDTH*size*xAxis).eval().data());
    glVertex3dv((v - RIBBON_ARROW_WIDTH*size*xAxis).eval().data());
    glVertex3dv((v - RIBBON_ARROW_LENGTH*size*zAxis).eval().data());
    glEnd();

    //draw back faces
    glEnable(GL_CULL_FACE);
//    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
//    glDepthMask(GL_TRUE);
  }

  void Eyecandy::setColor(const Color &color)
  {
    m_color = color;
  }

  Color Eyecandy::color() const
  {
    return m_color;
  }
}
