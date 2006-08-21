/**********************************************************************
  SBSRenderer - Renderer for "balls and sticks" display

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

#include "SBSRenderer.h"
#include "Primatives.h"
#include <openbabel/obiter.h>

#include <QtGui>

using namespace std;
using namespace OpenBabel;
using namespace Avogadro;

void SBSRenderer::initAtomDL()
{
  // initialize the atom DL
  atomDL = glGenLists(1);
  glNewList(atomDL, GL_COMPILE);
  glPushAttrib(GL_ALL_ATTRIB_BITS);
  glPushMatrix();
  GLUquadricObj *q = gluNewQuadric();
  gluQuadricDrawStyle(q, GLU_FILL );
  gluQuadricNormals(q, GLU_SMOOTH );
  gluSphere(q, 1, 18, 18);
  glPopMatrix();
  glPopAttrib();
  glEndList();
}

void SBSRenderer::renderAtom(Atom &atom)
{
  if( atomDL == 0 )
    initAtomDL();

  std::vector<double> rgb;
  glPushAttrib(GL_ALL_ATTRIB_BITS);
  glPushMatrix();
  glTranslated(atom.GetX(), atom.GetY(), atom.GetZ());
  rgb = etab.GetRGB(atom.GetAtomicNum());
  glScalef(.5, .5, .5);
  glColor3d(rgb[0], rgb[1], rgb[2]);
  glCallList(atomDL);
  glPopMatrix();
  glPopAttrib();

  /*
  glPushAttrib(GL_ALL_ATTRIB_BITS);
  glPushMatrix();
  GLUquadricObj *q = gluNewQuadric();
  gluQuadricDrawStyle(q, GLU_FILL );
  gluQuadricNormals(q, GLU_SMOOTH );
  glTranslated(atom.GetX(), atom.GetY(), atom.GetZ());
  rgb = etab.GetRGB(atom.GetAtomicNum());
  glColor3d(rgb[0], rgb[1], rgb[2]);
  gluSphere(q, 1, 18, 18);
  glPopMatrix();
  glPopAttrib();
  */
}

void SBSRenderer::renderBond(Bond &bond)
{
  cout << "Render Bond" << endl;
}

/*
GLuint SBSRenderer::Render(OBMol &mol)
{
  std::vector<double> rgb;
  if (!dlist)
    dlist = glGenLists(1);

  glNewList(dlist, GL_COMPILE);
  glPushAttrib(GL_ALL_ATTRIB_BITS);
  glPushMatrix();
  //  glTranslated(0.0, 0.0, 0.0);
  GLUquadricObj *q = gluNewQuadric();
  gluQuadricDrawStyle(q, GLU_FILL );
  gluQuadricNormals(q, GLU_SMOOTH );
  FOR_ATOMS_OF_MOL(a, mol)
  {
    rgb = etab.GetRGB(a->GetAtomicNum());
    glColor3d(rgb[0], rgb[1], rgb[2]);
    glTranslated(a->GetX(), a->GetY(), a->GetZ());
    gluSphere(q, etab.GetVdwRad(a->GetAtomicNum()) * 0.3, 9, 9);
    glTranslated(-a->GetX(), -a->GetY(), -a->GetZ());
  }
  OBAtom *bgn, *end;
  vector3 v;
  double arot, xrot, yrot;
  double w;
  double l1, l2;
  double trans, radius = 0.1;
  // Cribbed from Gnome Chemistry Utils (gtkchem3dviewer.c)
  FOR_BONDS_OF_MOL(b, mol)
  {
    glPushMatrix();
    bgn = b->GetBeginAtom();
    end = b->GetEndAtom();
    glTranslated(bgn->GetX(), bgn->GetY(), bgn->GetZ());
    v = end->GetVector() - bgn->GetVector();
    w = sqrt(v.x()*v.x() + v.y()*v.y());
    if (w > 0.0)
    {
      xrot = -v.y() / w;
      yrot = v.x() / w;
      arot = atan2(w, v.z()) * RAD_TO_DEG;
    }
    else
    {
      xrot = 0.0;
      if (v.z() > 0.0) yrot = arot = 0.0;
      else
      {
        yrot = 1.0;
        arot = 180.0;
      }
    }

    glRotated(arot, xrot, yrot, 0.0);
    l1 = b->GetLength() * etab.GetVdwRad(bgn->GetAtomicNum()) /
      (etab.GetVdwRad(bgn->GetAtomicNum()) + etab.GetVdwRad(end->GetAtomicNum()));
    l2 = b->GetLength() - l1;

    rgb = etab.GetRGB(bgn->GetAtomicNum());
    glColor3d(rgb[0], rgb[1], rgb[2]);

    gluCylinder(q, radius, radius, l1, 5, 1);

    rgb = etab.GetRGB(end->GetAtomicNum());
    glColor3d(rgb[0], rgb[1], rgb[2]);
    glTranslated(0.0, 0.0, l1);
    gluCylinder(q, radius, radius, l1, 5, 1);

    glPopMatrix();
  }
  gluDeleteQuadric(q);
  glPopMatrix();
  glPopAttrib();
  glEndList();

  return dlist;
}
*/

Q_EXPORT_PLUGIN2(SBSRendererFactory, SBSRendererFactory)
