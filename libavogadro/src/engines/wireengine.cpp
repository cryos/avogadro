/**********************************************************************
  WireEngine - Engine for wireframe display

  Copyright (C) 2006-2007 by Geoffrey R. Hutchison
  Some portions Copyright (C) 2006-2007 by Donald E. Curtis

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
#include "wireengine.h"

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

bool WireEngine::render()
{
  const QList<Primitive *> *list;

  list = queue().primitiveList(Primitive::AtomType);
  for( int i=0; i<list->size(); i++ ) {
    render((Atom *)(*list)[i]);
  }

  list = queue().primitiveList(Primitive::BondType);
  for( int i=0; i<list->size(); i++ ) {
    render((Bond *)(*list)[i]);
  }

  return true;
}

bool WireEngine::render(const Atom *a)
{
   glPushAttrib(GL_ALL_ATTRIB_BITS);

   glPushName(Primitive::AtomType);
   glPushName(a->GetIdx());

  Vector3d v (a->GetVector().AsArray());

  if (a->isSelected()) {
    Color( 0.3, 0.6, 1.0, 0.7 ).applyAsMaterials();
    glPointSize(etab.GetVdwRad(a->GetAtomicNum()) * 4.0);
  }
  else {
    Color(a).applyAsMaterials();
    glPointSize(etab.GetVdwRad(a->GetAtomicNum()) * 3.0);
  }
  
  glBegin(GL_POINTS);
  glVertex3d(v.x(), v.y(), v.z());
  glEnd();

   glPopName();
   glPopName();
   glPopAttrib();

   return true;
}

bool WireEngine::render(const Bond *b)
{
  const OBAtom *atom1 = static_cast<const OBAtom *>( b->GetBeginAtom() );
  const OBAtom *atom2 = static_cast<const OBAtom *>( b->GetEndAtom() );
  Vector3d v1 (atom1->GetVector().AsArray());
  Vector3d v2 (atom2->GetVector().AsArray());
  std::vector<double> rgb;

  glPushAttrib(GL_ALL_ATTRIB_BITS);
  glLineWidth(1.0);
  glBegin(GL_LINES);

  // hard to separate atoms from bonds in this view
  // so we let the user always select atoms
  glPushName( Primitive::AtomType);
  glPushName( atom1->GetIdx() );
  Color(atom1).applyAsMaterials();
  glVertex3d(v1.x(), v1.y(), v1.z());
  glPopName();

  glPushName( atom2->GetIdx() );
  Color(atom2).applyAsMaterials();
  glVertex3d(v2.x(), v2.y(), v2.z());
  glPopName();
  glPopName();

  glEnd();
  glPopAttrib();

  return true;
}

bool WireEngine::render(const Molecule *m)
{
  // Disabled
  return false;
}

#include "wireengine.moc"

Q_EXPORT_PLUGIN2(bsengine, WireEngineFactory)
