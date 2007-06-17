/**********************************************************************
  AxesEngine - Engine for "axes" display

  Copyright (C) 2007      Marcus D. Hanwell

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

#include "axesengine.h"
#include <config.h>

#include <avogadro/glwidget.h>

#include <eigen/regression.h>

#include <QtPlugin>

using namespace std;
using namespace OpenBabel;
using namespace Avogadro;
using namespace Eigen;

AxesEngine::AxesEngine(QObject *parent) : Engine(parent)
{
  setName(tr("Axes"));
  setDescription(tr("Renders x, y and z axes"));
}

bool AxesEngine::render(GLWidget *gl)
{
  gl->painter()->begin(gl);

  // save the opengl projection matrix and set up an orthogonal projection
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  // Ensure the axes are of the same length
  double aspectRatio = static_cast<double>(gl->width())/static_cast<double>(gl->height());
  glOrtho(0, aspectRatio, 0, 1, 0, 1);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();

  // Don't want any lighting or blending for the axes
  glDisable(GL_LIGHTING);
  glDisable(GL_BLEND);

  // Set the origin and calculate the positions of the axes
  Vector3d origin = Vector3d(0.07, 0.07, -.07);
  MatrixP3d axisTranslation;
  axisTranslation.loadTranslation(gl->camera()->transformedXAxis() * 0.06);
  Vector3d aX = axisTranslation * origin;
  axisTranslation.loadTranslation(gl->camera()->transformedYAxis() * 0.06);
  Vector3d aY = axisTranslation * origin;
  axisTranslation.loadTranslation(gl->camera()->transformedZAxis() * 0.06);
  Vector3d aZ = axisTranslation * origin;

  // Draw the axes in red, green and blue so they can be easily identified
  glBegin(GL_LINES);
  glColor4f(1.0, 0.0, 0.0, 1.);
  glVertex3d(origin.x(), origin.y(), origin.z());
  glVertex3d(aX.x(), aX.y(), aX.z());
  glColor4f(0.0, 1.0, 0.0, 1.);
  glVertex3d(origin.x(), origin.y(), origin.z());
  glVertex3d(aY.x(), aY.y(), aY.z());
  glColor4f(0.0, 0.0, 1.0, 1.);
  glVertex3d(origin.x(), origin.y(), origin.z());
  glVertex3d(aZ.x(), aZ.y(), aZ.z());
  glEnd();
  // FIXME Would be good to draw labels on the axes too, can't figure out
  // how to do that with the current drawText functions in this projection
//  gl->painter()->drawText(aX, "x");

  // restore the original OpenGL projection and lighting
  glEnable(GL_LIGHTING);
  glPopMatrix();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);

  gl->painter()->end();
  return true;
}

#include "axesengine.moc"

Q_EXPORT_PLUGIN2(axesengine, AxesEngineFactory)
