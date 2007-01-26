/**********************************************************************
  GLWidget - general OpenGL display

  Copyright (C) 2006 by Geoffrey R. Hutchison
  Some portions Copyright (C) 2006 by Donald E. Curtis

  This file is part of the Avogadro molecular editor project.  For more
  information, see <http://avogadro.sourceforge.net/>

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

#include <avogadro/engine.h>
#include <avogadro/color.h>

#include <eigen/projective.h>

#include <QGLWidget>
#include <QMouseEvent>
#include <QDir>
#include <QDebug>
#include <QPluginLoader>

#include <vector>

namespace Avogadro {
  
  class GLHit
  {
    public:
      GLHit(GLuint n, GLuint t, GLuint min, GLuint max);
      friend bool operator<(const GLHit &h1, const GLHit &h2);
      friend bool operator==(const GLHit &h1, const GLHit &h2);

      GLuint type;
      GLuint name;
      GLuint minZ;
      GLuint maxZ;
  };

  class GLWidgetPrivate;
  class GLWidget : public QGLWidget
  {
    Q_OBJECT
    Q_PROPERTY(QColor background READ background WRITE setBackground)
    Q_PROPERTY(float scale READ scale WRITE setScale)

    public:
      GLWidget(QWidget *parent = 0);
      GLWidget(const QGLFormat &format, QWidget *parent = 0);
      GLWidget(Molecule *molecule, const QGLFormat &format, QWidget *parent = 0);

      void addDL(GLuint dl);
      void removeDL(GLuint dl);

      QSize sizeHint() const;
      QSize minimumSizeHint() const;

      void setBackground(const QColor &background);
      QColor background() const;

      void rotate(float x, float y, float z);
      void translate(float x, float y, float z);
      void setScale(float s);
      float scale() const;

      void setMolecule(Molecule *molecule);
      const Molecule* molecule() const;

      Engine *defaultEngine() const;
      QList<Engine *> engines() const;

      /**
       * Get the hits for a region starting at (x,y) of size (w x y)
       */
      QList<GLHit> hits(int x, int y, int w, int h) const;

    public Q_SLOTS:
      void setDefaultEngine(int i);
      void setDefaultEngine(Engine *engine);

      void addPrimitive(Primitive *primitive);
      void updatePrimitive(Primitive *primitive);
      void removePrimitive(Primitive *primitive);

      void updateModel();

//dc:       void rowsInsert(const QModelIndex &parent, int start, int end);
//dc:       void rowsRemove(const QModelIndex &parent, int start, int end);

    Q_SIGNALS:
      void mousePress( QMouseEvent * event );
      void mouseRelease( QMouseEvent * event );
      void mouseMove( QMouseEvent * event );

    protected:
      GLWidgetPrivate * const d;
      
      virtual void initializeGL();
      virtual void paintGL();
      virtual void resizeGL(int, int);

      virtual void mousePressEvent( QMouseEvent * event );
      virtual void mouseReleaseEvent( QMouseEvent * event );
      virtual void mouseMoveEvent( QMouseEvent * event );

      virtual void setCamera() const;
      virtual void render(GLenum mode) const;
      
      void loadEngines();
      
    private:
      void constructor();

  };

} // end namespace Avogadro

#endif
