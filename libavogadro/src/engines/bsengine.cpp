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
#include "bsengine.h"

#include <avogadro/primitive.h>
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
    m_sphere.setup(3);
    m_cylinder.setup(6);
    m_setup = true;
  }

  glDisable( GL_NORMALIZE );
  glEnable( GL_RESCALE_NORMAL );
  list = queue().primitiveList(Primitive::AtomType);
  foreach( Primitive *p, list ) {
    render(static_cast<const Atom *>(p));
  }

  glDisable( GL_RESCALE_NORMAL);
  glEnable( GL_NORMALIZE );

  list = queue().primitiveList(Primitive::BondType);
  foreach( Primitive *p, list ) {
    render(static_cast<const Bond *>(p));
  }

  return true;
}

inline double BSEngine::radius(const Atom *a)
{
  return etab.GetVdwRad(a->GetAtomicNum()) * 0.3;
}

double BSEngine::radius(const Primitive *p)
{
  switch(p->type())
  {
    case Primitive::AtomType:
      return radius(static_cast<const Atom *>(p));
    case Primitive::BondType:
      return 0.1;
    default:
      return 0.;
  }
}

bool BSEngine::render(const Atom *a)
{
  glPushName(Primitive::AtomType);
  glPushName(a->GetIdx());
  Color(a).applyAsMaterials();

  m_sphere.draw(a->GetVector().AsArray(), radius(a));

  if (a->isSelected())
    {
      Color( 0.3, 0.6, 1.0, 0.7 ).applyAsMaterials();
      glEnable( GL_BLEND );
      m_sphere.draw(a->GetVector().AsArray(), 0.18 + radius(a));
      glDisable( GL_BLEND );
    }

  glPopName();
  glPopName();

  return true;
}

bool BSEngine::render(const Bond *b)
{
  Eigen::Vector3d normalVector;

  // FIXME: should be qobject_cast but bug with Qt/Mac
  GLWidget *gl = dynamic_cast<GLWidget *>(parent());
  if(gl) {
    normalVector = gl->normalVector();
  }

  const OBAtom *atom1 = static_cast<const OBAtom *>( b->GetBeginAtom() );
  const OBAtom *atom2 = static_cast<const OBAtom *>( b->GetEndAtom() );

  Vector3d v1 (atom1->GetVector().AsArray());
  Vector3d v2 (atom2->GetVector().AsArray());
  Vector3d v3 (( v1 + v2 ) / 2);
  std::vector<double> rgb;

  double shift = 0.15;
  int order = b->GetBO();

  // for now, just allow selecting atoms
  //  glPushName(bondType);
  //  glPushName(b->GetIdx());
  Color(atom1).applyAsMaterials();
  m_cylinder.draw( v1, v3, radius(b), order, shift, normalVector);

  Color(atom2).applyAsMaterials();
  m_cylinder.draw( v2, v3, radius(b), order, shift, normalVector);
  //  glPopName();
  //  glPopName();

  return true;
}

bool BSEngine::render(const Molecule *m)
{
  // Disabled
  return false;
}

#include "bsengine.moc"

Q_EXPORT_PLUGIN2(bsengine, BSEngineFactory)
