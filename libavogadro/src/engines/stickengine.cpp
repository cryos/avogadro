/**********************************************************************
  StickEngine - Engine for "stick" display

  Copyright (C) 2006-2007 Geoffrey R. Hutchison

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
#include "stickengine.h"

#include <avogadro/primitive.h>
#include <avogadro/color.h>
#include <avogadro/glwidget.h>
#include <avogadro/camera.h>

#include <openbabel/obiter.h>

#include <QtPlugin>
#include <QMessageBox>

using namespace std;
using namespace OpenBabel;
using namespace Eigen;
using namespace Avogadro;

StickEngine::StickEngine(QObject *parent) : Engine(parent)
{
  setName(tr("Stick"));
  setDescription(tr("Renders as Cylinders"));
}

StickEngine::~StickEngine()
{
}

bool StickEngine::renderOpaque(GLWidget *gl)
{
  gl = gl;

  QList<Primitive *> list;

  gl->painter()->begin(gl);

  glPushAttrib(GL_TRANSFORM_BIT);
  glDisable( GL_NORMALIZE );
  glEnable( GL_RESCALE_NORMAL );

  // Build up a list of the atoms and render them
  list = primitives().subList(Primitive::AtomType);
  foreach( Primitive *p, list )
  {
    renderOpaque(gl, static_cast<Atom *>(p));
  }

  list = primitives().subList(Primitive::BondType);

  // render bonds (sticks)
  glDisable( GL_RESCALE_NORMAL );
  glEnable( GL_NORMALIZE );

  // Get a list of bonds and render them
  list = primitives().subList(Primitive::BondType);
  foreach( Primitive *p, list )
  {
    renderOpaque(gl, static_cast<const Bond *>(p));
  }

  glPopAttrib();

  gl->painter()->end();
  return true;
}

bool StickEngine::renderOpaque(GLWidget *gl, const Atom* a)
{
  Color map = colorMap();

  // Push the atom type and name
  glPushName(Primitive::AtomType);
  glPushName(a->GetIdx());

  map.set(a);
  map.applyAsMaterials();

  gl->painter()->drawSphere( a->pos(), radius(a) );

  if (gl->isSelected(a))
  {
    map.set( 0.3, 0.6, 1.0, 0.7 );
    map.applyAsMaterials();
    glEnable( GL_BLEND );
    gl->painter()->drawSphere( a->pos(), SEL_ATOM_EXTRA_RADIUS + radius(a) );
    glDisable( GL_BLEND );
  }

  glPopName();
  glPopName();

  return true;
}

bool StickEngine::renderOpaque(GLWidget *gl, const Bond* b)
{
  Color map = colorMap();

  // Push the type and name
  glPushName(Primitive::BondType);
  glPushName(b->GetIdx()+1);

  const Atom* atom1 = static_cast<const Atom *>(b->GetBeginAtom());
  const Atom* atom2 = static_cast<const Atom *>(b->GetEndAtom());
  Vector3d v1 (atom1->pos());
  Vector3d v2 (atom2->pos());
  Vector3d v3 (( v1 + v2 ) / 2);

  map.set(atom1);
  map.applyAsMaterials();
  gl->painter()->drawCylinder( v1, v3, radius(atom1) );

  map.set(atom2);
  map.applyAsMaterials();
  gl->painter()->drawCylinder( v3, v2, radius(atom1) );

  // Render the selection highlight
  if (gl->isSelected(b))
  {
    map.set( 0.3, 0.6, 1.0, 0.7 );
    map.applyAsMaterials();
    glEnable( GL_BLEND );
    gl->painter()->drawCylinder( v1, v2, SEL_BOND_EXTRA_RADIUS + radius(atom1) );
    glDisable( GL_BLEND );
  }

  glPopName();
  glPopName();

  return true;
}

double StickEngine::radius(const GLWidget *gl, const Primitive *p) const
{
  // Atom radius
  if (p->type() == Primitive::AtomType)
  {
    if (gl)
    {
      if (gl->isSelected(p))
        return radius(static_cast<const Atom *>(p)) + SEL_ATOM_EXTRA_RADIUS;
    }
    return radius(static_cast<const Atom *>(p));
  }
  // Bond radius
  else if (p->type() == Primitive::BondType)
  {
    const Atom* a = static_cast<const Atom *>((static_cast<const Bond *>(p))->GetBeginAtom());
    if (gl)
    {
      if (gl->isSelected(p))
        return radius(a) + SEL_BOND_EXTRA_RADIUS;
    }
    return radius(a);
  }
  // Something else
  else
    return 0.;
}

double StickEngine::transparencyDepth() const
{
  return 0.25;
}

Engine::EngineFlags StickEngine::flags() const
{
  return Engine::Atoms | Engine::Bonds;
}

inline double StickEngine::radius(const Atom*) const
{
  return 0.25;
}

#include "stickengine.moc"

Q_EXPORT_PLUGIN2(stickengine, Avogadro::StickEngineFactory)
