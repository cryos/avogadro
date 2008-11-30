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
    .def("setQuickRenderEnabled", &GLWidget::setQuickRenderEnabled)
    .def("isQuickRenderEnabled", &GLWidget::isQuickRenderEnabled)
    .def("deviceWidth", &GLWidget::deviceWidth)
    .def("deviceHeight", &GLWidget::deviceHeight)
    .def("radius", radius_ptr1, return_value_policy<copy_const_reference>())
    .def("radius", radius_ptr2)
    .def("tool", &GLWidget::tool, return_value_policy<reference_existing_object>())
    .def("background", &GLWidget::background)
    .def("setColorMap", &GLWidget::setColorMap)
    .def("colorMap", &GLWidget::colorMap, return_value_policy<reference_existing_object>())
    //.def("setUndoStack", &GLWidget::setUndoStack)
    //.def("undoStack", &GLWidget::undoStack)
    .def("molecule", molecule_ptr, return_value_policy<reference_existing_object>())
    .def("updateGeometry", &GLWidget::updateGeometry)
    .def("camera", &GLWidget::camera, return_value_policy<reference_existing_object>())
    .def("engines", &GLWidget::engines)
    .def("hits", &GLWidget::hits)
    .def("computeClickedPrimitive", &GLWidget::computeClickedPrimitive, return_value_policy<reference_existing_object>())
    .def("computeClickedAtom", &GLWidget::computeClickedAtom, return_value_policy<reference_existing_object>())
    .def("computeClickedBond", &GLWidget::computeClickedBond, return_value_policy<reference_existing_object>())
    .def("center", &GLWidget::center, return_value_policy<return_by_value>())
    .def("normalVector", &GLWidget::normalVector, return_value_policy<return_by_value>())
    .def("farthestAtom", &GLWidget::farthestAtom, return_value_policy<reference_existing_object>())
    .def("setQuality", &GLWidget::setQuality)
    .def("quality", &GLWidget::quality)
    .def("setFogLevel", &GLWidget::setFogLevel)
    .def("fogLevel", &GLWidget::fogLevel)
    .def("setRenderAxes", &GLWidget::setRenderAxes)
    .def("renderAxes", &GLWidget::renderAxes)
    .def("setRenderDebug", &GLWidget::setRenderDebug)
    .def("renderDebug", &GLWidget::renderDebug)
    .def("renderPrimitives", &GLWidget::renderPrimitives)
    .def("setToolGroup", &GLWidget::setToolGroup)
    .def("toolGroup", &GLWidget::toolGroup, return_value_policy<reference_existing_object>())
    .def("painter", &GLWidget::painter, return_value_policy<reference_existing_object>())
    .def("primitives", &GLWidget::primitives)
    .def("selectedPrimitives", &GLWidget::selectedPrimitives)
    .def("toggleSelected", &GLWidget::toggleSelected)
    .def("setSelected", &GLWidget::setSelected)
    .def("clearSelected", &GLWidget::clearSelected)
    .def("isSelected", &GLWidget::isSelected)
    .def("addNamedSelection", &GLWidget::addNamedSelection)
    .def("removeNamedSelection", removeNamedSelection_ptr1)
    .def("removeNamedSelection", removeNamedSelection_ptr2)
    .def("renameNamedSelection", &GLWidget::renameNamedSelection)
    .def("namedSelections", &GLWidget::namedSelections)
    .def("namedSelectionPrimitives", namedSelectionPrimitives_ptr1)
    .def("namedSelectionPrimitives", namedSelectionPrimitives_ptr2)
    .def("setUnitCells", &GLWidget::setUnitCells)
    .def("clearUnitCell", &GLWidget::clearUnitCell)
    .def("aCells", &GLWidget::aCells)
    .def("bCells", &GLWidget::bCells)
    .def("cCells", &GLWidget::cCells)
    .def("current", &GLWidget::current, return_value_policy<reference_existing_object>())
    .staticmethod("current")
    .def("setCurrent", &GLWidget::setCurrent)
    .def("writeSettings", &GLWidget::writeSettings)
    .def("readSettings", &GLWidget::readSettings)
/*
    public Q_SLOTS:
      void setTool(Tool *tool);
      void addPrimitive(Primitive *primitive);
      void updatePrimitive(Primitive *primitive);
      void removePrimitive(Primitive *primitive);
      void setBackground(const QColor &background);
      void setMolecule(Molecule *molecule);
      void addEngine(Engine *engine);
      void removeEngine(Engine *engine);
      void loadDefaultEngines();
      void invalidateDLs();
      */
    ;

} 

