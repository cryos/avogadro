/**********************************************************************
  BSEngine - Engine for "balls and sticks" display

  Copyright (C) 2006 by Geoffrey R. Hutchison
  Some portions Copyright (C) 2006 by Donald E. Curtis
  Some portions Copyright (C) 2007 by Marcus D. Hanwell

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
#include "sphereengine.h"

#include <avogadro/primitives.h>
#include <avogadro/color.h>
#include <avogadro/glwidget.h>

#include <openbabel/obiter.h>
#include <eigen/regression.h>

#include <QtGui>

using namespace std;
using namespace OpenBabel;
using namespace Avogadro;
using namespace Eigen;

bool BSEngine::render(GLWidget *gl)
{
  QList<Primitive *> list;

  if (!m_setup) {
    m_sphere.setup(6);
    m_setup = true;
  }

  glDisable( GL_NORMALIZE );
  glEnable( GL_RESCALE_NORMAL );
  list = queue().primitiveList(Primitive::AtomType);
  foreach( Primitive *p, list ) {
    render((Atom *) p);
  }

  glDisable( GL_RESCALE_NORMAL);
  glEnable( GL_NORMALIZE );

  return true;
}

bool BSEngine::render(const Atom *a)
{
  glPushName(Primitive::AtomType);
  glPushName(a->GetIdx());
  Color(a).applyAsMaterials();

  m_sphere.draw(a->GetVector().AsArray(), etab.GetVdwRad(a->GetAtomicNum()));

  if (a->isSelected())
    {
      Color( 0.3, 0.6, 1.0, 0.7 ).applyAsMaterials();
      glEnable( GL_BLEND );
      m_sphere.draw(a->GetVector().AsArray(), 0.18 + etab.GetVdwRad(a->GetAtomicNum()));
      glDisable( GL_BLEND );
    }

  glPopName();
  glPopName();

  return true;
}

bool BSEngine::render(const Bond *b)
{
  // Disabled
  return false;
}

bool BSEngine::render(const Molecule *m)
{
  // Disabled
  return false;
}

#include "sphereengine.moc"

Q_EXPORT_PLUGIN2(sphereengine, BSEngineFactory)
