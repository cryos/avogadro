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

    //Color(atom).applyAsMaterials();
    glColor3f(1.0,1.0,1.0);
    double zDistance = gl->camera().distance(pos);

    if(zDistance < 50.0) {
      const MatrixP3d & m = gl->camera().matrix();

      // compute the unit vector toward the camera, in the molecule's coordinate system.
      // to do this, we apply the inverse of the camera's rotation to the
      // vector (0,0,1). This amount to taking the 3rd column of the
      // inverse of the camera's rotation. But the inverse of a rotation is
      // just its transpose. Thus we want to take the 3rd row of the camera's
      // rotation matrix.
      Vector3d zAxis( m(2,0), m(2,1), m(2,2) );

      Vector3d drawPos = pos + zAxis * renderRadius;
      gl->renderText(drawPos.x(), drawPos.y(), drawPos.z(), 
          QString::number(atom->GetIdx()));
    }
  }
  glPopAttrib();
}

#include "labelengine.moc"

Q_EXPORT_PLUGIN2(labelengine, LabelEngineFactory)
