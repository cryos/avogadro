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

#include <QtPlugin>
#include <QMessageBox>

using namespace std;
using namespace OpenBabel;
using namespace Avogadro;
using namespace Eigen;

const int BSENGINE_DETAIL_LEVEL = 4;

bool BSEngine::render(GLWidget *gl)
{
  QList<Primitive *> list;

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
  double r;
  switch(p->type())
  {
    case Primitive::AtomType:
      r = radius(static_cast<const Atom *>(p));
      if(p->isSelected())
      {
        return r + SEL_ATOM_EXTRA_RADIUS;
      }
      return r;
      break;
    case Primitive::BondType:
      return 0.1;
      break;
    default:
      return 0.;
      break;
  }
}

bool BSEngine::render(const Atom *a)
{
  // FIXME: should be qobject_cast but bug with Qt/Mac
  GLWidget *gl = dynamic_cast<GLWidget *>(parent());

  Color map = colorMap();

  glPushName(Primitive::AtomType);
  glPushName(a->GetIdx());
  map.set(a);
  map.applyAsMaterials();

  gl->painter()->drawSphere( a->pos(), radius(a), BSENGINE_DETAIL_LEVEL );

  if (a->isSelected())
    {
      map.set( 0.3, 0.6, 1.0, 0.7 );
      map.applyAsMaterials();
      glEnable( GL_BLEND );
      gl->painter()->drawSphere( a->pos(), SEL_ATOM_EXTRA_RADIUS + radius(a), BSENGINE_DETAIL_LEVEL );
      glDisable( GL_BLEND );
    }

  glPopName();
  glPopName();

  return true;
}

bool BSEngine::render(const Bond *b)
{
  Eigen::Vector3d normalVector;
  Color map = colorMap();

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

  const double bondRadius = 0.1;
  const double shift = 0.15;
  int order = b->GetBO();

  // for now, just allow selecting atoms
  //  glPushName(bondType);
  //  glPushName(b->GetIdx());
  map.set(atom1);
  map.applyAsMaterials();
  gl->painter()->drawMultiCylinder( v1, v3, bondRadius, order, shift, BSENGINE_DETAIL_LEVEL );

  map.set(atom2);
  map.applyAsMaterials();
  gl->painter()->drawMultiCylinder( v3, v2, bondRadius, order, shift, BSENGINE_DETAIL_LEVEL );
  //  glPopName();
  //  glPopName();

  return true;
}

bool BSEngine::render(const Molecule *m)
{
  // Disabled
  return false;
}

void BSEngine::options()
{
  QMessageBox::StandardButton ret;
  ret = QMessageBox::information(qobject_cast<QWidget*>(parent()),
                                 tr("Avogadro"),
                                 tr("This will be for render options"),
                                 QMessageBox::Yes | QMessageBox::Cancel);
}

#include "bsengine.moc"

Q_EXPORT_PLUGIN2(bsengine, BSEngineFactory)
