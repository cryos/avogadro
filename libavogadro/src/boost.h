#ifndef __BOOST_H
#define __BOOST_H

#include <Python.h>
#include <boost/python.hpp>
#include <avogadro/primitive.h>

#include <openbabel/mol.h>
#include <openbabel/atom.h>

using namespace boost::python;

using namespace Avogadro;
using namespace OpenBabel;

BOOST_PYTHON_MODULE(Avogadro) {
  class_<OpenBabel::OBAtom>("OBAtom");
  class_<Avogadro::Atom, bases<OpenBabel::OBAtom>, boost::noncopyable>("Atom")
    .def("GetIdx", &OBAtom::GetIdx)
    .def("GetX", &OBAtom::GetX)
    .def("GetY", &OBAtom::GetY)
    .def("GetZ", &OBAtom::GetZ);
  class_<Avogadro::Molecule/*, bases<OpenBabel::OBMol> */>("Molecule")
    //.add_property("atoms", range(&OBMol::BeginAtoms(), &OBMol::EndAtoms()))
    .def("NumAtoms", &OBMol::NumAtoms)
    .def("NumBonds", &OBMol::NumBonds)
    .def("DeleteAtom", &OBMol::DeleteAtom)
    .def("GetAtom", &Molecule::GetAtom, return_value_policy<reference_existing_object>()) //, return_internal_reference<1> >())
    .def("farthestAtom",&Molecule::farthestAtom, return_value_policy<reference_existing_object>() );
}

#endif
