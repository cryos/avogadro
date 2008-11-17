#ifndef BOOST_H
#define BOOST_H

#include <Python.h>
#include <boost/python.hpp>
#include <avogadro/primitive.h>

#include <avogadro/molecule.h>
#include <avogadro/atom.h>
#include <avogadro/bond.h>

using namespace boost::python;

using namespace Avogadro;
using namespace OpenBabel;

struct PySwigObject {
  PyObject_HEAD
  void * ptr;
  const char * desc;
};

// get the pointer to a OpenBabel:: object from its SWIG object
void* extract_swig_wrapped_pointer(PyObject* obj)
{
  char thisStr[] = "this";
  //first we need to get the this attribute from the Python Object
  if (!PyObject_HasAttrString(obj, thisStr))
    return NULL;

  PyObject* thisAttr = PyObject_GetAttrString(obj, thisStr);
  if (thisAttr == NULL)
    return NULL;

  //This Python Object is a SWIG Wrapper and contains our pointer
  return (((PySwigObject*)thisAttr)->ptr);
}


BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(OBMol_AddBondOverloads, OBMol::AddBond, 3, 4)
/*
BOOST_PYTHON_MODULE(Avogadro) {

  class_<Avogadro::Bond, bases<Avogadro::Primitive>, boost::noncopyable>("Bond")
    .def("length", length)
    .def("beginAtomId", beginAtomId)
    .def("endAtomId", endAtomId)
    .def("order", order)
    .def("setOrder", setOrder);

  class_<Avogadro::Atom, bases<Avogadro::Primitive>, boost::noncopyable>("Atom")
    .def("setAtomicNumber", setAtomicNumber)
    .def("setPos", setPos)
    .def("index", index)
    .def("atomicNumber", atomicNumber)
    .def("GetX", &OBAtom::GetX)
    .def("GetY", &OBAtom::GetY)
    .def("GetZ", &OBAtom::GetZ)
    .def("partialCharge", partialCharge);

  class_<OpenBabel::OBMol>("OBMol");

// OVerloaded methods, so we give them aliases
	void (OBMol::*SetTitle)(const char *) = &OBMol::SetTitle;
	void (OBMol::*ToInertialFrame)() = &OBMol::ToInertialFrame;
	void (OBMol::*Center)() = &OBMol::Center;
	bool (OBMol::*DeleteHydrogens)() = &OBMol::DeleteHydrogens;
	bool (OBMol::*AddHydrogens)(bool, bool, double) = &OBMol::AddHydrogens;
	OBBond * (OBMol::*GetBondIDX)(int)const = &OBMol::GetBond;
	OBBond * (OBMol::*GetBondAtoms)(int, int)const = &OBMol::GetBond;
    bool (OBMol::*AddBondValues)(int, int, int, int, int) = &OBMol::AddBond;


  class_<Avogadro::Molecule, bases<OpenBabel::OBMol> >("Molecule")
    .def(init<Molecule *>())
    //.add_property("atoms", range(&OBMol::BeginAtoms(), &OBMol::EndAtoms()))
    .def("BeginModify", &OBMol::BeginModify)
    .def("EndModify", &OBMol::EndModify)
    .def("NumAtoms", &OBMol::NumAtoms)
    .def("NumBonds", &OBMol::NumBonds)
    .def("NumResidues", &OBMol::NumResidues)
    .def("GetFormula", &OBMol::GetFormula)
    .def("GetTitle", &OBMol::GetTitle)
    .def("GetMolWt", &OBMol::GetMolWt)
    .def("GetTotalCharge", &OBMol::GetTotalCharge)
    .def("GetSpinMultiplicity", &OBMol::GetTotalSpinMultiplicity)
    .def("NewAtom", &OBMol::NewAtom, return_value_policy<reference_existing_object>())
    .def("AddAtom", &OBMol::AddAtom)
    .def("DeleteAtom", &OBMol::DeleteAtom)
    .def("DeleteBond", &OBMol::DeleteBond)
    .def("SetTitle", SetTitle)
    .def("SetTotalCharge", &OBMol::SetTotalCharge)
    .def("SetSpinMultiplicity", &OBMol::SetTotalSpinMultiplicity)
    .def("ToInertialFrame", ToInertialFrame)
    .def("Center", Center)
    .def("StripSalts", &OBMol::StripSalts)
    .def("AddPolarHydrogens", &OBMol::AddPolarHydrogens)
    .def("AddHydrogens", AddHydrogens)
    .def("DeleteHydrogens", DeleteHydrogens)
    .def("deleteLater", &Molecule::deleteLater)
    .def("radius", &Molecule::radius, return_value_policy<copy_const_reference>() )
    .def("update", &Molecule::update)
    .def("AddBond", AddBondValues, OBMol_AddBondOverloads())
    .def("GetBond", GetBondIDX, return_value_policy<reference_existing_object>()) //, return_internal_reference<1> >())
    .def("GetBond", GetBondAtoms, return_value_policy<reference_existing_object>()) //, return_internal_reference<1> >())
    .def("GetAtom", &OBMol::GetAtom, return_value_policy<reference_existing_object>()) //, return_internal_reference<1> >())
    .def("farthestAtom",&Molecule::farthestAtom, return_value_policy<reference_existing_object>() );


  // register openbabel SWIG objects
  converter::registry::insert(&extract_swig_wrapped_pointer, type_id<OpenBabel::OBMol>());
  converter::registry::insert(&extract_swig_wrapped_pointer, type_id<OpenBabel::OBAtom>());
} */


#endif
