/**********************************************************************
SphereRender - Renderer for van der Waals spheres

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

#include "SphereRender.h"
#include <openbabel/obiter.h>

using namespace std;
using namespace OpenBabel;
using namespace Avogadro;

GLuint SphereRender::Render(OpenBabel::OBMol &mol)
{
  std::vector<double> rgb;
  if (!dlist)
    dlist = glGenLists(1);

  glNewList(dlist, GL_COMPILE);
  glPushAttrib(GL_ALL_ATTRIB_BITS);
  glPushMatrix();
  glTranslated(0.0, 0.0, 0.0);
  GLUquadricObj *q = gluNewQuadric();
  gluQuadricDrawStyle(q, GLU_FILL );
  gluQuadricNormals(q, GLU_SMOOTH );
  FOR_ATOMS_OF_MOL(a, mol)
    {
      rgb = etab.GetRGB(a->GetAtomicNum());
      glColor3f(rgb[0], rgb[1], rgb[2]);
      glTranslated(a->GetX(), a->GetY(), a->GetZ());
      gluSphere(q, etab.GetVdwRad(a->GetAtomicNum()), 15, 15);
      glTranslated(-a->GetX(), -a->GetY(), -a->GetZ());
    }
  gluDeleteQuadric(q);
  glPopMatrix();
  glPopAttrib();
  glEndList();

  return dlist;
}

