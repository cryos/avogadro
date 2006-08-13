#ifndef __ASphereRender_H
#define __ASphereRender_H

#include <QGLWidget>
#include <openbabel/mol.h>

class ASphereRender
{
 public:
 ASphereRender() : dlist(0) {}
  ~ASphereRender() {}

  void Render(OpenBabel::OBMol &mol);

 private:
  GLuint dlist;

}

#endif
