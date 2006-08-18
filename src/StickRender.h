#ifndef __AStickRender_H
#define __AStickRender_H

#include <QGLWidget>
#include <openbabel/mol.h>

class AStickRender
{
 public:
 AStickRender() : dlist(0) {}
  ~AStickRender() {}

  GLuint Render(OpenBabel::OBMol &mol);

 private:
  GLuint dlist;

};

#endif
