/**********************************************************************
  WireEngine - Engine for wireframe display

  Copyright (C) 2006-2007 Geoffrey R. Hutchison
  Copyright (C) 2006-2007 Benoit Jacob

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

#include <config.h>
#include "wireengine.h"

#include <avogadro/primitive.h>
#include <avogadro/color.h>
#include <avogadro/glwidget.h>

#include <openbabel/obiter.h>
#include <eigen/regression.h>

#include <QtPlugin>
#include <QMessageBox>

using namespace std;
using namespace OpenBabel;
using namespace Eigen;

namespace Avogadro {

  WireEngine::WireEngine(QObject *parent) : Engine(parent)
  {
    setName(tr("Wireframe"));
    setDescription(tr("Wireframe rendering"));
  }

  bool WireEngine::renderOpaque(PainterDevice *pd)
  {
    QList<Primitive *> list;

    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glDisable(GL_LIGHTING);
    glDisable(GL_BLEND);

    list = primitives().subList(Primitive::AtomType);
    foreach( Primitive *p, list ) {
      renderOpaque(pd, static_cast<const Atom *>(p));
    }

    list = primitives().subList(Primitive::BondType);
    foreach( Primitive *p, list ) {
      renderOpaque(pd, static_cast<const Bond *>(p));
    }

    glPopAttrib();

    return true;
  }

  bool WireEngine::renderOpaque(PainterDevice *pd, const Atom *a)
  {
    const Vector3d & v = a->pos();
    const Camera *camera = pd->camera();

    Eigen::Vector3d transformedPos = pd->camera()->modelview() * v;

    // perform a rough form of frustum culling
    double dot = transformedPos.z() / transformedPos.norm();
    if(dot > -0.8) return true;

    Color map = colorMap();
    const float selectionColor[3] = {0.3, 0.6, 1.0};
    glPushName(Primitive::AtomType);
    glPushName(a->GetIdx());

    double size = 3.0; // default size;
    if (camera->distance(v) < 10.0)
      size = 4.0;
    else if (camera->distance(v) > 40.0 && camera->distance(v) < 85.0)
      size = 2.0;
    else if (camera->distance(v) > 85.0)
      size = 1.5;

    if (pd->isSelected(a)) {
      glColor3fv(selectionColor);
      glPointSize(etab.GetVdwRad(a->GetAtomicNum()) * (size + 1.0));
    }
    else{
      map.set(a);
      map.apply();
      glPointSize(etab.GetVdwRad(a->GetAtomicNum()) * size);
    }

    glBegin(GL_POINTS);
    glVertex3d(v.x(), v.y(), v.z());
    glEnd();

    glPopName();
    glPopName();

    return true;
  }

  bool WireEngine::renderOpaque(PainterDevice *pd, const Bond *b)
  {
    const Atom *atom1 = static_cast<const Atom *>( b->GetBeginAtom() );
    const Vector3d & v1 = atom1->pos();
    const Camera *camera = pd->camera();

    Eigen::Vector3d transformedEnd1 = pd->camera()->modelview() * v1;

    // perform a rough form of frustum culling
    double dot = transformedEnd1.z() / transformedEnd1.norm();
    if(dot > -0.8) return true;

    const Atom *atom2 = static_cast<const Atom *>( b->GetEndAtom() );
    const Vector3d & v2 = atom2->pos();

    Color map = colorMap();

    double width = 1.0;
    double averageDistance = (camera->distance(v1) + camera->distance(v2)) / 2.0;

    if (averageDistance < 10.0 && averageDistance > 5.0)
      width = 2.0;
    else if (averageDistance < 5.0)
      width = 2.5;

    glLineWidth(width);
    glBegin(GL_LINES);

    // hard to separate atoms from bonds in this view
    // so we let the user always select atoms
    map.set(atom1);
    map.apply();
    glVertex3d(v1.x(), v1.y(), v1.z());

    map.set(atom2);
    map.apply();
    glVertex3d(v2.x(), v2.y(), v2.z());

    glEnd();

    return true;
  }
}

#include "wireengine.moc"

Q_EXPORT_PLUGIN2(wireengine, Avogadro::WireEngineFactory)
