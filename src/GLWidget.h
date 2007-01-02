/**********************************************************************
  GLWidget - general OpenGL display

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

#ifndef __GLWIDGET_H
#define __GLWIDGET_H

#include "Primitives.h"
#include "Views.h"
#include "Engine.h"
#include "Tool.h"
#include "color.h"

#include <QGLWidget>
#include <QMouseEvent>

#include <vector>


namespace Avogadro {

  class GLHit
  {
    public:
      GLHit(GLuint n, GLuint t, GLuint min, GLuint max) { type=t; name = n; minZ = min; maxZ = max; }
      friend bool operator<(const GLHit &h1, const GLHit &h2) { return h1.minZ < h2.minZ; }
      friend bool operator==(const GLHit &h1, const GLHit &h2) { return ((h1.type == h2.type) && (h1.name == h2.name)); }

      GLuint type;
      GLuint name;
      GLuint minZ;
      GLuint maxZ;
  };

  class GLWidget : public QGLWidget
  {
    Q_OBJECT

    friend class Tool;

    public:
      GLWidget(QWidget *parent = 0);
      GLWidget(const QGLFormat &format, QWidget *parent = 0);

      void setView(View *v);
      View* getView() { return view; }

      Engine *getDefaultEngine() { return defaultEngine; }
      QList<Engine *> getEngines() { return engines; }

      void addDL(GLuint dl);
      void removeDL(GLuint dl);

      void setTool(Tool *tool);
      Tool* getTool();

      void setClearColor(const QColor &c) { _clearColor = c; qglClearColor(c);}
      QColor getClearColor() const { return _clearColor;}

      void rotate(float x, float y, float z);
      void translate(float x, float y, float z);
      void setScale(float s);
      float getScale();

      void setMolecule(Molecule *m);
      Molecule* getMolecule() { return molecule; }

      QList<GLHit> getHits(int x, int y, int w, int h);

    public slots:
      void setDefaultEngine(int i);
      void setDefaultEngine(Engine *e);

    protected:
      void init();
      virtual void initializeGL();
      virtual void paintGL();
      virtual void resizeGL(int, int);

      virtual void mousePressEvent( QMouseEvent * event );
      virtual void mouseReleaseEvent( QMouseEvent * event );
      virtual void mouseMoveEvent( QMouseEvent * event );

      void loadTools();
      void loadEngines();

      void setCamera();
      void render(GLenum mode);

      Engine *defaultEngine;
      QList<Engine *> engines;

      Tool *currentTool;
      QList<Tool *> tools;

      PrimitiveQueue defaultQueue;
      QList<PrimitiveQueue> queues;

      Molecule *molecule;
      View *view;
      QList<GLuint> _displayLists;

      GLdouble            _RotationMatrix[16];
      GLdouble            _TranslationVector[3];
      GLdouble            _Scale;
      QColor              _clearColor;

  };

} // end namespace Avogadro

#endif
