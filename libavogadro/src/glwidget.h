/**********************************************************************
  GLWidget - general OpenGL display

  Copyright (C) 2006-2009 Geoffrey R. Hutchison
  Copyright (C) 2006,2007 Donald Ephraim Curtis
  Copyright (C) 2007-2009 Marcus D. Hanwell

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.openmolecules.net/>

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

#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <avogadro/global.h>

#ifdef ENABLE_GLSL
  #include <GL/glew.h>
#endif
#include <QGLWidget>

#include <avogadro/glhit.h>

#include <QThread>

#include <Eigen/Core>

class QUndoStack;
class QMouseEvent;
class QGLContext;
class QSettings;

namespace Avogadro {

  class Primitive;
  class Atom;
  class Bond;
  class Molecule;
  class Camera;
  class Painter;
  class Tool;
  class ToolGroup;
  class Color;
  class Engine;
  class Painter;
  class PrimitiveList;

  /**
   * @class GLWidget glwidget.h <avogadro/glwidget.h>
   * @brief GL widget class for rendering molecules.
   * @author Donald Ephraim Curtis
   * @author Marcus D. Hanwell
   *
   * This widget provides a 3D graphical view of a molecule.   In terms
   * of the Model-View architecture we consider
   * the Molecule the model and GLWidget a view of this model.
   * The widget relies on various Engine subclasses to handle the
   * rendering of the 3d objects.
   *
   * Each engine is allocated a PrimitiveList object.  This queue contains
   * all primitivew which that engine is responsible for rendering for this
   * GLWidget.  Thus, we can have one queue containing only the bonds, and
   * one queue containing only the atoms which would allow bonds and atoms
   * to be rendered by two different engines.
   */
  class GLThread;
  class GLWidgetPrivate;
  class GLPainterDevice;
  class A_EXPORT GLWidget : public QGLWidget
  {
    friend class GLThread;

    Q_OBJECT
    Q_PROPERTY(QColor background READ background WRITE setBackground)
//    Q_PROPERTY(float scale READ scale WRITE setScale)

    public:
      /**
       * Constructor.
       * @param parent the widget parent.
       */
      GLWidget(QWidget *parent = 0);

      /**
       * Constructor.
       * @param format the QGLFormat information.
       * @param parent the widget parent.
       * @param shareWidget a widget to share the same graphics -- i.e., the underlying GLPainterDevice
       */
      explicit GLWidget(const QGLFormat &format, QWidget *parent = 0, const GLWidget * shareWidget = 0);

      /**
       * Constructor.
       * @param molecule the molecule to view.
       * @param format the QGLFormat information.
       * @param parent the widget parent.
       * @param shareWidget a widget to share the same graphics -- i.e., the underlying GLPainterDevice
       */
      GLWidget(Molecule *molecule, const QGLFormat &format, QWidget *parent = 0, const GLWidget * shareWidget = 0);

      /**
       * Destructor.
       */
      ~GLWidget();

      /**
       * @param enabled True if quick render is desired when moving the view.
       */
      void setQuickRender(bool enabled);

      /**
       * @return True if quick rendering is being used.
       */
      bool quickRender() const;

      /**
      * @param enabled True if we should render the unit cell axes
      */
      void setRenderUnitCellAxes(bool enabled);

      /**
       * @return True if unit cell axes are being rendered.
       */
      bool renderUnitCellAxes() const;

      /**
       * @return the width of the widget in pixels.
       */
      int deviceWidth() { return width(); }

      /**
       * @return the height of the widget in pixels.
       */
      int deviceHeight() { return height(); }

      /**
       * Virtual function setting the size hint for this widget.
       *
       * @return the preferred size of the widget.
       */
      QSize sizeHint() const;

      /**
       * Virtual function setting the minimum size hit for this widget.
       *
       * @return the minimum size the widget can take without causing
       * unspecified behaviour.
       */
      QSize minimumSizeHint() const;

      /**
       * @return the radius of the primitive object for this glwidget.
       */
      double radius(const Primitive *p) const;

      /**
       * @return the active Tool.
       */
      Tool* tool() const;

      /**
       * @return the current background color of the rendering area.
       */
      QColor background() const;

      /**
       * Set the current global color map for Primitives.
       */
      void setColorMap(Color *);

      /**
       * @return the current global color map for Primitives.
       */
      Color *colorMap() const;

      /**
       * @param undoStack the new undoStack.
       */
      void setUndoStack(QUndoStack *undoStack);

      /**
       * @return the current GLWidget undoStack.
       */
      QUndoStack* undoStack() const;

      /**
       * @return the current Molecule being viewed.
       */
      const Molecule* molecule() const;

      /**
       * @return the current Molecule being viewed.
       */
      Molecule* molecule();

      /**
       * update the Molecule geometry.
       */
      void updateGeometry();

      /**
       * @return a pointer to the Camera of this widget.
       */
      Camera * camera() const;

      /**
       * @return a list of engines.
       */
      QList<Engine *> engines() const;

      /**
       * Get the hits for a region starting at (x, y) of size (w * h).
       */
      QList<GLHit> hits(int x, int y, int w, int h);

      /**
       * Take a point and figure out which is the closest Primitive under that point.
       * @param p the point on the widget that was clicked.
       * @return the closest Primitive that was clicked or 0 if nothing.
       */
      Primitive* computeClickedPrimitive(const QPoint& p);

      /**
       * Take a point and figure out which is the closest Atom under that point.
       * @param p the point on the widget that was clicked.
       * @return the closest Atom that was clicked or 0 if nothing.
       */
      Atom* computeClickedAtom(const QPoint& p);

      /**
       * Take a point and figure out which is the closest Bond under that point.
       * @param p the point on the widget that was clicked.
       * @return the closest Bond that was clicked or 0 if nothing.
       */
      Bond* computeClickedBond(const QPoint& p);

      /**
       * @return the point at the center of the Molecule.
       */
      const Eigen::Vector3d & center() const;
      /**
       * @return the normalVector for the entire Molecule.
       */
      const Eigen::Vector3d & normalVector() const;
      /**
       * @return the radius of the Molecule.
       */
      double radius() const;
      /**
       * @return the Atom farthest away from the camera.
       */
      const Atom *farthestAtom() const;

      /**
       * @param quality set the global quality of the widget.
       */
      void setQuality(int quality);

      /**
       * @return the global quality of the widget.
       */
      int quality() const;

      /**
       * @param level Set the global fog level of the widget.
       */
      void setFogLevel(int level);

      /**
       * @return The global fog level of the widget.
       */
      int fogLevel() const;

      /**
       * Set to render x, y, z axes as an overlay in the bottom left of the widget.
       */
      void setRenderAxes(bool renderAxes);

      /**
       * @return true if the x, y, z axes are being rendered.
       */
      bool renderAxes();

      /**
       * Set to render the "debug info" (i.e., FPS, number of atoms, etc.)
       */
      void setRenderDebug(bool renderDebug);

      /**
       * @return true if the debug panel is being drawn
       */
      bool renderDebug();

      /**
       * Set the ToolGroup of the GLWidget.
       */
      void setToolGroup(ToolGroup *toolGroup);

      /**
       * @return the ToolGroup of the GLWidget.
       */
      ToolGroup * toolGroup() const;

      /** Returns the Painter of this widget. For instance, to draw a sphere in this
        * widget, you could do:
        * @code
          painter()->drawSphere(center, radius)
        * @endcode
        * @sa class Painter
        */
      Painter *painter() const;

      /** @name Selection Methods
       *  These methods are used to manipulate user-selected primitives.
       *  Each view tracks a list of selected objects (e.g., atoms)
       *  which can be passed to engines, tools, or used for tasks like
       *  copying selected atoms, etc.
       * @{
       */

      /**
       * @return the current selected primitives (all Primitive types)
       */
      PrimitiveList selectedPrimitives() const;

      /**
       * Toggle the selection for the atoms in the supplied list.
       * That is, if the primitive is selected, deselect it and vice-versa.
       *
       * @param primitives the set of objects to update.
       */
      void toggleSelected(PrimitiveList primitives);

      /**
       * Toggle the selection for the GLWidget, that is if the primitive is
       * selected, deselect it and vice-versa.
       */
      void toggleSelected();

      /**
       * Change the selection status for the atoms in the supplied list.
       * All objects in the list will have the same selection status.
       *
       * @param primitives the set of objects to update.
       * @param select whether to select or deselect the objects.
       */
      void setSelected(PrimitiveList primitives, bool select = true); // do we pass by value intentionally

      /**
       * Deselect all objects.
       */
      void clearSelected();

      /**
       * @return true if the Primitive is selected.
       */
      bool isSelected(const Primitive *p) const;

      /**
       * Add a new named selection.
       *
       * @param name name of the new selection.
       * @param primitives the primitives ids.
       * @return true if the name is not taken
       */
      bool addNamedSelection(const QString &name, PrimitiveList &primitives);
      /**
       * Remove a named selection by name.
       *
       * @param name name of the selection to remove.
       */
      void removeNamedSelection(const QString &name);
      /**
       * Remove a named selection by index. Using the index is useful in Models.
       *
       * @param index index of the selection to remove.
       */
      void removeNamedSelection(int index);
      /**
       * Rename a named selection by index. Using the index is useful in Models.
       *
       * @param index index of the selection to rename.
       */
      void renameNamedSelection(int index, const QString &name);
      /**
       * Get the names of all named selections.
       *
       * @return a list with all the named selections.
       */
      QList<QString> namedSelections();
      /**
       * Get the primitives of a named selections by name.
       *
       * @param name name of the selection.
       * @return the primitives for this named selection.
       */
      PrimitiveList namedSelectionPrimitives(const QString &name);
      /**
       * Get the primitives of a named selections by index.
       * Using the index is useful in Models.
       *
       * @param index index of the selection.
       * @return the primitives for this named selection.
       */
      PrimitiveList namedSelectionPrimitives(int index);
      /* end selection method grouping */
      /** @} */

      /**
       * Set the number of unit cells for a periodic molecule like a crystal
       * a, b, and c, are the three primitive unit cell axes.
       * Does nothing if the molecule does not have a unit cell defined.
       *
       * @param a number of unit cells to display along the a axis.
       * @param b number of unit cells to display along the b axis.
       * @param c number of unit cells to display along the c axis.
       */
      void setUnitCells(int a, int b, int c);

      /**
       * Clear the unit cell data.
       */
      void clearUnitCell();

      /**
       * @return The number of unit cells to display along the a axis.
       */
      int aCells();

      /**
       * @return The number of unit cells to display along the b axis.
       */
      int bCells();

      /**
       * @return The number of unit cells to display along the c axis.
       */
      int cCells();

      /**
       * Static pointer to the current GLWidget.
       */
      static GLWidget *m_current;

      /**
       * @return a pointer to the current GLWidget.
       */
      static GLWidget *current();

      /**
       * Set this instance of the GLWidget as the current GLWidget instance.
       */
      static void setCurrent(GLWidget *current);

      /**
       * Trigger a render for GL2PS.
       */
      void renderNow();

      /**
       * Write the settings of the GLWidget in order to save them to disk.
       */
      virtual void writeSettings(QSettings &settings) const;

      /**
       * Read the settings of the GLWidget and restore them.
       */
      virtual void readSettings(QSettings &settings);


    protected:
      friend class GLGraphicsView;
      /**
       * Virtual function called by QGLWidget on initialization of
       * the GL area.
       */
      virtual void initializeGL();

      /**
       * Virtual function called by GLWidget before render() to set up the
       * display correctly.
       */
      virtual void paintGL();

      /**
       * Virtual function called by GLGraphicsView before render() to set up the
       * display correctly and leave it in an appropriate state after.
       */
      virtual void paintGL2();

      /**
       * Virtual function called whn the GL area is resized
       */
      virtual void resizeGL(int, int);

      /**
       * Virtual function called when the GL area needs repainting.
       */
      virtual void paintEvent(QPaintEvent *event);

      /**
       * Called on resize of the GLWidget to perform resizing of the display.
       */
      virtual void resizeEvent(QResizeEvent *event);

      /**
       * Focus Event
       */
      virtual bool event(QEvent *event);

      /**
       * Virtual function reaction to mouse press in the GL rendering area.
       */
      virtual void mousePressEvent(QMouseEvent * event);

      /**
       * Virtual function reaction to mouse release in the GL rendering area.
       */
      virtual void mouseReleaseEvent(QMouseEvent * event);

      /**
       * Virtual function reaction to mouse being moved in the GL rendering area.
       */
      virtual void mouseMoveEvent(QMouseEvent * event);

      /**
       * Virtual function reaction to mouse while in the GL rendering area.
       */
      virtual void wheelEvent(QWheelEvent * event);

      /**
       * Response to key press events.
       * @param event the key event information
       */
      virtual void keyPressEvent(QKeyEvent *event);

      /**
       * Response to key release events.
       * @param event the key event information
       */
      virtual void keyReleaseEvent(QKeyEvent *event);

      /**
       * Render the scene. To be used in both modes GL_RENDER and GL_SELECT.
       * This function calls the render functions of the engines as well as the
       * paint events of the tools and is where everything drawn onto the widget
       * is called from.
       */
      virtual void render();

      /**
       * Render a full crystal cell
       * Called by render() automatically
       *
       * @param displayList the display list of the primitive unit cell
       */
      virtual void renderCrystal(GLuint displayList);

      /**
       * Render crystal unit cell axes
       * called by renderCrystal() automatically
       *
       */
      virtual void renderCrystalAxes();

      /**
       * Render x, y, z axes as an overlay on the bottom left of the widget.
       */
      virtual void renderAxesOverlay();

      /**
       * Render a debug overlay with extra debug information on the GLWidget.
       */
      virtual void renderDebugOverlay();

      /**
       * This will return a painting condition that must be met each time
       * before a GLThread can run.
       *
       * @return painting condition.
       */
//       QWaitCondition *paintCondition() const;


    private:
      GLWidgetPrivate * const d;

      /**
       * Helper function called by all constructors.
       */
      void constructor(const GLWidget *shareWidget =0);

      /**
       * Compute the average frames per second over the last 200+ ms.
       */
      inline double computeFramesPerSecond();

      bool              m_glslEnabled;
      Tool*             m_navigateTool; /// NavigateTool is a super tool

    public Q_SLOTS:

      /**
       * Set the active Tool of the GLWidget.
       */
      void setTool(Tool *tool);

      /**
       * Set the background color of the rendering area (the default is black).
       *
       * @param background the new background color.
       */
      void setBackground(const QColor &background);

      /**
       * Set the molecule model for this view.
       * @param molecule the molecule to view.
       */
      void setMolecule(Molecule *molecule);

      /**
       * Add an engine to the GLWidget. 
       * @param engine Engine to add to this widget.
       */
      void addEngine(Engine *engine);

      /**
       * Remove an engine from the GLWidget. 
       * @param engine Engine to remove from this widget.
       */
      void removeEngine(Engine *engine);

      /**
       * Reset to default engines (one of each factory).
       */
      void loadDefaultEngines();

      /**
       * Reload the engine plugins.
       */
      void reloadEngines();

      /**
       * Signal that something changed and the display lists should be invalidated.
       */
      void invalidateDLs();

      /**
       * One or more tools are deleted..
       */
      void toolsDestroyed();

    Q_SIGNALS:
      /**
       * Signal for the mouse press event which is passed to the tools.
       */
      void mousePress(QMouseEvent * event);

      /**
       * Signal for the mouse release event which is passed to the tools.
       */
      void mouseRelease( QMouseEvent * event );

      /**
       * Signal for the mouse move event which is passed to the tools.
       */
      void mouseMove( QMouseEvent * event );

      /**
       * Signal for the mouse wheel event which is passed to the tools.
       */
      void wheel( QWheelEvent * event);

      /**
       * Signal that the Molecule has changed.
       */
      void moleculeChanged(Molecule *newMolecule);

      /**
       * Signal that an Engine has been added to the GLWidget.
       */
      void engineAdded(Engine *engine);

      /**
       * Signal that an Engine has been removed from the GLWidget.
       */
      void engineRemoved(Engine *engine);

      /**
       * Signal that the GLWidget has resized. Can be used to update child
       * widgets, see overlay extension for example.
       */
      void resized();

      /**
       * Named selections have changed.
       */
      void namedSelectionsChanged();

      /**
       * Signal that this GLWidget was activated.
       */
      void activated(GLWidget *);

  };

} // end namespace Avogadro

#endif
