#ifndef BOOST_H
#define BOOST_H

#include <Python.h>
#include <boost/python.hpp>
#include <avogadro/primitive.h>

#include <openbabel/mol.h>
#include <openbabel/atom.h>
#include <openbabel/bond.h>

using namespace boost::python;

using namespace Avogadro;
using namespace OpenBabel;

BOOST_PYTHON_MODULE(Avogadro) {
  class_<OpenBabel::OBBond>("OBBond");
  class_<Avogadro::Bond, bases<OpenBabel::OBBond>, boost::noncopyable>("Bond")
    .def("GetLength", &OBBond::GetLength)
    .def("GetBO", &OBBond::GetBO)
		.def("SetBO", &OBBond::SetBO);
  class_<OpenBabel::OBAtom>("OBAtom");
// Overloaded SetVector, so we give it an alias
	void (OBAtom::*SetVectorDouble)(double,double,double) = &OBAtom::SetVector;

  class_<Avogadro::Atom, bases<OpenBabel::OBAtom>, boost::noncopyable>("Atom")
    .def("SetAtomicNum", &OBAtom::SetAtomicNum)
    .def("SetIsotope", &OBAtom::SetIsotope)
    .def("SetImplicitValence", &OBAtom::SetImplicitValence)
    .def("SetPartialCharge", &OBAtom::SetPartialCharge)
    .def("SetFormalCharge", &OBAtom::SetFormalCharge)
    .def("SetVector", SetVectorDouble)
    .def("MatchesSMARTS", &OBAtom::MatchesSMARTS)
    .def("GetIdx", &OBAtom::GetIdx)
    .def("GetAtomicNum", &OBAtom::GetAtomicNum)
    .def("GetAtomicMass", &OBAtom::GetAtomicMass)
    .def("GetX", &OBAtom::GetX)
    .def("GetY", &OBAtom::GetY)
    .def("GetZ", &OBAtom::GetZ)
    .def("GetPartialCharge", &OBAtom::GetPartialCharge);
  class_<OpenBabel::OBMol>("OBMol");

// OVerloaded methods, so we give them aliases
	void (OBMol::*SetTitle)(const char *) = &OBMol::SetTitle;
	void (OBMol::*ToInertialFrame)() = &OBMol::ToInertialFrame;
	void (OBMol::*Center)() = &OBMol::Center;
	bool (OBMol::*DeleteHydrogens)() = &OBMol::DeleteHydrogens;
	bool (OBMol::*AddHydrogens)(bool, bool) = &OBMol::AddHydrogens;
	OBBond * (OBMol::*GetBondIDX)(int)const = &OBMol::GetBond;
	OBBond * (OBMol::*GetBondAtoms)(int, int)const = &OBMol::GetBond;

  class_<Avogadro::Molecule, bases<OpenBabel::OBMol> >("Molecule")
    //.add_property("atoms", range(&OBMol::BeginAtoms(), &OBMol::EndAtoms()))
    .def("NumAtoms", &OBMol::NumAtoms)
    .def("NumBonds", &OBMol::NumBonds)
    .def("NumResidues", &OBMol::NumResidues)
    .def("GetFormula", &OBMol::GetFormula)
    .def("GetTitle", &OBMol::GetTitle)
    .def("GetMolWt", &OBMol::GetMolWt)
    .def("GetTotalCharge", &OBMol::GetTotalCharge)
    .def("GetSpinMultiplicity", &OBMol::GetTotalSpinMultiplicity)
    .def("NewAtom", &OBMol::NewAtom, return_value_policy<reference_existing_object>())
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
    .def("GetBond", GetBondIDX, return_value_policy<reference_existing_object>()) //, return_internal_reference<1> >())
    .def("GetBond", GetBondAtoms, return_value_policy<reference_existing_object>()) //, return_internal_reference<1> >())
    .def("GetAtom", &OBMol::GetAtom, return_value_policy<reference_existing_object>()) //, return_internal_reference<1> >())
    .def("farthestAtom",&Molecule::farthestAtom, return_value_policy<reference_existing_object>() );
}

#endif
