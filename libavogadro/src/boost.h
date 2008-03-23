#include <Python.h>
#include <boost/python.hpp>
#include <avogadro/primitive.h>

using namespace boost::python;

using namespace Avogadro;
using namespace OpenBabel;

BOOST_PYTHON_MODULE(Avogadro) {
  class_<Avogadro::Atom, boost::noncopyable>("Atom")
    .def("GetIdx", &OBAtom::GetIdx)
    .def("GetX", &OBAtom::GetX)
    .def("GetY", &OBAtom::GetY)
    .def("GetZ", &OBAtom::GetZ);
  class_<Avogadro::Molecule/*, bases<OpenBabel::OBMol> */>("Molecule")
    //.add_property("atoms", range(&OBMol::BeginAtoms(), &OBMol::EndAtoms()))
    .def("NumAtoms", &OBMol::NumAtoms)
    .def("NumBonds", &OBMol::NumBonds)
    .def("DeleteAtom", &OBMol::DeleteAtom)
    .def("GetAtom", &OBMol::GetAtom, return_internal_reference<1>())
    .def("farthestAtom",&Molecule::farthestAtom, return_internal_reference<1>() );
}
