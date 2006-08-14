#include "ASphereRender.h"
#include <openbabel/obiter.h>

using namespace std;
using namespace OpenBabel;

GLuint ASphereRender::Render(OpenBabel::OBMol &mol)
{
  std::vector<double> rgb;
  if (!dlist)
    dlist = glGenLists(1);

  glNewList(dlist, GL_COMPILE);
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
      gluSphere(q, etab.GetVdwRad(a->GetAtomicNum()), 10, 10);
      glTranslated(-a->GetX(), -a->GetY(), -a->GetZ());
    }
  gluDeleteQuadric(q);
  glPopMatrix();
  glEndList();

  return dlist;
}

