#ifndef __ABSRender_H
#define __ABSRender_H

#include <QGLWidget>
#include <openbabel/mol.h>

class ABSRender
{
 public:
 ABSRender() : dlist(0) {}
  ~ABSRender() {}

  GLuint Render(OpenBabel::OBMol &mol);

 private:
  GLuint dlist;

};

#endif
