#include <Python.h>
#include <boost/python.hpp>

#include <avogadro/atom.h>
#include <avogadro/bond.h>
#include <avogadro/fragment.h>
#include <avogadro/residue.h>
#include <avogadro/cube.h>
#include <avogadro/molecule.h>
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

void export_GLWidget()
{
  class_<Avogadro::GLHit, boost::noncopyable>("GLHit", no_init)
    .add_property("type", &GLHit::type)
    .add_property("name", &GLHit::name)
    .add_property("minZ", &GLHit::minZ)
    .add_property("maxZ", &GLHit::maxZ)
    ;

  const double & (GLWidget::*radius_ptr1)() const = &GLWidget::radius;
  double (GLWidget::*radius_ptr2)(const Primitive *) const = &GLWidget::radius;
  void (GLWidget::*toggleSelected_ptr1)() = &GLWidget::toggleSelected;
  //void (GLWidget::*toggleSelected_ptr2)(PrimitiveList list) = &GLWidget::toggleSelected;
  void (GLWidget::*removeNamedSelection_ptr1)(const QString &name) = &GLWidget::removeNamedSelection;
  void (GLWidget::*removeNamedSelection_ptr2)(int index) = &GLWidget::removeNamedSelection;
  PrimitiveList (GLWidget::*namedSelectionPrimitives_ptr1)(const QString &name) = &GLWidget::namedSelectionPrimitives;
  PrimitiveList (GLWidget::*namedSelectionPrimitives_ptr2)(int index) = &GLWidget::namedSelectionPrimitives;
  Molecule* (GLWidget::*molecule_ptr)() = &GLWidget::molecule;


  class_<Avogadro::GLWidget, /*bases<QWidget>,*/ boost::noncopyable>("GLWidget")
    // constructors
    .def(init<QWidget*>())
    .def(init<const QGLFormat&, QWidget*, const GLWidget*>())
    .def(init<Molecule*, const QGLFormat&, QWidget*, const GLWidget*>())


    // read/write properties
    .add_property("quickRender", &GLWidget::quickRender, &GLWidget::setQuickRender)
    .add_property("colorMap", make_function(&GLWidget::colorMap, return_value_policy<reference_existing_object>()),
        &GLWidget::setColorMap)
    .add_property("molecule", make_function(molecule_ptr, return_value_policy<reference_existing_object>()),
        &GLWidget::setMolecule)
    .add_property("tool", make_function(&GLWidget::tool, return_value_policy<reference_existing_object>()),
        &GLWidget::setTool)
    .add_property("quality", &GLWidget::quality, &GLWidget::setQuality)
    .add_property("fogLevel", &GLWidget::fogLevel, &GLWidget::setFogLevel)
    .add_property("renderAxes", &GLWidget::renderAxes, &GLWidget::setRenderAxes)
    .add_property("renderDebug", &GLWidget::renderDebug, &GLWidget::setRenderDebug)
    .add_property("toolGroup", make_function(&GLWidget::toolGroup, return_value_policy<reference_existing_object>()),
        &GLWidget::setToolGroup)
    .add_property("background", &GLWidget::background, &GLWidget::setBackground)
    .add_property("undoStack", make_function(&GLWidget::undoStack, return_value_policy<return_by_value>()),
        &GLWidget::setUndoStack)

    // read-only properties
    .add_property("deviceWidth", &GLWidget::deviceWidth)
    .add_property("deviceHeight", &GLWidget::deviceHeight)
    .add_property("camera", make_function(&GLWidget::camera, return_value_policy<reference_existing_object>()))
    .add_property("engines", &GLWidget::engines)
    .add_property("center", make_function(&GLWidget::center, return_value_policy<return_by_value>()))
    .add_property("normalVector", make_function(&GLWidget::normalVector, return_value_policy<return_by_value>()))
    .add_property("farthestAtom", make_function(&GLWidget::farthestAtom, return_value_policy<reference_existing_object>()))
    .add_property("painter", make_function(&GLWidget::painter, return_value_policy<reference_existing_object>()))
    .add_property("selectedPrimitives", &GLWidget::selectedPrimitives)
    .add_property("namedSelections", &GLWidget::namedSelections)
    .add_property("aCells", &GLWidget::aCells)
    .add_property("bCells", &GLWidget::bCells)
    .add_property("cCells", &GLWidget::cCells)


    // real functions
    .def("radius", radius_ptr1, return_value_policy<copy_const_reference>())
    .def("radius", radius_ptr2)
    .def("updateGeometry", &GLWidget::updateGeometry)
    .def("hits", &GLWidget::hits)
    .def("computeClickedPrimitive", &GLWidget::computeClickedPrimitive, return_value_policy<reference_existing_object>())
    .def("computeClickedAtom", &GLWidget::computeClickedAtom, return_value_policy<reference_existing_object>())
    .def("computeClickedBond", &GLWidget::computeClickedBond, return_value_policy<reference_existing_object>())
    .def("renderPrimitives", &GLWidget::renderPrimitives)
    .def("toggleSelected", toggleSelected_ptr1)
    .def("setSelected", &GLWidget::setSelected)
    .def("clearSelected", &GLWidget::clearSelected)
    .def("isSelected", &GLWidget::isSelected)
    .def("addNamedSelection", &GLWidget::addNamedSelection)
    .def("removeNamedSelection", removeNamedSelection_ptr1)
    .def("removeNamedSelection", removeNamedSelection_ptr2)
    .def("renameNamedSelection", &GLWidget::renameNamedSelection)
    .def("namedSelectionPrimitives", namedSelectionPrimitives_ptr1)
    .def("namedSelectionPrimitives", namedSelectionPrimitives_ptr2)
    .def("setUnitCells", &GLWidget::setUnitCells)
    .def("clearUnitCell", &GLWidget::clearUnitCell)
    .def("current", &GLWidget::current, return_value_policy<reference_existing_object>())
    .staticmethod("current")
    .def("setCurrent", &GLWidget::setCurrent)
    .def("writeSettings", &GLWidget::writeSettings)
    .def("readSettings", &GLWidget::readSettings)
    .def("loadDefaultEngines", &GLWidget::loadDefaultEngines)
    .def("addPrimitive", &GLWidget::addPrimitive)
    .def("updatePrimitive", &GLWidget::updatePrimitive)
    .def("removePrimitive", &GLWidget::removePrimitive)
    .def("addEngine", &GLWidget::addEngine)
    .def("removeEngine", &GLWidget::removeEngine)
    .def("invalidateDLs", &GLWidget::invalidateDLs)
    ;

}

