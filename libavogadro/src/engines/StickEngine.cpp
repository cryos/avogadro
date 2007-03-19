/**********************************************************************
  StickEngine - Engine for "sticks" display

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

#include "StickEngine.h"
#include "Primitives.h"
#include "color.h"

#include <openbabel/obiter.h>

#include <QtGui>

using namespace std;
using namespace OpenBabel;
using namespace Avogadro;

bool StickEngine::render(PrimitiveQueue *q, const MolGeomInfo &molGeomInfo)
{
  QList<Primitive *> *queue;

  queue = q->getTypeQueue(atomType);
  for( int i=0; i<queue->size(); i++ ) {
    render((Atom *)(*queue)[i]);
  }

  queue = q->getTypeQueue(bondType);
  for( int i=0; i<queue->size(); i++ ) {
    render((Bond *)(*queue)[i], molGeomInfo);
  }

  return true;
}


bool StickEngine::render(Atom *a)
{
  if (!m_sphere.isValid())
    m_sphere.setup(2);

  glDisable( GL_NORMALIZE );
  glEnable( GL_RESCALE_NORMAL );

  Color(a).applyAsMaterials();

  glPushName(atomType);
  glPushName(a->GetIdx());
  m_sphere.draw(a->GetVector(), 0.3);

  if (a->isSelected())
    {
      Color( 0.3, 0.6, 1.0, 0.7 ).applyAsMaterials();
      glEnable( GL_BLEND );
      m_sphere.draw(a->GetVector(), 0.18 + etab.GetVdwRad(a->GetAtomicNum()) * 0.3);
      glDisable( GL_BLEND );
    }

  glPopName();
  glPopName();
  glEnable( GL_NORMALIZE );
  glDisable( GL_RESCALE_NORMAL );

  return true;
}

bool StickEngine::render(Bond *b, const MolGeomInfo &molGeomInfo)
{
  // cout << "Render Bond..." << endl;
  if (!m_cylinder.isValid())
    m_cylinder.setup(12);

  glDisable( GL_NORMALIZE );
  glEnable( GL_RESCALE_NORMAL );

  OBAtom *atom1 = static_cast<OBAtom *>( b->GetBeginAtom() );
  OBAtom *atom2 = static_cast<OBAtom *>( b->GetEndAtom() );
  
  vector3 v1 = atom1->GetVector();
  vector3 v2 = atom2->GetVector();
  vector3 v3 = ( v1 + v2 ) / 2;
 
  double radius = 0.3;
  int order = 1;

  // hard to separate atoms from bonds in this view
  // so we let the user always select atoms
  glPushName( bondType);
  glPushName( b->GetIdx() );
  Color(atom1).applyAsMaterials();
  m_cylinder.draw( v1, v3, radius, order, 0.0, molGeomInfo.normalVector());

  Color(atom2).applyAsMaterials();
  m_cylinder.draw( v2, v3, radius, order, 0.0, molGeomInfo.normalVector());

  if (b->isSelected())
    {
      Color( 0.3, 0.6, 1.0, 0.7 ).applyAsMaterials();
      glEnable( GL_BLEND );
      m_cylinder.draw( v1, v2, radius + 0.18, order, 0.0, molGeomInfo.normalVector());
      glDisable( GL_BLEND );
    }

  glPopName();
  glPopName();

  glEnable( GL_NORMALIZE );
  glDisable( GL_RESCALE_NORMAL );

  return true;
}

Q_EXPORT_PLUGIN2(StickEngine, StickEngineFactory)
