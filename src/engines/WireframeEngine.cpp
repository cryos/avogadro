/**********************************************************************
  WireframeEngine - Engine for wireframe display

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

#include "WireframeEngine.h"
#include "Primitives.h"
#include <openbabel/obiter.h>

#include <QtGui>

using namespace std;
using namespace OpenBabel;
using namespace Avogadro;

void WireframeEngine::initAtomDL()
{

  glLineWidth(1.0);
}

void WireframeEngine::render(Atom *atom)
{
  std::vector<double> rgb;
  rgb = etab.GetRGB(atom->GetAtomicNum());
  glPushAttrib(GL_ALL_ATTRIB_BITS);
  glBegin(GL_POINTS);
  glColor3d(rgb[0], rgb[1], rgb[2]);
  glPointSize(etab.GetVdwRad(atom->GetAtomicNum()));
  glVertex3d(atom->GetX(), atom->GetY(), atom->GetZ());
  glEnd();
  glPopAttrib();
}

void WireframeEngine::render(Bond *b)
{
  std::vector<double> rgb;
  glPushAttrib(GL_ALL_ATTRIB_BITS);
  glBegin(GL_LINES);
  rgb = etab.GetRGB(b->GetBeginAtom()->GetAtomicNum());
  glColor3d(rgb[0], rgb[1], rgb[2]);
  glVertex3d(b->GetBeginAtom()->GetX(), b->GetBeginAtom()->GetY(), b->GetBeginAtom()->GetZ());
  glColor3d(rgb[0], rgb[1], rgb[2]);
  rgb = etab.GetRGB(b->GetEndAtom()->GetAtomicNum());
  glVertex3d(b->GetEndAtom()->GetX(), b->GetEndAtom()->GetY(), b->GetEndAtom()->GetZ());
  glEnd();
  glPopAttrib();
}

Q_EXPORT_PLUGIN2(WireframeEngine, WireframeEngineFactory)
