/**********************************************************************
  Eyecandy - Draw arrows etc. inside the Avogadro scene

  Copyright (C) 2007 by Marcus D. Hanwell
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

using namespace Avogadro;
using namespace Eigen;

void Eyecandy::drawRotationHorizRibbon()
{
  glBegin(GL_QUAD_STRIP);
  for(int i = 0; i <= TESS_LEVEL; i++) {
    double alpha = m_xAngleStart + (static_cast<double>(i) / TESS_LEVEL)
                                * (m_xAngleEnd - m_xAngleStart);
    Vector3d v = cos(alpha) * m_xAxis + sin(alpha) * m_zAxis;
    Vector3d v1 = v - RIBBON_WIDTH * m_yAxis;
    Vector3d v2 = v + RIBBON_WIDTH * m_yAxis;
    glNormal3dv(v.array());
    glVertex3dv((m_center + m_renderRadius * v1).array());
    glVertex3dv((m_center + m_renderRadius * v2).array());
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
    glNormal3dv(v.array());
    glVertex3dv((m_center + m_renderRadius * v2).array());
    glVertex3dv((m_center + m_renderRadius * v1).array());
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
  glNormal3dv(v.array());
  glVertex3dv((m_center + m_renderRadius * v1).array());
  glVertex3dv((m_center + m_renderRadius * v3).array());
  glVertex3dv((m_center + m_renderRadius * v2).array());
  glEnd();
}

void Eyecandy::drawRotationRightArrow()
{
  Vector3d v = cos(m_xAngleStart) * m_xAxis + sin(m_xAngleStart) * m_zAxis;
  Vector3d v1 = v - RIBBON_ARROW_WIDTH * m_yAxis;
  Vector3d v2 = v + RIBBON_ARROW_WIDTH * m_yAxis;
  Vector3d v3 = v - RIBBON_ARROW_LENGTH * v.cross(m_yAxis);
  glBegin(GL_TRIANGLES);
  glNormal3dv(v.array());
  glVertex3dv((m_center + m_renderRadius * v1).array());
  glVertex3dv((m_center + m_renderRadius * v3).array());
  glVertex3dv((m_center + m_renderRadius * v2).array());
  glEnd();
}

void Eyecandy::drawRotationUpArrow()
{
  Vector3d v = cos(m_yAngleStart) * m_yAxis + sin(m_yAngleStart) * m_zAxis;
  Vector3d v1 = v - RIBBON_ARROW_WIDTH * m_xAxis;
  Vector3d v2 = v + RIBBON_ARROW_WIDTH * m_xAxis;
  Vector3d v3 = v + RIBBON_ARROW_LENGTH * v.cross(m_xAxis);
  glBegin(GL_TRIANGLES);
  glNormal3dv(v.array());
  glVertex3dv((m_center + m_renderRadius * v1).array());
  glVertex3dv((m_center + m_renderRadius * v2).array());
  glVertex3dv((m_center + m_renderRadius * v3).array());
  glEnd();
}

void Eyecandy::drawRotationDownArrow()
{
  Vector3d v = cos(m_yAngleEnd) * m_yAxis + sin(m_yAngleEnd) * m_zAxis;
  Vector3d v1 = v + RIBBON_ARROW_WIDTH * m_xAxis;
  Vector3d v2 = v - RIBBON_ARROW_WIDTH * m_xAxis;
  Vector3d v3 = v - RIBBON_ARROW_LENGTH * v.cross(m_xAxis);
  glBegin(GL_TRIANGLES);
  glNormal3dv(v.array());
  glVertex3dv((m_center + m_renderRadius * v1).array());
  glVertex3dv((m_center + m_renderRadius * v2).array());
  glVertex3dv((m_center + m_renderRadius * v3).array());
  glEnd();
}

void Eyecandy::drawRotation(GLWidget *widget, Atom *clickedAtom, double xAngle, double yAngle)
{
  if(clickedAtom)
  {
    m_center = clickedAtom->pos();
    m_renderRadius = qMax(widget->radius(clickedAtom) * 1.1 + 0.2,
                          MINIMUM_APPARENT_SIZE * widget->camera()->distance(m_center));
  }
  else
  {
    m_center = widget->center();
    m_renderRadius = qMax(qMax(widget->radius() * 0.7, CAMERA_NEAR_DISTANCE),
                          MINIMUM_APPARENT_SIZE * widget->camera()->distance(m_center));
  }
  
  m_xAngleStart = 2.0 * M_PI * (0.25 + RIBBON_APERTURE) - xAngle;
  m_xAngleEnd = 2.0 * M_PI * (1.25 - RIBBON_APERTURE) - xAngle;
  m_yAngleStart = 2.0 * M_PI * (0.25 + RIBBON_APERTURE) + yAngle;
  m_yAngleEnd = 2.0 * M_PI * (1.25 - RIBBON_APERTURE) + yAngle;
  m_xAxis = widget->camera()->backtransformedXAxis();
  m_yAxis = widget->camera()->backtransformedYAxis();
  m_zAxis = widget->camera()->backtransformedZAxis();
  
  glEnable(GL_BLEND);
  glDepthMask(GL_FALSE);
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
          
  glDisable(GL_BLEND);
  glDepthMask(GL_TRUE);
}

void Eyecandy::drawTranslation(GLWidget *widget, Atom *clickedAtom)
{
  double shift, size;
  if(clickedAtom)
  {
    m_center = clickedAtom->pos();
    size = qMax(widget->radius(clickedAtom) * 1.1 + 0.2,
                MINIMUM_APPARENT_SIZE * widget->camera()->distance(m_center));
    shift = widget->radius(clickedAtom);
  }
  else
  {
    m_center = widget->center();
    size = qMax(qMax(widget->radius(), CAMERA_NEAR_DISTANCE),
                MINIMUM_APPARENT_SIZE * widget->camera()->distance(m_center));
    shift = 0.;
  }
  glEnable(GL_BLEND);
  glDisable(GL_LIGHTING);
  glDepthMask(GL_FALSE);
  m_color.apply();

  // Set up the axes and some vectors to work with
  Vector3d xAxis = widget->camera()->backtransformedXAxis();
  Vector3d yAxis = widget->camera()->backtransformedYAxis();
  Vector3d zAxis = widget->camera()->backtransformedZAxis();
  Vector3d v;
   
  // Horizontal arrow, pointing left
  v = m_center + shift * zAxis;
  glBegin(GL_QUAD_STRIP);
  glVertex3dv((v + RIBBON_WIDTH*size*yAxis).array());
  glVertex3dv((v - RIBBON_WIDTH*size*yAxis).array());
  v += RIBBON_LENGTH * size * xAxis;
  glVertex3dv((v + RIBBON_WIDTH*size*yAxis).array());
  glVertex3dv((v - RIBBON_WIDTH*size*yAxis).array());
  glEnd();
  glBegin(GL_TRIANGLES);
  glVertex3dv((v + RIBBON_ARROW_WIDTH*size*yAxis).array());
  glVertex3dv((v - RIBBON_ARROW_WIDTH*size*yAxis).array());
  glVertex3dv((v + RIBBON_ARROW_LENGTH*size*xAxis).array());
  glEnd();
  // Horizontal arrow, pointing right
  v = m_center + shift*zAxis;
  glBegin(GL_QUAD_STRIP);
  glVertex3dv((v - RIBBON_WIDTH*size*yAxis).array());
  glVertex3dv((v + RIBBON_WIDTH*size*yAxis).array());
  v -= RIBBON_LENGTH*size * xAxis;
  glVertex3dv((v - RIBBON_WIDTH*size*yAxis).array());
  glVertex3dv((v + RIBBON_WIDTH*size*yAxis).array());
  glEnd();
  glBegin(GL_TRIANGLES);
  glVertex3dv((v - RIBBON_ARROW_WIDTH*size*yAxis).array());
  glVertex3dv((v + RIBBON_ARROW_WIDTH*size*yAxis).array());
  glVertex3dv((v - RIBBON_ARROW_LENGTH*size*xAxis).array());
  glEnd();
  // Vertical arrow, pointing up
  v = m_center + shift*zAxis;
  glBegin(GL_QUAD_STRIP);
  glVertex3dv((v - RIBBON_WIDTH*size*xAxis).array());
  glVertex3dv((v + RIBBON_WIDTH*size*xAxis).array());
  v += RIBBON_LENGTH*size * yAxis;
  glVertex3dv((v - RIBBON_WIDTH*size*xAxis).array());
  glVertex3dv((v + RIBBON_WIDTH*size*xAxis).array());
  glEnd();
  glBegin(GL_TRIANGLES);
  glVertex3dv((v - RIBBON_ARROW_WIDTH*size*xAxis).array());
  glVertex3dv((v + RIBBON_ARROW_WIDTH*size*xAxis).array());
  glVertex3dv((v + RIBBON_ARROW_LENGTH*size*yAxis).array());
  glEnd();
  // Vertical arrow, pointing down
  v = m_center + shift*zAxis;
  glBegin(GL_QUAD_STRIP);
  glVertex3dv((v + RIBBON_WIDTH*size*xAxis).array());
  glVertex3dv((v - RIBBON_WIDTH*size*xAxis).array());
  v -= RIBBON_LENGTH*size * yAxis;
  glVertex3dv((v + RIBBON_WIDTH*size*xAxis).array());
  glVertex3dv((v - RIBBON_WIDTH*size*xAxis).array());
  glEnd();
  glBegin(GL_TRIANGLES);
  glVertex3dv((v + RIBBON_ARROW_WIDTH*size*xAxis).array());
  glVertex3dv((v - RIBBON_ARROW_WIDTH*size*xAxis).array());
  glVertex3dv((v - RIBBON_ARROW_LENGTH*size*yAxis).array());
  glEnd();

  glDisable(GL_BLEND);
  glEnable(GL_LIGHTING);
  glDepthMask(GL_TRUE);
}

void Eyecandy::drawZoom(GLWidget *widget, Atom *clickedAtom)
{
  widget->painter()->setColor(&m_color);
  if(clickedAtom) {
    double renderRadius = qMax(widget->radius(clickedAtom) * 1.1 + 0.2,
                            MINIMUM_APPARENT_SIZE * 0.3 * widget->camera()->distance(clickedAtom->pos()));
    glEnable( GL_BLEND );
    widget->painter()->drawSphere(clickedAtom->pos(), renderRadius);
    glDisable( GL_BLEND );
  }
  else
  {
    // zoom with respect to molecule's center: let's not draw any eyecandy
    // as I can't think of any that would be useful.
  }
}