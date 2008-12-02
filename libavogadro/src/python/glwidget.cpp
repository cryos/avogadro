#include <Python.h>
#include <boost/python.hpp>

#include <avogadro/primitive.h>
#include <avogadro/atom.h>
#include <avogadro/bond.h>
#include <avogadro/fragment.h>
#include <avogadro/residue.h>
#include <avogadro/cube.h>
#include <avogadro/molecule.h>
#include <avogadro/glwidget.h>
#include <avogadro/camera.h>
#include <avogadro/toolgroup.h>
#include <avogadro/tool.h>
#include <avogadro/color.h>

using namespace boost::python;
using namespace Avogadro;

void export_GLWidget()
{
 
  const double & (GLWidget::*radius_ptr1)() const = &GLWidget::radius;
  double (GLWidget::*radius_ptr2)(const Primitive *) const = &GLWidget::radius; 
  void (GLWidget::*removeNamedSelection_ptr1)(const QString &name) = &GLWidget::removeNamedSelection;
  void (GLWidget::*removeNamedSelection_ptr2)(int index) = &GLWidget::removeNamedSelection;
  PrimitiveList (GLWidget::*namedSelectionPrimitives_ptr1)(const QString &name) = &GLWidget::namedSelectionPrimitives;
  PrimitiveList (GLWidget::*namedSelectionPrimitives_ptr2)(int index) = &GLWidget::namedSelectionPrimitives;
  Molecule* (GLWidget::*molecule_ptr)() = &GLWidget::molecule;
 
  class_<Avogadro::GLWidget, boost::noncopyable>("GLWidget")
    // read/write properties
    .add_property("quickRenderEnabled", &GLWidget::setQuickRenderEnabled, &GLWidget::isQuickRenderEnabled)
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
 
    //.def("background", &GLWidget::background)  // QColor
    //void setBackground(const QColor &background); // QColor
    //.def("setUndoStack", &GLWidget::setUndoStack) // QUndoStack
    //.def("undoStack", &GLWidget::undoStack) // QUndoStack
 
    // read-only properties
    .add_property("deviceWidth", &GLWidget::deviceWidth)
    .add_property("deviceHeight", &GLWidget::deviceHeight)
    .add_property("camera", make_function(&GLWidget::camera, return_value_policy<reference_existing_object>()))
    .add_property("engines", &GLWidget::engines)
    .add_property("center", make_function(&GLWidget::center, return_value_policy<return_by_value>()))
    .add_property("normalVector", make_function(&GLWidget::normalVector, return_value_policy<return_by_value>()))
    .add_property("farthestAtom", make_function(&GLWidget::farthestAtom, return_value_policy<reference_existing_object>()))
    .add_property("painter", make_function(&GLWidget::painter, return_value_policy<reference_existing_object>()))
    .add_property("primitives", &GLWidget::primitives)
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
    .def("toggleSelected", &GLWidget::toggleSelected)
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
/*
    public Q_SLOTS:
      void addPrimitive(Primitive *primitive);
      void updatePrimitive(Primitive *primitive);
      void removePrimitive(Primitive *primitive);
      void addEngine(Engine *engine);
      void removeEngine(Engine *engine);
      void loadDefaultEngines();
      void invalidateDLs();
      */
    ;

} 

