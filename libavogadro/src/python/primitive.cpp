// Last update: timvdm 12 May 2009

#include <boost/python.hpp>

#include <avogadro/primitive.h>

#include <QReadWriteLock>

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

  class_<Avogadro::Primitive, boost::noncopyable>("Primitive", no_init)
    // read-only properties (managed by molecule)
    .add_property("id", 
        &Primitive::id,
        "The unique id of the primitive.")

    .add_property("index",
        &Primitive::index,
        "The index of the primitive.")

    .add_property("type", 
        &Primitive::type,
        "The primitive type (one of Primitive::Type)")

    //
    // real functions
    //
    .def("update", &Primitive::update,
        "Function used to push changes to a primitive to the rest of the "
        "system.  At this time there is no way (other than this) to "
        "generate a signal when properties of a primitive change.")

    // most functions lock the lock, having it in python will only cause double locks to freeze the app...
    //.def("lock", &Primitive::lock, return_value_policy<manage_new_object>())
    ;
 
}
