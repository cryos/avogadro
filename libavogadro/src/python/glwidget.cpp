// Last update: timvdm 18 June 2009
#include <boost/python.hpp>

#include <avogadro/atom.h>
#include <avogadro/bond.h>
#include <avogadro/fragment.h>
#include <avogadro/residue.h>
#include <avogadro/cube.h>
#include <avogadro/molecule.h>
#include <avogadro/glhit.h>
#include <avogadro/glwidget.h>
#include <avogadro/painter.h>
#include <avogadro/camera.h>
#include <avogadro/toolgroup.h>
#include <avogadro/tool.h>
#include <avogadro/color.h>
#include <avogadro/primitivelist.h>
#include <avogadro/engine.h>

#include <QWidget>
#include <QUndoStack>
#include <QPoint>
#include <QColor>

using namespace boost::python;
using namespace Avogadro;
  
void setSelected(GLWidget &self, PrimitiveList primitives)
{
  self.setSelected(primitives);
}

void export_GLWidget()
{
  class_<Avogadro::GLHit, boost::noncopyable>("GLHit", no_init)
    .add_property("type", &GLHit::type)
    .add_property("name", &GLHit::name)
    .add_property("minZ", &GLHit::minZ)
    .add_property("maxZ", &GLHit::maxZ)
    ;

  double (GLWidget::*radius_ptr1)() const = &GLWidget::radius;
  double (GLWidget::*radius_ptr2)(const Primitive *) const = &GLWidget::radius;
  void (GLWidget::*toggleSelected_ptr1)() = &GLWidget::toggleSelected;
  //void (GLWidget::*toggleSelected_ptr2)(PrimitiveList list) = &GLWidget::toggleSelected;
  void (GLWidget::*removeNamedSelection_ptr1)(const QString &name) = &GLWidget::removeNamedSelection;
  void (GLWidget::*removeNamedSelection_ptr2)(int index) = &GLWidget::removeNamedSelection;
  PrimitiveList (GLWidget::*namedSelectionPrimitives_ptr1)(const QString &name) = &GLWidget::namedSelectionPrimitives;
  PrimitiveList (GLWidget::*namedSelectionPrimitives_ptr2)(int index) = &GLWidget::namedSelectionPrimitives;
  Molecule* (GLWidget::*molecule_ptr)() = &GLWidget::molecule;


  class_<Avogadro::GLWidget, boost::noncopyable, std::auto_ptr<Avogadro::GLWidget> >("GLWidget")
    // constructors
    .def(init<QWidget*>())
    .def(init<const QGLFormat&>())
    .def(init<const QGLFormat&, QWidget*>())
    .def(init<const QGLFormat&, QWidget*, const GLWidget*>())
    .def(init<Molecule*, const QGLFormat&>())
    .def(init<Molecule*, const QGLFormat&, QWidget*>())
    .def(init<Molecule*, const QGLFormat&, QWidget*, const GLWidget*>())
    //
    // read/write properties
    //
    .add_property("quickRender", 
        &GLWidget::quickRender, 
        &GLWidget::setQuickRender,
        "True if quick rendering is being used.")

    .add_property("renderUnitCellAxes", 
        &GLWidget::renderUnitCellAxes, 
        &GLWidget::setRenderUnitCellAxes, 
        "True if unit cell axes are being rendered.")

    .add_property("colorMap", 
        make_function(&GLWidget::colorMap, return_value_policy<reference_existing_object>()),
        &GLWidget::setColorMap, 
        "The current global color map for Primitives.")

    .add_property("molecule", 
        make_function(molecule_ptr, return_value_policy<reference_existing_object>()),
        &GLWidget::setMolecule, 
        "The current Molecule being viewed.")

    .add_property("tool", 
        make_function(&GLWidget::tool, return_value_policy<reference_existing_object>()),
        &GLWidget::setTool, 
        "The active Tool.")

    .add_property("quality", 
        &GLWidget::quality, 
        &GLWidget::setQuality, 
        "The global quality of the widget.")

    .add_property("fogLevel", 
        &GLWidget::fogLevel, 
        &GLWidget::setFogLevel, 
        "The global fog level of the widget.")

    .add_property("renderAxes", 
        &GLWidget::renderAxes, 
        &GLWidget::setRenderAxes, 
        "True if the x, y, z axes are being rendered.")

    .add_property("renderDebug", 
        &GLWidget::renderDebug, 
        &GLWidget::setRenderDebug,
        "True if the debug panel is being drawn")

    .add_property("toolGroup", 
        make_function(&GLWidget::toolGroup, return_value_policy<reference_existing_object>()),
        &GLWidget::setToolGroup, 
        "The ToolGroup of the GLWidget.")

    .add_property("background", 
        &GLWidget::background, 
        &GLWidget::setBackground,
        "The current background color of the rendering area.")

    .add_property("undoStack", 
        make_function(&GLWidget::undoStack, return_value_policy<return_by_value>()),
        &GLWidget::setUndoStack, 
        "The current GLWidget undoStack.")
    
    //
    // read-only properties
    //
    .add_property("deviceWidth", 
        &GLWidget::deviceWidth, 
        "The width of the widget in pixels.")

    .add_property("deviceHeight", 
        &GLWidget::deviceHeight, 
        "The height of the widget in pixels.")

    .add_property("camera", 
        make_function(&GLWidget::camera, return_value_policy<reference_existing_object>()),
        "The Camera of this widget.")

    .add_property("engines", 
        &GLWidget::engines, 
        "A list of engines.")

    .add_property("center", 
        make_function(&GLWidget::center, return_value_policy<return_by_value>()),
        "The point at the center of the Molecule.")

    .add_property("normalVector", 
        make_function(&GLWidget::normalVector, return_value_policy<return_by_value>()),
        "The normalVector for the entire Molecule.")

    .add_property("farthestAtom", 
        make_function(&GLWidget::farthestAtom, return_value_policy<reference_existing_object>()),
        "The Atom farthest away from the camera.")

    .add_property("painter", 
        make_function(&GLWidget::painter, return_value_policy<reference_existing_object>()),
        "The Painter of this widget.")

    .add_property("selectedPrimitives", 
        &GLWidget::selectedPrimitives, 
        "The current selected primitives (all Primitive types)")

    .add_property("namedSelections", 
        &GLWidget::namedSelections, 
        "Get the names of all named selections.")

    .add_property("aCells", 
        &GLWidget::aCells, 
        "The number of unit cells to display along the a axis.")
    .add_property("bCells", 
        &GLWidget::bCells, 
        "The number of unit cells to display along the b axis.")
    .add_property("cCells", 
        &GLWidget::cCells, 
        "The number of unit cells to display along the c axis.")

    //
    // real functions
    //
    .def("radius", 
        radius_ptr2, 
        "The radius of the primitive object for this glwidget.")
    .def("radius", 
        radius_ptr1, 
        "The radius of the Molecule.")

    .def("updateGeometry", 
        &GLWidget::updateGeometry, 
        "Update the Molecule geometry.")

    .def("hits", 
        &GLWidget::hits, 
        "Get the hits for a region starting at (x, y) of size (w * h).")

    .def("computeClickedPrimitive", 
        &GLWidget::computeClickedPrimitive, return_value_policy<reference_existing_object>(),
        "Take a point and figure out which is the closest Primitive under that point.")
    .def("computeClickedAtom", 
        &GLWidget::computeClickedAtom, return_value_policy<reference_existing_object>(),
        "Take a point and figure out which is the closest Atom under that point.")
    .def("computeClickedBond", 
        &GLWidget::computeClickedBond, return_value_policy<reference_existing_object>(),
        "Take a point and figure out which is the closest Bond under that point.")

    .def("toggleSelected", 
        toggleSelected_ptr1, 
        "Toggle the selection for the GLWidget, that is if the primitive is selected, deselect it and vice-versa.")

    .def("setSelected", 
        &GLWidget::setSelected, 
        "Toggle the selection for the GLWidget, that is if the primitive is selected, deselect it and vice-versa.")

    .def("setSelected", 
        setSelected, 
        "Toggle the selection for the GLWidget, that is if the primitive is selected, deselect it and vice-versa.")

    .def("clearSelected", 
        &GLWidget::clearSelected, 
        "Deselect all objects.")

    .def("isSelected", 
        &GLWidget::isSelected, 
        "True if the Primitive is selected.")

    .def("addNamedSelection", 
        &GLWidget::addNamedSelection, 
        "Add a new named selection.")

    .def("removeNamedSelection", 
        removeNamedSelection_ptr1, 
        "Remove a named selection by name.")
    .def("removeNamedSelection", 
        removeNamedSelection_ptr2, 
        "Remove a named selection by index. Using the index is useful in Models.")

    .def("renameNamedSelection", 
        &GLWidget::renameNamedSelection, 
        "Rename a named selection by index. Using the index is useful in Models.")

    .def("namedSelectionPrimitives", 
        namedSelectionPrimitives_ptr1, 
        "Get the primitives of a named selections by name.")

    .def("namedSelectionPrimitives", 
        namedSelectionPrimitives_ptr2, 
        "Get the primitives of a named selections by index.")

    .def("setUnitCells", 
        &GLWidget::setUnitCells, 
        "Set the number of unit cells for a periodic molecule like a crystal "
        "a, b, and c, are the three primitive unit cell axes. "
        "Does nothing if the molecule does not have a unit cell defined.")

    .def("clearUnitCell", 
        &GLWidget::clearUnitCell, 
        "Clear the unit cell data.")

    .def("current", 
        &GLWidget::current, return_value_policy<reference_existing_object>(), 
        "The current GLWidget.").staticmethod("current")

    .def("setCurrent", 
        &GLWidget::setCurrent, 
        "Set this instance of the GLWidget as the current GLWidget instance.")

    .def("writeSettings", 
        &GLWidget::writeSettings, 
        "Write the settings of the GLWidget in order to save them to disk.")

    .def("readSettings", 
        &GLWidget::readSettings, 
        "Read the settings of the GLWidget and restore them.")

    .def("loadDefaultEngines", 
        &GLWidget::loadDefaultEngines, 
        "Reset to default engines (one of each factory).")

    .def("reloadEngines", 
        &GLWidget::reloadEngines, 
        "Reload the engine plugins.")

    .def("addEngine", 
        &GLWidget::addEngine, 
        "Add an engine to the GLWidget.")

    .def("removeEngine", 
        &GLWidget::removeEngine, 
        "Remove an engine from the GLWidget.")

    .def("invalidateDLs", 
        &GLWidget::invalidateDLs,
        "Signal that something changed and the display lists should be invalidated.")

    .def("toolsDestroyed", 
        &GLWidget::toolsDestroyed, 
        "One or more tools are deleted..")
    ;

}

