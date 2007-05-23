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

StickEngine::StickEngine(QObject *parent) : Engine(parent), m_glwidget(0), m_update(true)
{
  setName(tr("Stick"));
  setDescription(tr("Renders as Cylinders"));
}

StickEngine::~StickEngine()
{
}

bool StickEngine::render(GLWidget *gl)
{
  m_glwidget = gl;
  Color map = colorMap();

  m_glwidget->painter()->begin(m_glwidget);

  QList<Primitive *> list;

  m_update = false;
  glPushAttrib(GL_TRANSFORM_BIT);
  glDisable( GL_NORMALIZE );
  glEnable( GL_RESCALE_NORMAL );
  list = primitives().subList(Primitive::AtomType);
  glPushName(Primitive::AtomType);
  foreach( Primitive *p, list ) {
    Atom * a = static_cast<Atom *>(p);
    glPushName(a->GetIdx());

    map.set(a);
    map.applyAsMaterials();

    m_glwidget->painter()->drawSphere( a->pos(), radius(a) );

    if (m_glwidget->isSelected(a))
    {
      map.set( 0.3, 0.6, 1.0, 0.7 );
      map.applyAsMaterials();
      glEnable( GL_BLEND );
      m_glwidget->painter()->drawSphere( a->pos(), SEL_ATOM_EXTRA_RADIUS + radius(a) );
      glDisable( GL_BLEND );
    }
    glPopName();
  }
  glPopName();

  list = primitives().subList(Primitive::BondType);
  Eigen::Vector3d normalVector;

  // render bonds (sticks)
  glDisable( GL_RESCALE_NORMAL );
  glEnable( GL_NORMALIZE );
  
  foreach( Primitive *p, list ) {
    const Bond *b = static_cast<const Bond *>(p);

    const Atom* atom1 = static_cast<const Atom *>(b->GetBeginAtom());
    const Atom* atom2 = static_cast<const Atom *>(b->GetEndAtom());
    Vector3d v1 (atom1->pos());
    Vector3d v2 (atom2->pos());
    Vector3d v3 (( v1 + v2 ) / 2);

    map.set(atom1);
    map.applyAsMaterials();
    m_glwidget->painter()->drawCylinder( v1, v3, radius(atom1) );

    map.set(atom2);
    map.applyAsMaterials();
    m_glwidget->painter()->drawCylinder( v3, v2, radius(atom1) );
  }

  glPopAttrib();

  m_glwidget->painter()->end();
  return true;
}

double StickEngine::radius(const Primitive *p)
{
  if (p->type() == Primitive::AtomType)
  {
    const Atom *a = static_cast<const Atom *>(p);
    double r = radius(a);
    if (m_glwidget)
    {
      if (m_glwidget->isSelected(p))
        return r + SEL_ATOM_EXTRA_RADIUS;
    }
    return r;
  }
  else
    return 0.;
}

inline double StickEngine::radius(const Atom*)
{
  return 0.25;
}

bool StickEngine::render(const Atom*)
{
  return true;
}

bool StickEngine::render(const Bond*)
{
  return true;
}

void StickEngine::addPrimitive(Primitive *primitive)
{
  Engine::addPrimitive(primitive);
  m_update = true;
}

void StickEngine::updatePrimitive(Primitive *primitive)
{
  Engine::updatePrimitive(primitive);
  m_update = true;
}

void StickEngine::removePrimitive(Primitive *primitive)
{
  Engine::removePrimitive(primitive);
  m_update = true;
}

bool StickEngine::render(const Molecule*)
{
  // Disabled
  return false;
}

#include "stickengine.moc"

Q_EXPORT_PLUGIN2(stickengine, Avogadro::StickEngineFactory)
