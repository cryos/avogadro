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
#include <openbabel/obiter.h>

#include <QtGui>

using namespace std;
using namespace OpenBabel;
using namespace Avogadro;

void SphereEngine::render(Atom *atom)
{
  m_sphere.setup(2);
  
  glDisable( GL_NORMALIZE );
  glEnable( GL_RESCALE_NORMAL );

  Color(atom).applyAsMaterials();
  m_sphere.draw(atom->GetVector(), etab.GetVdwRad(atom->GetAtomicNum()));
  glEnable( GL_NORMALIZE );
  glDisable( GL_RESCALE_NORMAL );
}

void SphereEngine::render(Bond *)
{
  // nothing to do
}

Q_EXPORT_PLUGIN2(SphereEngine, SphereEngineFactory)
