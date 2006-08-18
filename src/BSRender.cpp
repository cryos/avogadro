#include "ABSRender.h"
#include <openbabel/obiter.h>

using namespace std;
using namespace OpenBabel;

GLuint ABSRender::Render(OpenBabel::OBMol &mol)
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
      glColor3d(rgb[0], rgb[1], rgb[2]);
      glTranslated(a->GetX(), a->GetY(), a->GetZ());
      gluSphere(q, etab.GetVdwRad(a->GetAtomicNum()) * 0.3, 12, 12);
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

      gluCylinder(q, radius, radius, l1, 6, 1);

      rgb = etab.GetRGB(end->GetAtomicNum());
      glColor3d(rgb[0], rgb[1], rgb[2]);
      glTranslated(0.0, 0.0, l1);
      gluCylinder(q, radius, radius, l1, 6, 1);

      glPopMatrix();
    }
  gluDeleteQuadric(q);
  glPopMatrix();
  glPopAttrib();
  glEndList();

  return dlist;
}

