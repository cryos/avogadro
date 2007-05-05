/**********************************************************************
  LabelEngine - Engine for displaying labels.

  Copyright (C) 2006 by Geoffrey R. Hutchison
  Some portions Copyright (C) 2006 by Donald E. Curtis

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.sourceforge.net/>

  Some code is based on Open Babel
  For more information, see <http://openbabel.sourceforge.net/>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation version 2 of the License.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
 ***********************************************************************/

#include "config.h"
#include "labelengine.h"

#include <avogadro/primitive.h>
#include <avogadro/color.h>
#include <avogadro/glwidget.h>
#include <avogadro/camera.h>

#include <openbabel/obiter.h>
#include <eigen/regression.h>

#include <QtPlugin>
#include <QMessageBox>

using namespace std;
using namespace OpenBabel;
using namespace Avogadro;
using namespace Eigen;

bool LabelEngine::render(GLWidget *gl)
{
  QList<Primitive *> list;

  list = queue().primitiveList(Primitive::AtomType);
  glPushAttrib(GL_ALL_ATTRIB_BITS);
  glDisable(GL_LIGHTING);
  foreach( Primitive *p, list ) {
    Atom *atom = static_cast<const Atom *>(p);
    const Vector3d pos = atom->pos();

    double renderRadius = 0.;
    foreach(Engine *engine, gl->engines())
    {
      if(engine->isEnabled())
      {
        double engineRadius = engine->radius(atom);
        if(engineRadius > renderRadius) {
          renderRadius = engineRadius;
        }
      }
    }
    renderRadius += 0.05;

    double zDistance = gl->camera().distance(pos);

    if(zDistance < 50.0) {
      QString str = QString::number(atom->GetIdx());
      const MatrixP3d & m = gl->camera().matrix();

      // compute the unit vector toward the camera, in the molecule's coordinate system.
      // to do this, we apply the inverse of the camera's rotation to the
      // vector (0,0,1). This amount to taking the 3rd column of the
      // inverse of the camera's rotation. But the inverse of a rotation is
      // just its transpose. Thus we want to take the 3rd row of the camera's
      // rotation matrix.
      Vector3d zAxis( m(2,0), m(2,1), m(2,2) );

      Vector3d drawPos = pos + zAxis * renderRadius;

      Vector3d projectedDrawPos = gl->project(drawPos);
      projectedDrawPos.y() = gl->height() - projectedDrawPos.y();
      Vector3d projectedDrawPos_up = projectedDrawPos + Vector3d(0, -1, 0);
      Vector3d projectedDrawPos_down = projectedDrawPos + Vector3d(0, 1, 0);
      Vector3d projectedDrawPos_left = projectedDrawPos + Vector3d(-1, 0, 0);
      Vector3d projectedDrawPos_right = projectedDrawPos + Vector3d(1, 0, 0);

      Vector3d drawPos_up    = gl->unProject( projectedDrawPos_up );
      Vector3d drawPos_down  = gl->unProject( projectedDrawPos_down );
      Vector3d drawPos_left  = gl->unProject( projectedDrawPos_left );
      Vector3d drawPos_right = gl->unProject( projectedDrawPos_right );

      glColor3f(0.0, 0.0, 0.0);
      gl->renderText(drawPos_up.x(), drawPos_up.y(), drawPos_up.z(), str);
      gl->renderText(drawPos_down.x(), drawPos_down.y(), drawPos_down.z(), str);
      gl->renderText(drawPos_left.x(), drawPos_left.y(), drawPos_left.z(), str);
      gl->renderText(drawPos_right.x(), drawPos_right.y(), drawPos_right.z(), str);

      glColor3f(1.0,1.0,1.0);
      gl->renderText(drawPos.x(), drawPos.y(), drawPos.z(), str);
    }
  }
  glPopAttrib();
}

void LabelEngine::options()
{
  QMessageBox::StandardButton ret;
  ret = QMessageBox::information(qobject_cast<QWidget*>(parent()),
                                 tr("Avogadro"),
                                 tr("This will be for render options"),
                                 QMessageBox::Yes | QMessageBox::Cancel);
}

#include "labelengine.moc"

Q_EXPORT_PLUGIN2(labelengine, LabelEngineFactory)
