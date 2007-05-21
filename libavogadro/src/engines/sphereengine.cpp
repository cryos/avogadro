/**********************************************************************
  SphereEngine - Engine for "spheres" display

  Copyright (C) 2006-2007 Geoffrey R. Hutchison
  Copyright (C) 2007      Benoit Jacob
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

#include "sphereengine.h"
#include <config.h>

#include <avogadro/primitive.h>
#include <avogadro/color.h>
#include <avogadro/glwidget.h>

#include <openbabel/obiter.h>
#include <eigen/regression.h>

#include <QtPlugin>
#include <QMessageBox>

using namespace std;
using namespace OpenBabel;
using namespace Avogadro;
using namespace Eigen;

SphereEngine::SphereEngine(QObject *parent) : Engine(parent), m_setup(false), m_glwidget(0) 
{
  setName(tr("Sphere"));
  setDescription(tr("Renders atoms as spheres"));
}

bool SphereEngine::render(GLWidget *gl)
{
  m_glwidget = gl;
  m_glwidget->painter()->begin(m_glwidget);

  QList<Primitive *> list;

  if (!m_setup) {
    m_setup = true;
  }

  glDisable( GL_NORMALIZE );
  glEnable( GL_RESCALE_NORMAL );
  list = primitiveList().subList(Primitive::AtomType);
  foreach( Primitive *p, list ) {
    render(static_cast<const Atom *>(p));
  }

  glDisable( GL_RESCALE_NORMAL);
  glEnable( GL_NORMALIZE );

  gl->painter()->end();
  return true;
}

inline double SphereEngine::radius(const Atom *a)
{
  return etab.GetVdwRad(a->GetAtomicNum());
}

double SphereEngine::radius(const Primitive *p)
{
  if (p->type() == Primitive::AtomType)
  {
    const Atom *a = static_cast<const Atom *>(p);
    double r = radius(a);
    if (m_glwidget)
    {
      if (m_glwidget->selectedItem(p))
        return r + SEL_ATOM_EXTRA_RADIUS;
    }
    return r;
  }
  else
    return 0.;
}

bool SphereEngine::render(const Atom *a)
{
  Color map = colorMap();
  glPushName(Primitive::AtomType);
  glPushName(a->GetIdx());
  map.set(a);
  map.applyAsMaterials();

  m_glwidget->painter()->drawSphere( a->pos(), radius(a) );

  if (m_glwidget->selectedItem(a))
    {
      map.set( 0.3, 0.6, 1.0, 0.7 );
      map.applyAsMaterials();
      glEnable( GL_BLEND );
      m_glwidget->painter()->drawSphere( a->pos(), SEL_ATOM_EXTRA_RADIUS + radius(a) );
      glDisable( GL_BLEND );
    }

  glPopName();
  glPopName();

  return true;
}

#include "sphereengine.moc"

Q_EXPORT_PLUGIN2(sphereengine, SphereEngineFactory)
