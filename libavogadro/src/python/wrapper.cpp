#ifndef BOOST_H
#define BOOST_H

#include <Python.h>
#include <Numeric/arrayobject.h> 
#include <boost/python.hpp>
#include <boost/python/numeric.hpp>
#include <boost/python/tuple.hpp>

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
/*
color.h
cylinder.h
elementtranslate.h
engine.h
extension.h
glwidget.h
mesh.h
navigate.h
painterdevice.h
painter.h
periodictableview.h
plugin.h
pluginmanager.h
primitivelist.h
sphere.h
toolgroup.h
tool.h
undosequence.h
*/

using namespace boost::python;
using namespace Avogadro;

#include <iostream>

/*
 * Handle automatic convertions:
 * 
 * Vector3d <-> python array
 */
//namespace Eigen { namespace {

  struct Vector3d_to_python_array
  {
    static PyObject* convert(Eigen::Vector3d const &vec)
    {
      return incref(numeric::array(make_tuple(vec.x(), vec.y(), vec.z())).ptr());
    }
    
    static PyObject* convert(Eigen::Vector3d *vec)
    {
      if (!vec)
        throw_error_already_set();
      
      return incref(numeric::array(make_tuple(vec->x(), vec->y(), vec->z())).ptr());
    }

    static PyObject* convert(const Eigen::Vector3d *vec)
    {
      if (!vec)
        throw_error_already_set();

      return incref(numeric::array(make_tuple(vec->x(), vec->y(), vec->z())).ptr());
    }
  };

  struct Vector3d_from_python_array
  {
    Vector3d_from_python_array()
    {
      converter::registry::push_back( &convertible, &construct, type_id<Eigen::Vector3d>() );
    }

    static void* convertible(PyObject *obj_ptr)
    {
      if (!PyArray_Check(obj_ptr))
        return 0;
      return obj_ptr;
    }

    static void construct(PyObject *obj_ptr, converter::rvalue_from_python_stage1_data *data)
    {
      PyArrayObject *array = reinterpret_cast<PyArrayObject*>(obj_ptr);

      // check the dimensions
      if (array->nd != 1)
        throw_error_already_set(); // the array has at least two dimensions (matrix)
  
      if (array->dimensions[0] != 3)
        throw_error_already_set(); // the 1D array does not have exactly 3 elements

      double *values = reinterpret_cast<double*>(array->data);
    
      void *storage = ((converter::rvalue_from_python_storage<Eigen::Vector3d>*)data)->storage.bytes;
      new (storage) Eigen::Vector3d(values[0], values[1], values[2]);
      data->convertible = storage;
    }
  };

  void init_module()
  {
    import_array(); // needed for NumPy 
    to_python_converter<Eigen::Vector3d, Vector3d_to_python_array>();
    to_python_converter<Eigen::Vector3d*, Vector3d_to_python_array>();
    to_python_converter<const Eigen::Vector3d*, Vector3d_to_python_array>();
    Vector3d_from_python_array();
  }
//}}

/*
class EigenTest
{
  public:
    Eigen::Vector3d* vectorPtr() { return &m_vec; }
    Eigen::Vector3d& vectorRef() { return m_vec; }
    const Eigen::Vector3d* vectorConstPtr() { return &m_vec; }
    const Eigen::Vector3d& vectorConstRef() { return m_vec; }
 
    const Eigen::Vector3d* vectorConstPtrConst() const { return &m_vec; }
  private:
    Eigen::Vector3d m_vec;
};
*/

BOOST_PYTHON_MODULE(Avogadro) {
  
  // include the Eigen converter(s)
  init_module();

  /*
  class_<EigenTest>("EigenTest")
    .def("vectorPtr", &EigenTest::vectorPtr, return_value_policy<return_by_value>())
    .def("vectorRef", &EigenTest::vectorRef, return_value_policy<return_by_value>())
    .def("vectorConstPtr", &EigenTest::vectorConstPtr, return_value_policy<return_by_value>())
    .def("vectorConstRef", &EigenTest::vectorConstRef, return_value_policy<return_by_value>())
    .def("vectorConstPtrConst", &EigenTest::vectorConstPtrConst, return_value_policy<return_by_value>())
    ;
  */

  ////////////////////////////////////////////////////////////////////
  //
  // Primitive
  //
  //////////////////////////////////////////////////////////////////// 

  class_<Avogadro::Primitive, boost::noncopyable>("Primitive")
    .def("id", &Primitive::id)
    .def("index", &Primitive::index)
    .def("update", &Primitive::update)
    ;
 
  ////////////////////////////////////////////////////////////////////
  //
  // Bond
  //
  //////////////////////////////////////////////////////////////////// 
 
  class_<Avogadro::Bond, bases<Avogadro::Primitive>, boost::noncopyable>("Bond", no_init)
    .def("setBegin", &Bond::setBegin)
    .def("beginAtomId", &Bond::beginAtomId)
    .def("setEnd", &Bond::setEnd)
    .def("endAtomId", &Bond::endAtomId)
    .def("setAtoms", &Bond::setAtoms)
    .def("otherAtom", &Bond::otherAtom)
    .def("order", &Bond::order)
    .def("setOrder", &Bond::setOrder)
    .def("length", &Bond::length)
    ;
 
  ////////////////////////////////////////////////////////////////////
  //
  // Atom
  //
  //////////////////////////////////////////////////////////////////// 

  // define function pointers to handle overloading
  void (Atom::*setPos_ptr)(const Eigen::Vector3d&) = &Atom::setPos;
  void (Atom::*addBond_ptr)(unsigned long int) = &Atom::addBond;
  void (Atom::*deleteBond_ptr)(unsigned long int) = &Atom::deleteBond;

  class_<Avogadro::Atom, bases<Avogadro::Primitive>, boost::noncopyable>("Atom", no_init)
    .def("pos", &Atom::pos, return_value_policy<return_by_value>())
    .def("setPos", setPos_ptr)
    .def("atomicNumber", &Atom::atomicNumber)
    .def("setAtomicNumber", &Atom::setAtomicNumber)
    .def("addBond", addBond_ptr)
    .def("deleteBond", deleteBond_ptr)
    .def("bonds", &Atom::bonds)
    .def("neighbors", &Atom::neighbors)
    .def("valence", &Atom::valence)
    .def("isHydrogen", &Atom::isHydrogen)
    .def("setPartialCharge", &Atom::setPartialCharge)
    .def("partialCharge", &Atom::partialCharge)
    ;

  ////////////////////////////////////////////////////////////////////
  //
  // Fragment
  //
  //////////////////////////////////////////////////////////////////// 
  
  class_<Avogadro::Fragment, bases<Avogadro::Primitive>, boost::noncopyable>("Fragment", no_init)
    .def("name", &Fragment::name)
    .def("setName", &Fragment::setName)
    .def("addAtom", &Fragment::addAtom)
    .def("removeAtom", &Fragment::removeAtom)
    //.def("atoms", &Fragment::atoms)
    .def("addBond", &Fragment::addBond)
    .def("removeBond", &Fragment::removeBond)
    //.def("bonds", &Fragment::bonds)
    ;

  ////////////////////////////////////////////////////////////////////
  //
  // Residue
  //
  //////////////////////////////////////////////////////////////////// 

  class_<Avogadro::Residue, bases<Avogadro::Fragment>, boost::noncopyable>("Residue", no_init)
    //.def("number", &Residue::number)
    //.def("setNumber", &Residue::setNumber)
    .def("chainNumber", &Residue::chainNumber)
    .def("setChainNumber", &Residue::setChainNumber)
    //.def("atomId", &Residue::atomId)
    //.def("atomIds", &Residue::atomIds)
    //.def("setAtomId", &Residue::setAtomId)
    //.def("setAtomIds", &Residue::setAtomIds)
    ;

  ////////////////////////////////////////////////////////////////////
  //
  // Cube
  //
  //////////////////////////////////////////////////////////////////// 

  bool (Cube::*setLimits_ptr1)(const Eigen::Vector3d &, const Eigen::Vector3d &, double) = &Cube::setLimits;
  bool (Cube::*setLimits_ptr2)(const Eigen::Vector3d &, const Eigen::Vector3i &, double) = &Cube::setLimits;
  bool (Cube::*setLimits_ptr3)(const Molecule *, double, double) = &Cube::setLimits;
  double (Cube::*value_ptr1)(int, int, int) const = &Cube::value;
  double (Cube::*value_ptr2)(const Eigen::Vector3i &) const = &Cube::value;
  double (Cube::*value_ptr3)(const Eigen::Vector3d &) const = &Cube::value;

  class_<Avogadro::Cube, bases<Avogadro::Primitive>, boost::noncopyable>("Cube", no_init)
    .def("min", &Cube::min)
    .def("max", &Cube::max)
    .def("spacing", &Cube::spacing)
    .def("dimensions", &Cube::dimensions)
    .def("setLimits", setLimits_ptr1)
    .def("setLimits", setLimits_ptr2)
    .def("setLimits", setLimits_ptr3)
    .def("data", &Cube::data)
    .def("setData", &Cube::setData)
    .def("closestIndex", &Cube::closestIndex)
    .def("indexVector", &Cube::indexVector)
    .def("position", &Cube::position)
    .def("value", value_ptr1)
    .def("value", value_ptr2)
    .def("value", value_ptr3)
    .def("setValue", &Cube::setValue)
    .def("name", &Cube::name)
    .def("setName", &Cube::setName)
    ;

  ////////////////////////////////////////////////////////////////////
  //
  // Molecule
  //
  //////////////////////////////////////////////////////////////////// 

  // define function pointers to handle overloading
  Atom* (Molecule::*newAtom_ptr1)() = &Molecule::newAtom;
  Atom* (Molecule::*newAtom_ptr2)(unsigned long int) = &Molecule::newAtom;
  void (Molecule::*deleteAtom_ptr1)(Atom*) = &Molecule::deleteAtom;
  void (Molecule::*deleteAtom_ptr2)(unsigned long int) = &Molecule::deleteAtom;
  Bond* (Molecule::*newBond_ptr1)() = &Molecule::newBond;
  Bond* (Molecule::*newBond_ptr2)(unsigned long int) = &Molecule::newBond;
  void (Molecule::*deleteBond_ptr1)(Bond*) = &Molecule::deleteBond;
  void (Molecule::*deleteBond_ptr2)(unsigned long int) = &Molecule::deleteBond;
  void (Molecule::*deleteCube_ptr1)(Cube*) = &Molecule::deleteCube;
  void (Molecule::*deleteCube_ptr2)(unsigned long int) = &Molecule::deleteCube;
  void (Molecule::*deleteResidue_ptr1)(Residue*) = &Molecule::deleteResidue;
  void (Molecule::*deleteResidue_ptr2)(unsigned long int) = &Molecule::deleteResidue;
  void (Molecule::*deleteRing_ptr1)(Fragment*) = &Molecule::deleteRing;
  void (Molecule::*deleteRing_ptr2)(unsigned long int) = &Molecule::deleteRing;
  Atom* (Molecule::*atom_ptr)(int) = &Molecule::atom;
  Bond* (Molecule::*bond_ptr1)(int) = &Molecule::bond;
  Bond* (Molecule::*bond_ptr2)(unsigned long, unsigned long) = &Molecule::bond;
  Bond* (Molecule::*bond_ptr3)(const Atom*, const Atom*) = &Molecule::bond;

  class_<Avogadro::Molecule, bases<Avogadro::Primitive>, boost::noncopyable>("Molecule")
    .def("setFileName", &Molecule::setFileName)
    .def("fileName", &Molecule::fileName)
    .def("newAtom", newAtom_ptr1, return_value_policy<reference_existing_object>())
    .def("newAtom", newAtom_ptr2, return_value_policy<reference_existing_object>())
    .def("setAtomPos", &Molecule::setAtomPos)
    .def("atomPos", &Molecule::atomPos, return_value_policy<return_by_value>())
    .def("deleteAtom", deleteAtom_ptr1)
    .def("deleteAtom", deleteAtom_ptr2)
    .def("newBond", newBond_ptr1, return_value_policy<reference_existing_object>())
    .def("newBond", newBond_ptr2, return_value_policy<reference_existing_object>())
    .def("deleteBond", deleteBond_ptr1)
    .def("deleteBond", deleteBond_ptr2)
    .def("addHydrogens", &Molecule::addHydrogens)
    .def("deleteHydrogens", &Molecule::deleteHydrogens)
    .def("calculatePartialCharges", &Molecule::calculatePartialCharges)
    .def("newCube", &Molecule::newCube, return_value_policy<reference_existing_object>())
    .def("deleteCube", deleteCube_ptr1)
    .def("deleteCube", deleteCube_ptr2)
    .def("newResidue", &Molecule::newResidue, return_value_policy<reference_existing_object>())
    .def("deleteResidue", deleteResidue_ptr1)
    .def("deleteResidue", deleteResidue_ptr2)
    .def("newRing", &Molecule::newRing, return_value_policy<reference_existing_object>())
    .def("deleteRing", deleteRing_ptr1)
    .def("deleteRing", deleteRing_ptr2)
    .def("numAtoms", &Molecule::numAtoms)
    .def("numBonds", &Molecule::numBonds)
    .def("numCubes", &Molecule::numCubes)
    .def("numResidues", &Molecule::numResidues)
    .def("atom", atom_ptr, return_value_policy<reference_existing_object>())
    .def("atomById", &Molecule::atomById, return_value_policy<reference_existing_object>())
    .def("bond", bond_ptr1, return_value_policy<reference_existing_object>())
    .def("bond", bond_ptr2, return_value_policy<reference_existing_object>())
    .def("bond", bond_ptr3, return_value_policy<reference_existing_object>())
    .def("bondById", &Molecule::bondById, return_value_policy<reference_existing_object>())
    .def("residue", &Molecule::residue, return_value_policy<reference_existing_object>())
    .def("residueById", &Molecule::residueById, return_value_policy<reference_existing_object>())
    .def("atoms", &Molecule::atoms)
    .def("bonds", &Molecule::bonds)
    .def("cubes", &Molecule::cubes)
    .def("residues", &Molecule::residues)
    .def("rings", &Molecule::rings)
    .def("clear", &Molecule::clear)
    .def("center", &Molecule::center, return_value_policy<return_by_value>())
    .def("normalVector", &Molecule::normalVector, return_value_policy<return_by_value>())
    .def("radius", &Molecule::radius)
    .def("farthestAtom", &Molecule::farthestAtom, return_value_policy<reference_existing_object>())
    .def("translate", &Molecule::translate)
    ;

  ////////////////////////////////////////////////////////////////////
  //
  // Camera
  //
  //////////////////////////////////////////////////////////////////// 
  
  //Eigen::Transform3d& (Camera::*modelview_ptr)() = &Camera::modelview;
  Eigen::Vector3d (Camera::*unProject_ptr1)(const Eigen::Vector3d&) const = &Camera::unProject;
  Eigen::Vector3d (Camera::*unProject_ptr2)(const QPoint&, const Eigen::Vector3d&) const = &Camera::unProject;
  Eigen::Vector3d (Camera::*unProject_ptr3)(const QPoint&) const = &Camera::unProject;
  
  class_<Avogadro::Camera, boost::noncopyable>("Camera")
    .def("parent", &Camera::parent, return_value_policy<reference_existing_object>())
    .def("setAngleOfViewY", &Camera::setAngleOfViewY)
    .def("angleOfViewY", &Camera::angleOfViewY)
    .def("setModelview", &Camera::setModelview)
    //.def("modelview", modelview_ptr)
    .def("applyPerspective", &Camera::applyPerspective)
    .def("applyModelview", &Camera::applyModelview)
    .def("initializeViewPoint", &Camera::initializeViewPoint)
    .def("distance", &Camera::distance)
    .def("translate", &Camera::translate)
    .def("pretranslate", &Camera::pretranslate)
    .def("rotate", &Camera::rotate)
    .def("prerotate", &Camera::prerotate)
    .def("unProject", unProject_ptr1)
    .def("unProject", unProject_ptr2)
    .def("unProject", unProject_ptr3)
    .def("project", &Camera::project)
    .def("backTransformedXAxis", &Camera::backTransformedXAxis)
    .def("backTransformedYAxis", &Camera::backTransformedYAxis)
    .def("backTransformed2Axis", &Camera::backTransformedZAxis)
    .def("transformedXAxis", &Camera::transformedXAxis)
    .def("transformedYAxis", &Camera::transformedYAxis)
    .def("transformedZAxis", &Camera::transformedZAxis)
    .def("normalize", &Camera::normalize)
    ;

  ////////////////////////////////////////////////////////////////////
  //
  // GLWidget
  //
  //////////////////////////////////////////////////////////////////// 
 
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

#endif



