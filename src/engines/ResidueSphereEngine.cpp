/**********************************************************************
  ResidueSphereEngine - Engine for "balls and sticks" display

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

#include "ResidueSphereEngine.h"
#include "Primitives.h"
#include <openbabel/obiter.h>

#include <QtGui>

using namespace std;
using namespace OpenBabel;
using namespace Avogadro;

bool ResidueSphereEngine::render(Molecule *m)
{
  if (m->NumResidues() == 0)
    return false;

  if (!m_sphere.isValid())
    m_sphere.setup(3);

  glDisable( GL_NORMALIZE );
  glEnable( GL_RESCALE_NORMAL );
  glColor3d(0.7, 0.1, 0.1);
  glPushName(residueTypeName);

  vector3 midpoint;

  vector<OpenBabel::OBResidue*>::iterator i;
  for(OBResidue *r = m->BeginResidue(i); r; r = m->NextResidue(i))
    {
      midpoint.Set(0.0, 0.0, 0.0);
      
      vector<OpenBabel::OBAtom*>::iterator j;
      for(OBAtom *a = r->BeginAtom(j); a; a = r->NextAtom(j))
        midpoint += a->GetVector();
      
      midpoint /= r->GetNumAtoms();
      glPushName(r->GetIdx());
      m_sphere.draw(midpoint, 2.0);
      glPopName();
    }

  glPopName();
  glEnable( GL_NORMALIZE );
  glDisable( GL_RESCALE_NORMAL );

  return true;
}

Q_EXPORT_PLUGIN2(ResidueSphereEngine, ResidueSphereEngineFactory)
