/**********************************************************************
  GLWidget - general OpenGL display

  Copyright (C) 2006 by Geoffrey R. Hutchison
  Copyright (C) 2006,2007 by Donald Ephraim Curtis

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

#include <avogadro/global.h>
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
  
  /**
   * @class GLHit
   * @brief Class for wrapping hits from GL picking.
   * @author Donald Ephraim Curtis
   *
   * Provides an easy to use class to contain OpenGL hits returned from the 
   * process of picking.  This class relies on the %Engine subclasses properly 
   * naming the objects which it is rendering.  For more information see the 
   * Engine documentation.
   */
  class GLHitPrivate;
  class A_EXPORT GLHit
  {
    public:
      /** 
       * Blank constructor.
       */
      GLHit();
      /**
       * Copy constructor.
       */
      GLHit(const GLHit &glHit);
      /** 
       * Constructor.
       * @param type The type of the OpenGL object that was picked which corresponds
       * to the Primitive::Type for the object
       * (ie. type==Primitive::AtomType means an Atom was picked).
       * @param name The name of the OpenGL object that was picked corresponding
       * to the primitive index
       * (ie. name==1 means Atom 1)
       */
      GLHit(GLuint type, GLuint name, GLuint minZ, GLuint maxZ);
      /**
       * Deconstructor.
       */
      ~GLHit();

      /**
       * Less than operator.
       * @param other the other GLHit object to compare to
       * @return (this->minZ < other->minZ) ? @c true : @c false
       */
      bool operator<(const GLHit &other) const;

      /**
       * Equivalence operator.
       * @param other the other GLHit object to test equivalence with
       * @return returns true if all elements are equivalent (type, name, minZ, maxZ)
       */
      bool operator==(const GLHit &other) const;

      /**
       * Copy operator.
       * @param other the GLHit object to set this object equal to
       * @return  *this
       */
      GLHit &operator=(const GLHit &other);

      /**
       * @return type of the object which was picked
       */
      GLuint type() const;

      /**
       * @return name of the object that was picked
       */
      GLuint name() const;

      /** 
       * @return the minimum Z value of this hit corresponding 
       * to the Z value of the drawn object closest to the camera
       */
      GLuint minZ() const;

      /**
       * @return the maximum Z value of this hit corresponding
       * to the Z value of the drawn object farthest from the camera
       */
      GLuint maxZ() const;

      /**
       * @param type new object type
       */
      void setType(GLuint type);
      /**
       * @param name new object name
       */
      void setName(GLuint name);
      /**
       * @param minZ minimum Z value to set for this object
       */
      void setMinZ(GLuint minZ);
      /**
       * @param maxZ maximum Z value to set for this object
       */
      void setMaxZ(GLuint maxZ);

    private:
      GLHitPrivate * const d;
  };

  /**
   * @class GLWidget
   * GL widget class for rendering molecules.
   * @author Donald Ephraim Curtis
   *
   * This widget provides a 3d graphical view of a molecule.   In terms 
   * of a Model-View architecture we consider
   * the Molecule the model and GLWidget a view of this model.
   * The widget relies on a various Engine subclasses to handle
   * rendering of the 3d objects.  
   *
   * Each engine is allocated a PrimitiveQueue object.  This queue contains
   * all primitivew which that engine is responsible for rendering for this
   * glwidget.  Thus, we can have one queue containing only the bonds, and 
   * one queue containing only the atoms which would allow bonds and atoms
   * to be rendered by two different engines.
   *
   * @todo These PrimitiveQueue's could be moved into the engines themselves
   * meaning the engines would track what they are to render.  This is dependant
   * on how we want our application to behave.  In the case that we allow multiple
   * views of the same molecule, we have the option of using the EngineFactory to
   * generate a while new set of engines for the new %GLWidget.  This means that
   * each %GLWidget would have it's own set of engines (different instances) and
   * would allow each %GLWidget control settings for their engines differently. 
   * For example, one view could set the Ball and Stick radius to 5 and one could
   * use the same engine but set the radius to 10.  In other words, different
   * settings for the same engine within a single project.
   */
  class GLWidgetPrivate;
  class A_EXPORT GLWidget : public QGLWidget
  {
    Q_OBJECT
    Q_PROPERTY(QColor background READ background WRITE setBackground)
    Q_PROPERTY(float scale READ scale WRITE setScale)

    public:
      /**
       * Constructor.
       * @param parent the widget parent
       */
      GLWidget(QWidget *parent = 0);

      /**
       * Constructor.
       * @param format the QGLFormat information
       * @param parent the widget parent
       */
      GLWidget(const QGLFormat &format, QWidget *parent = 0);

      /**
       * Constructor.
       * @param molecule the molecule to view
       * @param format the QGLFormat information
       * @param parent the widget parent
       */
      GLWidget(Molecule *molecule, const QGLFormat &format, QWidget *parent = 0);

      /**
       * Add an arbitrary display list to the %GLWidget rendering 
       * area.  This allows plugins to add auxilary visuals.
       *
       * @param dl the unsigned int representing the GL display list
       */
      void addDL(GLuint dl);
      /**
       * Remove a display list from the %GLWidget rendering
       * area previously added by addDL.  If the display list does
       * not exist nothing is removed.
       *
       * @param dl the display lists to remove
       */
      void removeDL(GLuint dl);

      /**
       * Virtual function setting the size hint for this widget.
       *
       * @return the preferred size of the widget
       */
      QSize sizeHint() const;

      /**
       * Virtual function setting the minimum size hit for this widget.
       *
       * @return the minimum size the widget can take without causing 
       * unspecified behaviour
       */
      QSize minimumSizeHint() const;

      /**
       * Set the background color of the rendering area.  (default black)
       *
       * @param background the new background color
       */
      void setBackground(const QColor &background);
      /**
       * @return the current background color of the rendering area
       */
      QColor background() const;

      /**
       * Rotate the object.
       *
       * @param x ammount to rotate around the X axis
       * @param y ammount to rotate around the Y axis
       * @param z ammount to rotate around the Z axis
       */
      void rotate(float x, float y, float z);

      /**
       * Translate the object.
       *
       * @param x ammount to translate in the X direction
       * @param y ammount to translate in the Y direction
       * @param z ammount to translate in the Z direction
       */
      void translate(float x, float y, float z);

      /**
       * Set the scale factor.
       *
       * @param s the new scale factor
       */
      void setScale(float s);
      /**
       * @return the current scale factor
       */
      float scale() const;

      /**
       * Set the molecule model for this view.
       * @param molecule the molecule to view
       */
      void setMolecule(Molecule *molecule);
      /**
       * @return the current molecule being viewed
       */
      const Molecule* molecule() const;

      /**
       * Set the default engine.
       * @param engine pointer to the new default engine
       */
      void setDefaultEngine(Engine *engine);
      /**
       * @return the default engine which new primitives 
       * will be added to
       */
      Engine *defaultEngine() const;

      /**
       * @return a QList of all the engines available to this %GLWidget
       */
      QList<Engine *> engines() const;

      /**
       * Get the hits for a region starting at (x,y) of size (w x y)
       */
      QList<GLHit> hits(int x, int y, int w, int h) const;

    public Q_SLOTS:
      /**
       * Set the default engine based on index.  If no engine
       * exists for this index, nothing is changed.
       * @param i the index in the engines list
       */
      void setDefaultEngine(int i);

      /**
       * Add the primitive to the widget.  This slot is called whenever
       * a new primitive is added to our molecule model.  It adds the 
       * primitive to the list in the appropriate group.
       *
       * @param primitive pointer to a primitive to add to the view
       */
      void addPrimitive(Primitive *primitive);
      /**
       * Update a primitive.  This slot is called whenever a primitive of our 
       * molecule model has been changed and we need to check our view.
       *
       * @note In some cases we are passed the molecule itself meaning that more
       * than one thing has changed in the molecule.
       *
       * @param primitive primitive that was changed
       */
      void updatePrimitive(Primitive *primitive);
      /** Remove a primitive.  This slot is called whenever a primitive of our
       * molecule model has been removed and we need to take it off our list.  
       * Additionally we need to update other items in our view that are impacted
       * by this change.
       *
       * @param primitive primitive that was removed
       */
      void removePrimitive(Primitive *primitive);


    Q_SIGNALS:
      void mousePress( QMouseEvent * event );
      void mouseRelease( QMouseEvent * event );
      void mouseMove( QMouseEvent * event );

    protected:
      GLWidgetPrivate * const d;
      
      /**
       * Virtual function called by QGLWidget on initialization of
       * the GL area.
       */
      virtual void initializeGL();
      /**
       * Virtual function called when the GL area needs repainting
       */
      virtual void paintGL();
      /**
       * Virtual functionc called whn the GL area is resized
       */
      virtual void resizeGL(int, int);

      /**
       * Virtual function reaction to mouse press in the GL rendering area.
       */
      virtual void mousePressEvent( QMouseEvent * event );
      /**
       * Virtual function reaction to mouse release in the GL rendering area.
       */
      virtual void mouseReleaseEvent( QMouseEvent * event );
      /**
       * Virtual function reaction to mouse being moved in the GL rendering area.
       */
      virtual void mouseMoveEvent( QMouseEvent * event );

      /** 
       * Helper function to setup the camera.
       */
      virtual void setCamera() const;
      /** 
       * Helper function for rendering the scene.
       */
      virtual void render(GLenum mode) const;
      
      /**
       * Helper function to load all available engines
       */
      void loadEngines();
      
    private:
      /**
       * Helper function called by all constructors
       */
      void constructor();

  };

} // end namespace Avogadro

#endif
