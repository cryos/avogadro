#include <Python.h>
#include <boost/python.hpp>

#include <avogadro/primitive.h>

using namespace boost::python;
using namespace Avogadro;

void export_Primitive()
{

  enum_<Primitive::Type>("PrimitiveType")
    // the Type enum
    .value("OtherType", Primitive::OtherType)
    .value("MoleculeType", Primitive::MoleculeType)
    .value("AtomType", Primitive::AtomType)
    .value("BondType", Primitive::BondType)
    .value("ResidueType", Primitive::ResidueType)
    .value("ChainType", Primitive::ChainType)
    .value("FragmentType", Primitive::FragmentType)
    .value("SurfaceType", Primitive::SurfaceType)
    .value("MeshType", Primitive::MeshType)
    .value("CubeType", Primitive::CubeType)
    .value("PlaneType", Primitive::PlaneType)
    .value("GridType", Primitive::GridType)
    .value("PointType", Primitive::PointType)
    .value("LineType", Primitive::LineType)
    .value("VectorType", Primitive::VectorType)
    .value("NonbondedType", Primitive::NonbondedType)
    .value("TextType", Primitive::TextType)
    .value("LastType", Primitive::LastType)
    .value("FirstType", Primitive::FirstType)
    ;

  class_<Avogadro::Primitive, boost::noncopyable>("Primitive")
    // read-only properties
    .add_property("id", &Primitive::id) // read-only, managed by Molecule
    .add_property("index", &Primitive::index) // read-only, managed by Molecule
    .add_property("type", &Primitive::type) // read-only, managed by Molecule
    // real functions
    .def("update", &Primitive::update)
    ;
 
}
