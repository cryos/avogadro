/**********************************************************************
  BSEngine - Engine for "balls and sticks" display

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

#include "BSEngine.h"
#include "Primitives.h"
#include "color.h"

#include <openbabel/obiter.h>

#include <QtGui>

using namespace std;
using namespace OpenBabel;
using namespace Avogadro;

bool BSEngine::render(PrimitiveQueue *q)
{
  QList<Primitive *> *queue;

  queue = q->getTypeQueue(atomType);
  for( int i=0; i<queue->size(); i++ ) {
    render((Atom *)(*queue)[i]);
  }

  queue = q->getTypeQueue(bondType);
  for( int i=0; i<queue->size(); i++ ) {
    render((Bond *)(*queue)[i]);
  }

  return true;
}

bool BSEngine::render(Atom *a)
{
  if (!m_sphere.isValid())
    m_sphere.setup(3);

  glDisable( GL_NORMALIZE );
  glEnable( GL_RESCALE_NORMAL );

  glPushName(atomType);
  glPushName(a->GetIdx());
  Color(a).applyAsMaterials();

  m_sphere.draw(a->GetVector(), etab.GetVdwRad(a->GetAtomicNum()) * 0.3);

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

bool BSEngine::render(Bond *b)
{
  // cout << "Render Bond..." << endl;
  if (!m_cylinder.isValid())
    m_cylinder.setup(6);

  glDisable( GL_NORMALIZE );
  glEnable( GL_RESCALE_NORMAL );

  OBAtom *atom1 = static_cast<OBAtom *>( b->GetBeginAtom() );
  OBAtom *atom2 = static_cast<OBAtom *>( b->GetEndAtom() );
  
  vector3 v1 = atom1->GetVector();
  vector3 v2 = atom2->GetVector();
  vector3 v3 = ( v1 + v2 ) / 2;
  std::vector<double> rgb;

  double radius = 0.1;
  double shift = 0.15;
  int order = b->GetBO();

  // for now, just allow selecting atoms
  //  glPushName(bondType);
  //  glPushName(b->GetIdx());
  Color(atom1).applyAsMaterials();
  m_cylinder.draw( v1, v3, radius, order, shift);

  Color(atom2).applyAsMaterials();
  m_cylinder.draw( v2, v3, radius, order, shift);
  //  glPopName();
  //  glPopName();

  glEnable( GL_NORMALIZE );
  glDisable( GL_RESCALE_NORMAL );

  return true;
}

bool BSEngine::render(Molecule *m)
{
  // Disabled
  return false;

  if (!m_sphere.isValid())
    m_sphere.setup(6);

  glDisable( GL_NORMALIZE );
  glEnable( GL_RESCALE_NORMAL );

  vector<OpenBabel::OBNodeBase*>::iterator i;
  for(Atom *a = (Atom *)m->BeginAtom(i); a; a = (Atom *)m->NextAtom(i))
  {
    glPushName(atomType);
    glPushName(a->GetIdx());
    //  Color(a).applyAsMaterials();
    std::vector<double> rgb;
    rgb = etab.GetRGB(a->GetAtomicNum());
    glColor3d(rgb[0], rgb[1], rgb[2]);

    m_sphere.draw(a->GetVector(), etab.GetVdwRad(a->GetAtomicNum()) * 0.3);

    if (a->isSelected())
      {
        glColor4d( 0.3, 0.6, 1.0, 0.7 );
        glEnable( GL_BLEND );
        m_sphere.draw(a->GetVector(), 0.18 + etab.GetVdwRad(a->GetAtomicNum()) * 0.3);
        glDisable( GL_BLEND );
      }

    glPopName();
    glPopName();
  }

  if (!m_cylinder.isValid())
    m_cylinder.setup(6);

  vector<OpenBabel::OBEdgeBase*>::iterator j;
  for(OBBond *b = m->BeginBond(j); b; b = m->NextBond(j))
  {
    OBAtom *atom1 = static_cast<OBAtom *>( b->GetBeginAtom() );
    OBAtom *atom2 = static_cast<OBAtom *>( b->GetEndAtom() );
    
    vector3 v1 = atom1->GetVector();
    vector3 v2 = atom2->GetVector();
    vector3 v3 = ( v1 + v2 ) / 2;
    std::vector<double> rgb;

    double radius = 0.1;
    double shift = 0.15;
    int order = b->GetBO();

    // for now, just allow selecting atoms
    //  glPushName(bondType);
    //  glPushName(b->GetIdx());
    rgb = etab.GetRGB(atom1->GetAtomicNum());
    glColor3d(rgb[0], rgb[1], rgb[2]);
    m_cylinder.draw( v1, v3, radius, order, shift);

    rgb = etab.GetRGB(atom2->GetAtomicNum());
    glColor3d(rgb[0], rgb[1], rgb[2]);
    m_cylinder.draw( v2, v3, radius, order, shift);
    //  glPopName();
    //  glPopName();
  }

  glEnable( GL_NORMALIZE );
  glDisable( GL_RESCALE_NORMAL );

  return true;
}

Q_EXPORT_PLUGIN2(BSEngine, BSEngineFactory)
