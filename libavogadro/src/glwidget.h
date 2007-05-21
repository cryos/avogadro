/**********************************************************************
  GLWidget - general OpenGL display

  Copyright (C) 2006,2007 Geoffrey R. Hutchison
  Copyright (C) 2006,2007 Donald Ephraim Curtis

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.sourceforge.net/>

  Avogadro is free software; you can redistribute it and/or modify 
  it under the terms of the GNU General Public License as published by 
  the Free Software Foundation; either version 2 of the License, or 
  (at your option) any later version.

  Avogadro is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.
 **********************************************************************/

#ifndef __GLWIDGET_H
#define __GLWIDGET_H

#include <avogadro/global.h>
#include <avogadro/engine.h>
#include <avogadro/tool.h>
#include <avogadro/color.h>
#include <avogadro/painter.h>
#include <avogadro/camera.h>

#include <QGLWidget>
#include <QDebug>

#include <vector>

class QUndoStack;
class QMouseEvent;

namespace Avogadro {
  
  class ToolGroup;

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
   * Each engine is allocated a PrimitiveList object.  This queue contains
   * all primitivew which that engine is responsible for rendering for this
   * glwidget.  Thus, we can have one queue containing only the bonds, and 
   * one queue containing only the atoms which would allow bonds and atoms
   * to be rendered by two different engines.
   */
  class GLWidgetPrivate;
  class A_EXPORT GLWidget : public QGLWidget
  {
    Q_OBJECT
    Q_PROPERTY(QColor background READ background WRITE setBackground)
//    Q_PROPERTY(float scale READ scale WRITE setScale)

    public:
      /**
       * Constructor
       * @param parent the widget parent
       */
      GLWidget(QWidget *parent = 0);

      /**
       * Constructor
       * @param format the QGLFormat information
       * @param parent the widget parent
       */
      explicit GLWidget(const QGLFormat &format, QWidget *parent = 0, const QGLWidget * shareWidget = 0);

      /**
       * Constructor
       * @param molecule the molecule to view
       * @param format the QGLFormat information
       * @param parent the widget parent
       */
      GLWidget(Molecule *molecule, const QGLFormat &format, QWidget *parent = 0, const QGLWidget * shareWidget = 0);

      /**
       * Deconstructor
       */
      ~GLWidget();

      /**
       * Add an arbitrary display list to the %GLWidget rendering 
       * area.  This allows plugins to add auxiliary visuals.
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
       * @return true if the GLWidget is stable as determined 
       * by the tools and extensions
       */
      bool isStable() const;

      /**
       * @param stable the new stable value
       */
      void setStable(bool stable);

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

      Tool* tool() const;
      ToolGroup toolManger() const;

      /**
       * @return the current background color of the rendering area
       */
      QColor background() const;

      /**
       * Set the molecule model for this view.
       * @param molecule the molecule to view
       */
      void setMolecule(Molecule *molecule);

      /**
       * @param undoStack the new undoStack
       */
      void setUndoStack(QUndoStack *undoStack);

      /**
       * @return the current GLWidget undoStack
       */
      QUndoStack* undoStack() const;

      /**
       * @return the current molecule being viewed
       */
      const Molecule* molecule() const;

      /**
       * @return the current molecule being viewed
       */
      Molecule* molecule();

      /**
       * update the Molecule Geometry.
       */
      void updateGeometry();

      /**
       * @return a pointer to the camera of this widget
       */
      Camera * camera() const;

      /**
       * @return a list of engines
       */
      QList<Engine *> engines() const;

      /**
       * Get the hits for a region starting at (x,y) of size (w x y)
       */
      QList<GLHit> hits(int x, int y, int w, int h);

      const Eigen::Vector3d & center() const;
      const Eigen::Vector3d & normalVector() const;
      const double & radius() const;
      const Atom *farthestAtom() const;

      void setToolGroup(ToolGroup *toolGroup);
      ToolGroup * toolGroup() const;

      /** Returns the Painter of this widget. For instance, to draw a sphere in this
        * widget, you could do:
        * @code
          painter()->drawSphere(center,radius)
        * @endcode
        * @sa class Painter
        */
      Painter *painter() const;

    public Q_SLOTS:

      void setTool(Tool *tool);

      void setPainter(Painter *painter);
      
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

      /** Toggle the selection for the atoms in the supplied list.
       * That is, if the primitive is selected, deselect it and vice-versa.
       * 
       * @param primitiveList the set of atoms to update
       */
      void toggleSelection(QList<Primitive *> primitiveList);

      /** Change the selection status for the atoms in the supplied list.
       * All atoms in the list will have the same selection status.
       *
       * @param primitiveList the set of atoms to update
       * @param select whether to select or deselect the atoms
       */
      void setSelection(QList<Primitive *> primitiveList, bool select);

      /** Deselect all atoms 
       */
      void clearSelection();

      /** \return the list of selected atoms
       */
      QList<Primitive *> selectedItems();

      /** \return true if the Primitive is selected
       */
      bool selectedItem(const Primitive *p);

      /** Set the number of unit cells for a periodic molecule like a crystal
       * a, b, and c, are the three primitive unit cell axes.
       * Does nothing if the molecule does not have a unit cell defined
       *
       * @param a Number of unit cells to display along the a axis
       * @param b Number of unit cells to display along the b axis
       * @param c Number of unit cells to display along the c axis
       */
      void setUnitCells(int a, int b, int c);

    Q_SIGNALS:
      void mousePress( QMouseEvent * event );
      void mouseRelease( QMouseEvent * event );
      void mouseMove( QMouseEvent * event );
      void wheel( QWheelEvent * event);

    protected:
      GLWidgetPrivate * const d;
      
      /**
       * Virtual function called by QGLWidget on initialization of
       * the GL area.
       */
      virtual void initializeGL();
      /**
       * virtual function we want to bypass if we are not the current context
       */
      virtual void glDraw();
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
       * Virtual function reaction to mouse while in the GL rendering area.
       */
      virtual void wheelEvent( QWheelEvent * event );

      /**
       * Render the scene. To be used in both modes GL_RENDER and GL_SELECT.
       */
      virtual void render();
      
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
