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
#include "GLEngine.h"

#include <QGLWidget>
#include <QMouseEvent>

#include <vector>


namespace Avogadro {

  class GLHit
  {
    public:
      GLHit(GLuint n, GLuint min, GLuint max) { name = n; minZ = min; maxZ = max; }
      friend bool operator<(const GLHit &h1, const GLHit &h2) { return h1.minZ < h2.minZ; }

      GLuint name;
      GLuint minZ;
      GLuint maxZ;
  };

  class GLWidget : public QGLWidget
  {
    Q_OBJECT

    public:
      GLWidget(QWidget *parent = 0);
      GLWidget(const QGLFormat &format, QWidget *parent = 0);

      void setView(View *v);
      View* getView() { return view; }

      GLEngine *getDefaultGLEngine() { return defaultGLEngine; }
      QList<GLEngine *> getGLEngines() { return glEngines; }

      void setClearColor(const QColor &c) { _clearColor = c; qglClearColor(c);}
      QColor getClearColor() const { return _clearColor;}

      void setMolecule(Molecule *m);
      Molecule* getMolecule() { return molecule; }

    public slots:
      void setDefaultGLEngine(int i);
      void setDefaultGLEngine(GLEngine *e);

    protected:
      void init();
      virtual void initializeGL();
      virtual void paintGL();
      virtual void resizeGL(int, int);

      virtual void mousePressEvent( QMouseEvent * event );
      virtual void mouseReleaseEvent( QMouseEvent * event );
      virtual void mouseMoveEvent( QMouseEvent * event );

      void loadGLEngines();

      void setCamera();
      void render(GLenum mode);

      void pick(int x, int y);

      //! Draw a selection box.
      void selectionBox(int sx, int sy, int ex, int ey);

      //! From the GL selection buffer; populate the hits 
      //! with the closest hit as the frontmost.
      void updateHitList(GLint hits, GLuint buffer[]);

      //! List of hits from a selection/pick
      QList<GLHit> _hits;

      //! Temporary var for adding selection box
      GLuint _selectionDL;

      GLEngine *defaultGLEngine;
      QList<GLEngine *> glEngines;

      Molecule *molecule;
      View *view;
      std::vector<GLuint> _displayLists;


      bool                _leftButtonPressed;  // rotation
      bool                _rightButtonPressed; // translation
      bool                _midButtonPressed;   // scale / zoom
      bool                _movedSinceButtonPressed;

      QPoint              _initialDraggingPosition;
      QPoint              _lastDraggingPosition;

      GLdouble            _RotationMatrix[16];
      GLdouble            _TranslationVector[3];
      GLdouble            _Scale;
      QColor              _clearColor;
  };

} // end namespace Avogadro

#endif
