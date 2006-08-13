#include "ASphereRender.h"
#include <openbabel/obiter.h>

using namespace OpenBabel;

void ASphereRender::Render(OpenBabel::OBMol &mol)
{
  std::vector<double> rgb;
  if (!dlist)
    dlist = glGenLists(1);

  glNewList(dlist, GL_COMPILE);
  glPushMatrix();
  glTranslate(0.0, 0.0, 0.0);
  GLUquadraticObj q = gluNewQuadric();
  gluQuadricDrawStyle(q, GLU_FILL );
  gluQuadricNormals(q, GLU_SMOOTH );
  FOR_ATOMS_OF_MOL(a, mol)
    {
      rgb = etab.GetRGB(a->GetAtomicNum());
      glColor3f(rgb[0], rgb[1], rgb[2]);
      glTranslate(a->GetX(), a->GetY(), a->GetZ());
      gluSphere(q, 0.03, 10, 10);
      glTranslate(-a->GetX(), -a->GetY(), -a->GetZ());
    }
  gluDeleteQuadric(q);
  glPopMatrix();
  glEndList();

  return dlist;
}

