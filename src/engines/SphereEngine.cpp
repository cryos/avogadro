/**********************************************************************
  SphereEngine - Engine for van der Waals sphere display

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

#include "SphereEngine.h"
#include "Primitives.h"
#include "color.h"

#include <openbabel/obiter.h>

#include <QtGui>

using namespace std;
using namespace OpenBabel;
using namespace Avogadro;

bool SphereEngine::render(Atom *a)
{
  if (!m_sphere.isValid())
    m_sphere.setup(4);
  
  glDisable( GL_NORMALIZE );
  glEnable( GL_RESCALE_NORMAL );

  Color(a).applyAsMaterials();

  glPushName(atomType);
  glPushName(a->GetIdx());
  m_sphere.draw(a->GetVector(), etab.GetVdwRad(a->GetAtomicNum()));

  if (a->isSelected())
    {
      Color( 0.3, 0.6, 1.0, 0.7 ).applyAsMaterials();
      glEnable( GL_BLEND );
      m_sphere.draw(a->GetVector(), 0.18 + etab.GetVdwRad(a->GetAtomicNum()));
      glDisable( GL_BLEND );
    }

  glPopName();
  glPopName();

  glEnable( GL_NORMALIZE );
  glDisable( GL_RESCALE_NORMAL );

  return true;
}

Q_EXPORT_PLUGIN2(SphereEngine, SphereEngineFactory)
